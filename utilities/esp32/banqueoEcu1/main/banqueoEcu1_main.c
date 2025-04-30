/**
 * @file banqueoEcu1_main.c
 * @brief ECU test bench logic for ESP32
 *
 * This file is part of the AutomotiveGuide_es project.
 *
 * > **Repository**: https://github.com/edgarefraindp/AutomotiveGuide_es
 * > **For donations and support**: Please visit the GitHub repository page
 *
 * @author AutomotiveGuide_es
 * @date April 2025
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "driver/timer.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

// Pin definitions for sensor emulation
#define PIN_EMU_CKP        GPIO_NUM_16    // CKP sensor emulation (crankshaft)
#define PIN_EMU_CMP        GPIO_NUM_17    // CMP sensor emulation (camshaft)
#define PIN_EMU_MAP        GPIO_NUM_18    // MAP sensor emulation (manifold pressure)
#define PIN_EMU_TPS        GPIO_NUM_19    // TPS sensor emulation (throttle position)
#define PIN_EMU_MAF        GPIO_NUM_21    // MAF sensor emulation (air flow)
#define PIN_EMU_ECT        GPIO_NUM_22    // ECT sensor emulation (coolant temp)
#define PIN_EMU_IAT        GPIO_NUM_23    // IAT sensor emulation (intake air temp)

// Pin definitions for actuator monitoring
#define PIN_MON_INJ1       GPIO_NUM_25    // Injector 1 monitoring
#define PIN_MON_INJ2       GPIO_NUM_26    // Injector 2 monitoring
#define PIN_MON_INJ3       GPIO_NUM_27    // Injector 3 monitoring
#define PIN_MON_INJ4       GPIO_NUM_32    // Injector 4 monitoring
#define PIN_MON_COIL       GPIO_NUM_33    // Coil monitoring
#define PIN_CTRL_PUMP      GPIO_NUM_34    // Fuel pump control
#define PIN_COMM           GPIO_NUM_35    // Communication line

// Constants for PWM configuration
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          PIN_EMU_TPS
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // 8192 levels of resolution
#define LEDC_FREQUENCY          5000              // Frequency in Hz

// Timer configuration for signal generation
#define TIMER_DIVIDER           16
#define TIMER_SCALE             (TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_CKP_INTERVAL_SEC  0.02              // 50Hz for 1500 RPM (4-stroke)

// Constants for simulated engine control
#define RPM_MIN                 800
#define RPM_MAX                 6000
#define RPM_DEFAULT             1000
#define TPS_MIN                 0
#define TPS_MAX                 100
#define TPS_DEFAULT             15                // 15% TPS = idle
#define MAP_MIN                 20                // 20 kPa (high suction)
#define MAP_MAX                 101               // 101 kPa (atmospheric pressure)
#define MAF_MIN                 2                 // g/s at idle
#define MAF_MAX                 150               // g/s at full load
#define ECT_MIN                 -40               // Minimum temperature (°C)
#define ECT_MAX                 120               // Maximum temperature (°C)
#define ECT_DEFAULT             85                // Normal operating temperature (°C)
#define IAT_DEFAULT             25                // Default ambient temperature (°C)

// Data buffer size
#define DATA_BUFFER_SIZE        1024

// Queue size for events
#define QUEUE_SIZE              10

// Constants for anomaly detection
#define PULSE_MARGIN            0.15              // 15% acceptable error margin
#define MIN_VALID_PULSE_US      50                // Minimum valid pulse (μs)

static const char *TAG = "BANQUEO_ECU";

// Structure to store simulated engine parameters
typedef struct {
    uint16_t rpm;                // Engine RPM
    uint8_t tps;                 // Throttle position (0-100%)
    uint8_t map;                 // MAP pressure (kPa)
    uint8_t maf;                 // MAF flow (g/s)
    int8_t ect;                  // Coolant temperature (°C)
    int8_t iat;                  // Intake air temperature (°C)
    bool engineRunning;          // Engine state (on/off)
} EngineParams;

// Structure to store output pulse parameters
typedef struct {
    uint32_t inj1PulseWidth;     // Injector 1 pulse width (μs)
    uint32_t inj2PulseWidth;     // Injector 2 pulse width (μs)
    uint32_t inj3PulseWidth;     // Injector 3 pulse width (μs)
    uint32_t inj4PulseWidth;     // Injector 4 pulse width (μs)
    uint32_t coilDwell;          // Coil dwell time (μs)
    uint32_t lastUpdateTime;     // Last update time (ms)
    bool inj1Active;             // Injector 1 state
    bool inj2Active;             // Injector 2 state
    bool inj3Active;             // Injector 3 state
    bool inj4Active;             // Injector 4 state
    bool coilActive;             // Coil state
    bool pumpActive;             // Pump state
} OutputSignals;

// Enumeration for pulse event types
typedef enum {
    EVENT_INJ1_RISING,
    EVENT_INJ1_FALLING,
    EVENT_INJ2_RISING,
    EVENT_INJ2_FALLING,
    EVENT_INJ3_RISING,
    EVENT_INJ3_FALLING,
    EVENT_INJ4_RISING,
    EVENT_INJ4_FALLING,
    EVENT_COIL_RISING,
    EVENT_COIL_FALLING
} PulseEventType;

// Structure for pulse events
typedef struct {
    PulseEventType type;
    uint64_t timestamp;          // Timestamp in microseconds
} PulseEvent;

// Global variables
static EngineParams engineParams = {
    .rpm = RPM_DEFAULT,
    .tps = TPS_DEFAULT,
    .map = MAP_MAX - 20,         // ~80 kPa at idle
    .maf = MAF_MIN + 3,          // 5 g/s at idle
    .ect = ECT_DEFAULT,
    .iat = IAT_DEFAULT,
    .engineRunning = true
};

static OutputSignals outputSignals = {0};
static uint64_t lastCkpPulseTime = 0;
static QueueHandle_t pulseEventQueue = NULL;

// Variables for CKP teeth emulation (type 60-2)
static const int TOTAL_TEETH = 60;
static const int MISSING_TEETH = 2;
static int currentTooth = 0;

/**
 * Configures the PWM channel for analog sensor emulation
 * 
 * @return ESP_OK if configuration was successful
 */
static esp_err_t ConfigurePwm(void)
{
    // Timer configuration for PWM
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Channel configuration for TPS
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL,
        .duty = 0,
        .gpio_num = PIN_EMU_TPS,
        .speed_mode = LEDC_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    return ESP_OK;
}

/**
 * Configures GPIO pins for sensor emulation and monitoring
 * 
 * @return ESP_OK if configuration was successful
 */
static esp_err_t ConfigureGPIO(void)
{
    // Configure pins for digital sensor emulation
    gpio_config_t io_conf_output = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << PIN_EMU_CKP) | 
                        (1ULL << PIN_EMU_CMP) |
                        (1ULL << PIN_EMU_MAP) |
                        (1ULL << PIN_EMU_MAF) |
                        (1ULL << PIN_EMU_ECT) |
                        (1ULL << PIN_EMU_IAT),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&io_conf_output);
    
    // Configure pins for actuator monitoring with interrupts
    gpio_config_t io_conf_input = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << PIN_MON_INJ1) |
                       (1ULL << PIN_MON_INJ2) |
                       (1ULL << PIN_MON_INJ3) |
                       (1ULL << PIN_MON_INJ4) |
                       (1ULL << PIN_MON_COIL) |
                       (1ULL << PIN_CTRL_PUMP),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf_input);
    
    // Install GPIO interrupt service
    gpio_install_isr_service(0);
    
    return ESP_OK;
}

/**
 * Configures UART for terminal communication
 * 
 * @return ESP_OK if configuration was successful
 */
static esp_err_t ConfigureUART(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    // Configure UART for terminal communication
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    return ESP_OK;
}

/**
 * Configures the timer for CKP/CMP signal generation
 * 
 * @return ESP_OK if configuration was successful
 */
static esp_err_t ConfigureTimer(void)
{
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_EN,
    };
    
    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &config));
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));
    
    // Calculate interval for CKP signal based on RPM
    double ckp_interval = (60.0 / engineParams.rpm) / (TOTAL_TEETH - MISSING_TEETH);
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, ckp_interval * TIMER_SCALE));
    
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
    ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));
    
    return ESP_OK;
}

/**
 * Generates PWM value based on sensor value and range
 * 
 * @param value Current sensor value
 * @param minValue Minimum sensor range value
 * @param maxValue Maximum sensor range value
 * @return Duty cycle value (0-8191)
 */
static uint32_t CalculatePwmDuty(int value, int minValue, int maxValue)
{
    const uint32_t MAX_DUTY = (1 << LEDC_DUTY_RES) - 1;
    
    // Limit value to range
    if (value < minValue) value = minValue;
    if (value > maxValue) value = maxValue;
    
    // Calculate percentage (0.0 - 1.0)
    float percentage = (float)(value - minValue) / (float)(maxValue - minValue);
    
    // Calculate duty cycle (inversely proportional for some sensors like NTC)
    return (uint32_t)(percentage * MAX_DUTY);
}

/**
 * Updates the TPS sensor signal
 */
static void UpdateTpsSensor(void)
{
    uint32_t duty = CalculatePwmDuty(engineParams.tps, TPS_MIN, TPS_MAX);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

/**
 * Updates the MAP sensor signal (implemented with PWM)
 */
static void UpdateMapSensor(void)
{
    // For MAP we use variable PWM frequency based on pressure
    // 100 Hz higher for every additional 10 kPa
    int frequency = 100 + (engineParams.map - MAP_MIN) * 10;
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency);
}

/**
 * Updates the MAF sensor emulation
 */
static void UpdateMafSensor(void)
{
    // Basic implementation - in a real system DAC would be used
    // For simple emulation we use PWM output with duty cycle
    uint32_t duty = CalculatePwmDuty(engineParams.maf, MAF_MIN, MAF_MAX);
    
    // Here output to an external DAC or filtered PWM would be implemented
}

/**
 * Emulates the resistance of the ECT sensor (NTC)
 */
static void UpdateEctSensor(void)
{
    // For temperature sensors a digital potentiometer or resistor network would be used
    // This is a simulated implementation for demonstration purposes
    float resistance = 0;
    
    // Simplified NTC resistance calculation
    if (engineParams.ect > -40 && engineParams.ect < 150) {
        // Simplified formula: R = R0 * e^(B*(1/T - 1/T0))
        // Where R0 = 2.5kOhm at 25°C, B = 3380
        float t1 = 1.0f / (273.15f + (float)engineParams.ect);
        float t0 = 1.0f / (273.15f + 25.0f);  // Reference temperature (25°C)
        resistance = 2500.0f * expf(3380.0f * (t1 - t0));
    }
    
    ESP_LOGI(TAG, "ECT: %d°C, Resistance: %.2f Ohm", engineParams.ect, resistance);
}

/**
 * Emulates the resistance of the IAT sensor (NTC)
 */
static void UpdateIatSensor(void)
{
    // Similar implementation to ECT
    float resistance = 0;
    
    if (engineParams.iat > -40 && engineParams.iat < 150) {
        float t1 = 1.0f / (273.15f + (float)engineParams.iat);
        float t0 = 1.0f / (273.15f + 25.0f);
        resistance = 2500.0f * expf(3380.0f * (t1 - t0));
    }
    
    ESP_LOGI(TAG, "IAT: %d°C, Resistance: %.2f Ohm", engineParams.iat, resistance);
}

/**
 * Timer handler for CKP/CMP signal generation
 * 
 * @param para Pointer to parameters (not used)
 */
static void IRAM_ATTR TimerGroupIsr(void *para)
{
    // Clear interrupt flag
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    
    // Increment tooth position
    currentTooth = (currentTooth + 1) % TOTAL_TEETH;
    
    // Check if we are in the missing teeth position (58-59 in a 60-2)
    bool generatePulse = true;
    if (currentTooth >= (TOTAL_TEETH - MISSING_TEETH)) {
        generatePulse = false;
    }
    
    // Generate CKP pulse
    if (engineParams.engineRunning) {
        gpio_set_level(PIN_EMU_CKP, generatePulse ? 1 : 0);
        
        // At a certain tooth, also generate CMP pulse (e.g., at tooth 10)
        if (currentTooth == 10) {
            gpio_set_level(PIN_EMU_CMP, 1);
        } else if (currentTooth == 11) {
            gpio_set_level(PIN_EMU_CMP, 0);
        }
    }
    
    // Recalculate interval based on current RPM
    double ckp_interval = (60.0 / engineParams.rpm) / (TOTAL_TEETH - MISSING_TEETH);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, ckp_interval * TIMER_SCALE);
}

/**
 * ISR (Interrupt Service Routine) for monitoring input pins
 * 
 * @param arg Custom argument (pin number in this case)
 */
static void IRAM_ATTR GpioIsrHandler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    PulseEvent event;
    event.timestamp = esp_timer_get_time();
    
    int level = gpio_get_level(gpio_num);
    
    switch (gpio_num) {
        case PIN_MON_INJ1:
            event.type = level ? EVENT_INJ1_RISING : EVENT_INJ1_FALLING;
            break;
        case PIN_MON_INJ2:
            event.type = level ? EVENT_INJ2_RISING : EVENT_INJ2_FALLING;
            break;
        case PIN_MON_INJ3:
            event.type = level ? EVENT_INJ3_RISING : EVENT_INJ3_FALLING;
            break;
        case PIN_MON_INJ4:
            event.type = level ? EVENT_INJ4_RISING : EVENT_INJ4_FALLING;
            break;
        case PIN_MON_COIL:
            event.type = level ? EVENT_COIL_RISING : EVENT_COIL_FALLING;
            break;
    }
    
    xQueueSendFromISR(pulseEventQueue, &event, NULL);
}

/**
 * Updates all values of emulated sensors
 */
static void UpdateAllSensors(void)
{
    UpdateTpsSensor();
    UpdateMapSensor();
    UpdateMafSensor();
    UpdateEctSensor();
    UpdateIatSensor();
    
    ESP_LOGI(TAG, "Sensors updated - RPM: %d, TPS: %d%%, MAP: %dkPa", 
             engineParams.rpm, engineParams.tps, engineParams.map);
}

/**
 * Calculates sensor values based on physical relationships
 * For example, higher TPS (throttle) results in lower MAP (suction pressure)
 */
static void CalculateSensorRelationships(void)
{
    // Correlate TPS with MAP (higher throttle opening, higher manifold pressure)
    if (engineParams.tps < 10) {
        // Idle - high suction
        engineParams.map = MAP_MIN + 15;
    } else if (engineParams.tps > 90) {
        // Throttle almost fully open - pressure close to atmospheric
        engineParams.map = MAP_MAX - 5;
    } else {
        // Linear interpolation between minimum and maximum
        engineParams.map = MAP_MIN + ((engineParams.tps * (MAP_MAX - MAP_MIN)) / 100);
    }
    
    // Correlate TPS with MAF (higher opening, higher air flow)
    if (engineParams.tps < 10) {
        // Minimum flow at idle
        engineParams.maf = MAF_MIN + 3;
    } else {
        // Non-linear interpolation (flow increases exponentially with TPS)
        float tpsPercentage = (float)engineParams.tps / 100.0f;
        float mafRange = MAF_MAX - MAF_MIN;
        engineParams.maf = MAF_MIN + (uint8_t)(mafRange * tpsPercentage * tpsPercentage);
    }
}

/**
 * Processes pulse events from the ECU output
 * 
 * @param lastEvent The last detected pulse event
 * @param state Current output state
 */
static void ProcessPulseEvent(PulseEvent lastEvent, OutputSignals* state)
{
    static uint64_t injStartTimes[4] = {0};
    static uint64_t coilStartTime = 0;
    
    uint64_t pulseWidth = 0;
    
    switch (lastEvent.type) {
        case EVENT_INJ1_RISING:
            injStartTimes[0] = lastEvent.timestamp;
            state->inj1Active = true;
            break;
        
        case EVENT_INJ1_FALLING:
            if (injStartTimes[0] > 0) {
                pulseWidth = lastEvent.timestamp - injStartTimes[0];
                if (pulseWidth > MIN_VALID_PULSE_US) {
                    state->inj1PulseWidth = pulseWidth;
                    ESP_LOGI(TAG, "Injector 1: %llu μs", pulseWidth);
                }
            }
            state->inj1Active = false;
            break;
            
        case EVENT_INJ2_RISING:
            injStartTimes[1] = lastEvent.timestamp;
            state->inj2Active = true;
            break;
            
        case EVENT_INJ2_FALLING:
            if (injStartTimes[1] > 0) {
                pulseWidth = lastEvent.timestamp - injStartTimes[1];
                if (pulseWidth > MIN_VALID_PULSE_US) {
                    state->inj2PulseWidth = pulseWidth;
                    ESP_LOGI(TAG, "Injector 2: %llu μs", pulseWidth);
                }
            }
            state->inj2Active = false;
            break;
            
        case EVENT_INJ3_RISING:
            injStartTimes[2] = lastEvent.timestamp;
            state->inj3Active = true;
            break;
            
        case EVENT_INJ3_FALLING:
            if (injStartTimes[2] > 0) {
                pulseWidth = lastEvent.timestamp - injStartTimes[2];
                if (pulseWidth > MIN_VALID_PULSE_US) {
                    state->inj3PulseWidth = pulseWidth;
                    ESP_LOGI(TAG, "Injector 3: %llu μs", pulseWidth);
                }
            }
            state->inj3Active = false;
            break;
            
        case EVENT_INJ4_RISING:
            injStartTimes[3] = lastEvent.timestamp;
            state->inj4Active = true;
            break;
            
        case EVENT_INJ4_FALLING:
            if (injStartTimes[3] > 0) {
                pulseWidth = lastEvent.timestamp - injStartTimes[3];
                if (pulseWidth > MIN_VALID_PULSE_US) {
                    state->inj4PulseWidth = pulseWidth;
                    ESP_LOGI(TAG, "Injector 4: %llu μs", pulseWidth);
                }
            }
            state->inj4Active = false;
            break;
            
        case EVENT_COIL_RISING:
            coilStartTime = lastEvent.timestamp;
            state->coilActive = true;
            break;
            
        case EVENT_COIL_FALLING:
            if (coilStartTime > 0) {
                pulseWidth = lastEvent.timestamp - coilStartTime;
                if (pulseWidth > MIN_VALID_PULSE_US) {
                    state->coilDwell = pulseWidth;
                    ESP_LOGI(TAG, "Coil Dwell: %llu μs", pulseWidth);
                }
            }
            state->coilActive = false;
            break;
    }
    
    state->lastUpdateTime = esp_timer_get_time() / 1000; // time in ms
}

/**
 * Analyzes ECU outputs and detects anomalies
 * 
 * @param state Current output signal state
 */
static void DetectAnomalies(const OutputSignals* state)
{
    static uint32_t lastAnomalyTime = 0;
    uint32_t currentTime = esp_timer_get_time() / 1000;
    
    // Only check every second to avoid console saturation
    if (currentTime - lastAnomalyTime < 1000) {
        return;
    }
    
    lastAnomalyTime = currentTime;
    
    // 1. Check for missing pulses in injectors
    if (engineParams.engineRunning && engineParams.rpm > 0) {
        if (state->inj1PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALY: No pulses in Injector 1");
        }
        
        if (state->inj2PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALY: No pulses in Injector 2");
        }
        
        if (state->inj3PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALY: No pulses in Injector 3");
        }
        
        if (state->inj4PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALY: No pulses in Injector 4");
        }
    }
    
    // 2. Check balance between injectors (should not have more than 15% difference)
    if (state->inj1PulseWidth > 0 && state->inj2PulseWidth > 0) {
        float ratio = (float)state->inj1PulseWidth / (float)state->inj2PulseWidth;
        if (ratio < (1.0 - PULSE_MARGIN) || ratio > (1.0 + PULSE_MARGIN)) {
            ESP_LOGW(TAG, "ANOMALY: Imbalance between Injector 1 (%lu μs) and Injector 2 (%lu μs)", 
                    state->inj1PulseWidth, state->inj2PulseWidth);
        }
    }
    
    // 3. Check that injection times increase with high TPS
    if (engineParams.tps > 50 && state->inj1PulseWidth < 3000) {
        ESP_LOGW(TAG, "ANOMALY: High TPS (%d%%) but low injection time (%lu μs)", 
                engineParams.tps, state->inj1PulseWidth);
    }
    
    // 4. Check correlation between RPM and coil dwell
    // Higher RPM, shorter dwell time
    if (engineParams.rpm > 3000 && state->coilDwell > 4000) {
        ESP_LOGW(TAG, "ANOMALY: High RPM (%d) but very long coil dwell (%lu μs)",
                engineParams.rpm, state->coilDwell);
    }
}

/**
 * Task to handle pulse events from ISR
 * 
 * @param pvParameters Task parameters (not used)
 */
static void PulseMonitorTask(void *pvParameters)
{
    PulseEvent event;
    
    // Initialize timestamps to avoid false positives
    outputSignals.lastUpdateTime = esp_timer_get_time() / 1000;
    
    while (1) {
        if (xQueueReceive(pulseEventQueue, &event, portMAX_DELAY)) {
            // Process pulse event
            ProcessPulseEvent(event, &outputSignals);
            
            // Analyze outputs to detect anomalies
            DetectAnomalies(&outputSignals);
        }
    }
}

/**
 * Processes commands received via serial port
 * 
 * @param cmdBuffer Buffer containing the received command
 * @param length Command length
 * @return true if the command was recognized and processed
 */
static bool ProcessCommand(const char* cmdBuffer, size_t length)
{
    if (strncmp(cmdBuffer, "rpm:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= RPM_MIN && value <= RPM_MAX) {
            engineParams.rpm = value;
            ESP_LOGI(TAG, "RPM set to: %d", engineParams.rpm);
            return true;
        }
    } else if (strncmp(cmdBuffer, "tps:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= TPS_MIN && value <= TPS_MAX) {
            engineParams.tps = value;
            ESP_LOGI(TAG, "TPS set to: %d%%", engineParams.tps);
            return true;
        }
    } else if (strncmp(cmdBuffer, "ect:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= ECT_MIN && value <= ECT_MAX) {
            engineParams.ect = value;
            ESP_LOGI(TAG, "ECT set to: %d°C", engineParams.ect);
            return true;
        }
    } else if (strncmp(cmdBuffer, "iat:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= ECT_MIN && value <= ECT_MAX) { // Same range as ECT
            engineParams.iat = value;
            ESP_LOGI(TAG, "IAT set to: %d°C", engineParams.iat);
            return true;
        }
    } else if (strcmp(cmdBuffer, "status") == 0) {
        ESP_LOGI(TAG, "=== System Status ===");
        ESP_LOGI(TAG, "RPM: %d", engineParams.rpm);
        ESP_LOGI(TAG, "TPS: %d%%", engineParams.tps);
        ESP_LOGI(TAG, "MAP: %d kPa", engineParams.map);
        ESP_LOGI(TAG, "MAF: %d g/s", engineParams.maf);
        ESP_LOGI(TAG, "ECT: %d°C", engineParams.ect);
        ESP_LOGI(TAG, "IAT: %d°C", engineParams.iat);
        ESP_LOGI(TAG, "Engine: %s", engineParams.engineRunning ? "On" : "Off");
        ESP_LOGI(TAG, "Injector 1: %lu μs", outputSignals.inj1PulseWidth);
        ESP_LOGI(TAG, "Injector 2: %lu μs", outputSignals.inj2PulseWidth);
        ESP_LOGI(TAG, "Injector 3: %lu μs", outputSignals.inj3PulseWidth);
        ESP_LOGI(TAG, "Injector 4: %lu μs", outputSignals.inj4PulseWidth);
        ESP_LOGI(TAG, "Coil Dwell: %lu μs", outputSignals.coilDwell);
        return true;
    } else if (strcmp(cmdBuffer, "start") == 0) {
        engineParams.engineRunning = true;
        ESP_LOGI(TAG, "Engine started");
        return true;
    } else if (strcmp(cmdBuffer, "stop") == 0) {
        engineParams.engineRunning = false;
        ESP_LOGI(TAG, "Engine stopped");
        return true;
    } else if (strcmp(cmdBuffer, "help") == 0) {
        ESP_LOGI(TAG, "Available commands:");
        ESP_LOGI(TAG, "  rpm:VALUE     - Set RPM (800-6000)");
        ESP_LOGI(TAG, "  tps:VALUE     - Set throttle position (0-100%)");
        ESP_LOGI(TAG, "  ect:VALUE     - Set coolant temperature (-40 to 120°C)");
        ESP_LOGI(TAG, "  iat:VALUE     - Set intake air temperature (-40 to 120°C)");
        ESP_LOGI(TAG, "  start         - Start engine (begin signals)");
        ESP_LOGI(TAG, "  stop          - Stop engine (halt signals)");
        ESP_LOGI(TAG, "  status        - Show current status");
        ESP_LOGI(TAG, "  help          - Show this help");
        return true;
    }
    
    return false;
}

/**
 * Task to handle user interface via serial port
 * 
 * @param pvParameters Task parameters (not used)
 */
static void SerialInterfaceTask(void *pvParameters)
{
    char cmdBuffer[64];
    size_t cmdIndex = 0;
    
    ESP_LOGI(TAG, "\n=== ECU Test Bench - Control Interface ===");
    ESP_LOGI(TAG, "Type 'help' to see available commands");
    
    while (1) {
        // Read data from serial port
        uint8_t c;
        int len = uart_read_bytes(UART_NUM_0, &c, 1, 20 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            // Local echo of character
            uart_write_bytes(UART_NUM_0, (const char*)&c, 1);
            
            if (c == '\n' || c == '\r') {
                // End of command
                uart_write_bytes(UART_NUM_0, "\n", 1);
                if (cmdIndex > 0) {
                    cmdBuffer[cmdIndex] = '\0';
                    if (!ProcessCommand(cmdBuffer, cmdIndex)) {
                        ESP_LOGW(TAG, "Unrecognized command: %s", cmdBuffer);
                    }
                    cmdIndex = 0;
                }
            } else if (cmdIndex < sizeof(cmdBuffer) - 1) {
                // Add character to buffer
                cmdBuffer[cmdIndex++] = c;
            }
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/**
 * Task to update engine simulation
 * 
 * @param pvParameters Task parameters (not used)
 */
static void EngineSimulationTask(void *pvParameters)
{
    while (1) {
        // Update relationships between sensors
        CalculateSensorRelationships();
        
        // Update all sensors
        UpdateAllSensors();
        
        // Wait before next update
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

/**
 * Initializes system components
 * 
 * @return ESP_OK if initialization was successful
 */
static esp_err_t InitializeSystem(void)
{
    esp_err_t ret;
    
    // Initialize event queue
    pulseEventQueue = xQueueCreate(QUEUE_SIZE, sizeof(PulseEvent));
    if (!pulseEventQueue) {
        ESP_LOGE(TAG, "Error creating event queue");
        return ESP_FAIL;
    }
    
    // Configure GPIO
    ret = ConfigureGPIO();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error in GPIO configuration");
        return ret;
    }
    
    // Configure UART
    ret = ConfigureUART();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error in UART configuration");
        return ret;
    }
    
    // Configure PWM
    ret = ConfigurePwm();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error in PWM configuration");
        return ret;
    }
    
    // Configure timer for CKP/CMP signals
    ret = ConfigureTimer();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error in timer configuration");
        return ret;
    }
    
    // Attach interrupt handlers for monitoring pins
    gpio_isr_handler_add(PIN_MON_INJ1, GpioIsrHandler, (void*) PIN_MON_INJ1);
    gpio_isr_handler_add(PIN_MON_INJ2, GpioIsrHandler, (void*) PIN_MON_INJ2);
    gpio_isr_handler_add(PIN_MON_INJ3, GpioIsrHandler, (void*) PIN_MON_INJ3);
    gpio_isr_handler_add(PIN_MON_INJ4, GpioIsrHandler, (void*) PIN_MON_INJ4);
    gpio_isr_handler_add(PIN_MON_COIL, GpioIsrHandler, (void*) PIN_MON_COIL);
    
    return ESP_OK;
}

/**
 * Main entry point of the application
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Starting ECU test bench system...");
    
    // Initialize the system
    esp_err_t ret = InitializeSystem();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error in system initialization");
        return;
    }
    
    // Create application tasks
    xTaskCreate(PulseMonitorTask, "pulse_monitor", 4096, NULL, 10, NULL);
    xTaskCreate(SerialInterfaceTask, "serial_interface", 4096, NULL, 5, NULL);
    xTaskCreate(EngineSimulationTask, "engine_simulation", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "ECU test bench system started successfully");
}