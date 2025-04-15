/**
 * @file encendidoElectronico1_main.c
 * @brief MAP Sensor Based Idle Control System
 * 
 * This project implements an idle control system based on reading
 * an intake manifold absolute pressure sensor (MAP). The system
 * monitors pressure and controls engine idle to prevent damage from
 * prolonged activation or inadequate pressure.
 * 
 * @author AutomotiveGuide_es
 * @date April 2025
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "driver/ledc.h"

// Pin definitions 
#define PIN_MAP_SENSOR       34  // Analog input pin for MAP sensor
#define PIN_IAC_VALVE        25  // PWM output pin for IAC valve (via TIP120)
#define PIN_THROTTLE_BODY    26  // PWM output pin for throttle body ("Y" connection)
#define PIN_IDLE_RELAY       27  // Output pin for idle control relay (via TIP120)
#define PIN_START_BUTTON     33  // Physical start/stop button
#define PIN_STATUS_LED       2   // Onboard status LED

// PWM Configuration 
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_IAC    LEDC_CHANNEL_0
#define LEDC_CHANNEL_THROTTLE LEDC_CHANNEL_1
#define LEDC_RESOLUTION     LEDC_TIMER_8_BIT  // 8-bit resolution (0-255)
#define LEDC_FREQUENCY      50               // 50 Hz PWM frequency for IAC

// MAP sensor constants
#define MAP_MIN_PRESSURE     20      // Minimum safe pressure in kPa
#define MAP_MAX_PRESSURE     105     // Maximum safe pressure in kPa
#define MAX_IDLE_TIME        300000  // Maximum continuous idle time (5 min in ms)
#define PRESSURE_HYSTERESIS  5       // Hysteresis to prevent oscillations (kPa)
#define ADC_VREF             3300    // ADC reference voltage in mV

// IAC valve constants
#define IAC_MIN_OPENING      0       // Minimum IAC valve opening (0%)
#define IAC_MAX_OPENING      255     // Maximum IAC valve opening (100%)

// IAC control options
#define IAC_CONTROL_MODE     1       // 1: Direct control, 2: "Y" connection control
#define USE_TIP120_FOR_IAC   1       // 1: Use TIP120 for IAC, 0: Direct control

// System states
#define STATE_OFF            0
#define STATE_ON             1
#define STATE_ERROR          2

// Log tag
static const char *TAG = "MAP_CTRL";

// Global variables
static int systemState = STATE_OFF;
static uint32_t idleStartTime = 0;
static esp_adc_cal_characteristics_t adcCharacteristics;
static bool remoteControlActive = false;
static int iacControlMode = IAC_CONTROL_MODE;

// Function prototypes
void ConfigureGPIO(void);
void ConfigureADC(void);
void ConfigurePWM(void);
void ConfigureBluetooth(void);
uint32_t ReadMAPSensor(void);
void ControlIACValve(uint32_t mapPressure);
void SendPWM(ledc_channel_t channel, uint8_t value);
void ControlEngineIdle(uint32_t mapPressure);
void StopIdle(const char* reason);
void StartIdle(void);
void ProcessBluetoothCommand(char *command);

/**
 * @brief Configures GPIOs used in the project
 */
void ConfigureGPIO(void)
{
    // Configure output pins
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_IDLE_RELAY) | (1ULL << PIN_STATUS_LED);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    // Configure input pin for start button (with pull-up resistor)
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_START_BUTTON);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    
    // Initial pin states
    gpio_set_level(PIN_IDLE_RELAY, 0);
    gpio_set_level(PIN_STATUS_LED, 0);
}

/**
 * @brief Configures PWM module to control IAC valve and throttle body
 */
void ConfigurePWM(void)
{
    // Timer configuration
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_RESOLUTION,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);
    
    // Channel configuration for IAC valve
    ledc_channel_config_t iac_conf = {
        .gpio_num = PIN_IAC_VALVE,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_IAC,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&iac_conf);
    
    // Channel configuration for throttle body ("Y" connection)
    ledc_channel_config_t throttle_conf = {
        .gpio_num = PIN_THROTTLE_BODY,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_THROTTLE,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&throttle_conf);
    
    ESP_LOGI(TAG, "PWM configured for IAC valve and throttle body");
}

/**
 * @brief Configures the ADC to read the MAP sensor
 */
void ConfigureADC(void)
{
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // ADC1 channel 6 = GPIO34
    
    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, ADC_VREF, &adcCharacteristics);
    
    ESP_LOGI(TAG, "ADC configured for MAP sensor reading");
}

/**
 * @brief Configures Bluetooth functionality
 */
void ConfigureBluetooth(void)
{
    ESP_LOGI(TAG, "Starting Bluetooth configuration");
    
    // Initialize NVS component - stores calibration data
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Release default BT controller
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
    
    // BT controller configuration
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // TODO: Implement callbacks for SPP (Serial Port Profile)
    // and discovery/pairing configuration
    
    // Set Bluetooth device name
    esp_bt_dev_set_device_name("IdleControl-ESP32");
    
    ESP_LOGI(TAG, "Bluetooth configured successfully");
}

/**
 * @brief Reads the MAP sensor and converts value to kPa
 * @return MAP pressure in kPa
 */
uint32_t ReadMAPSensor(void)
{
    uint32_t adc_reading = 0;
    
    // Take multiple readings for stability
    for (int i = 0; i < 10; i++) {
        adc_reading += adc1_get_raw(ADC1_CHANNEL_6);
    }
    adc_reading /= 10;
    
    // Convert reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adcCharacteristics);
    
    // Convert voltage to kPa (depends on specific sensor calibration)
    // Assuming typical MAP sensor: 0.5V = 10 kPa, 4.5V = 105 kPa
    uint32_t pressure_kpa = ((voltage - 500) * 95) / 4000 + 10;
    
    ESP_LOGI(TAG, "MAP reading: ADC=%d, Voltage=%dmV, Pressure=%dkPa", adc_reading, voltage, pressure_kpa);
    
    return pressure_kpa;
}

/**
 * @brief Sends a PWM value to the specified channel
 * @param channel LEDC channel to use
 * @param value PWM value from 0-255
 */
void SendPWM(ledc_channel_t channel, uint8_t value)
{
    // Convert 8-bit value (0-255) to duty cycle value (0-255 for 8-bit resolution)
    uint32_t duty = value;
    
    // Set duty cycle
    ledc_set_duty(LEDC_MODE, channel, duty);
    ledc_update_duty(LEDC_MODE, channel);
    
    ESP_LOGI(TAG, "PWM configured: Channel=%d, Value=%d", channel, value);
}

/**
 * @brief Controls IAC valve opening based on MAP pressure
 * @param mapPressure Current manifold pressure in kPa
 */
void ControlIACValve(uint32_t mapPressure)
{
    uint8_t opening;
    
    // If pressure is below minimum, open valve to allow more air
    if (mapPressure < MAP_MIN_PRESSURE) {
        opening = IAC_MAX_OPENING;  // Full open
        ESP_LOGI(TAG, "Low pressure: Maximum IAC opening");
    } 
    // If pressure is above minimum but still low, partial opening
    else if (mapPressure < MAP_MIN_PRESSURE + 10) {
        // Proportional adjustment: closer to minimum = more opening
        opening = (uint8_t)(IAC_MAX_OPENING * (1 - (mapPressure - MAP_MIN_PRESSURE) / 10.0));
        ESP_LOGI(TAG, "Acceptable pressure: Partial IAC opening (%d%%)", (opening * 100) / 255);
    } 
    // If pressure is adequate, close valve
    else {
        opening = IAC_MIN_OPENING;  // Closed
        ESP_LOGI(TAG, "Optimal pressure: IAC closed");
    }
    
    // Control valve using PWM according to configured mode
    if (iacControlMode == 1) {
        // Mode 1: Direct IAC valve control
        SendPWM(LEDC_CHANNEL_IAC, opening);
    } else {
        // Mode 2: Control with "Y" connection to throttle body
        SendPWM(LEDC_CHANNEL_THROTTLE, opening);
    }
}

/**
 * @brief Controls engine idle activation/deactivation based on MAP pressure
 * @param mapPressure Current manifold pressure in kPa
 */
void ControlEngineIdle(uint32_t mapPressure)
{
    // If pressure is outside safe range, stop idle
    if (mapPressure < MAP_MIN_PRESSURE - PRESSURE_HYSTERESIS) {
        StopIdle("Manifold pressure too low");
        return;
    }
    
    if (mapPressure > MAP_MAX_PRESSURE + PRESSURE_HYSTERESIS) {
        StopIdle("Manifold pressure too high");
        return;
    }
    
    // Check if maximum idle time was exceeded
    if (systemState == STATE_ON) {
        uint32_t currentTime = esp_timer_get_time() / 1000; // Convert to milliseconds
        if (currentTime - idleStartTime > MAX_IDLE_TIME) {
            StopIdle("Maximum idle time exceeded");
            return;
        }
    }
    
    // If system is off and pressure is adequate, start idle
    if (systemState == STATE_OFF && 
        mapPressure >= MAP_MIN_PRESSURE && 
        mapPressure <= MAP_MAX_PRESSURE) {
        StartIdle();
    }
}

/**
 * @brief Stops the engine idle
 * @param reason Reason for stopping idle
 */
void StopIdle(const char* reason)
{
    if (systemState == STATE_ON) {
        // Deactivate idle relay
        gpio_set_level(PIN_IDLE_RELAY, 0);
        
        // Update state
        systemState = STATE_OFF;
        
        // LED blink to indicate shutdown (5 quick blinks)
        for (int i = 0; i < 5; i++) {
            gpio_set_level(PIN_STATUS_LED, 1);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            gpio_set_level(PIN_STATUS_LED, 0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        
        ESP_LOGW(TAG, "Idle stopped: %s", reason);
    }
}

/**
 * @brief Starts the engine idle
 */
void StartIdle(void)
{
    if (systemState == STATE_OFF) {
        // Activate idle relay
        gpio_set_level(PIN_IDLE_RELAY, 1);
        
        // Record start time
        idleStartTime = esp_timer_get_time() / 1000; // Convert to milliseconds
        
        // Update state
        systemState = STATE_ON;
        
        // Turn on status LED
        gpio_set_level(PIN_STATUS_LED, 1);
        
        ESP_LOGI(TAG, "Idle started");
        
        // Safety warning
        ESP_LOGW(TAG, "SAFETY WARNING: For manual transmission vehicles, ensure vehicle is in neutral with parking brake engaged to prevent accidents");
    }
}

/**
 * @brief Processes commands received via Bluetooth
 * @param command Received command
 */
void ProcessBluetoothCommand(char *command)
{
    if (strcmp(command, "STOP") == 0) {
        ESP_LOGI(TAG, "Bluetooth command received: STOP");
        StopIdle("Remote command via Bluetooth");
        remoteControlActive = true;
    } 
    else if (strcmp(command, "START") == 0) {
        ESP_LOGI(TAG, "Bluetooth command received: START");
        // Start is only allowed after pressure verification
        remoteControlActive = true;
        // Next adequate pressure reading will start idle
    } 
    else if (strcmp(command, "IAC_MODE_1") == 0) {
        ESP_LOGI(TAG, "Bluetooth command received: Switch to direct IAC mode");
        iacControlMode = 1;
    }
    else if (strcmp(command, "IAC_MODE_2") == 0) {
        ESP_LOGI(TAG, "Bluetooth command received: Switch to Y-connection IAC mode");
        iacControlMode = 2;
    }
    else if (strcmp(command, "STATUS") == 0) {
        ESP_LOGI(TAG, "Bluetooth command received: STATUS");
        // Send current status via Bluetooth (implementation pending)
    }
}

/**
 * @brief Task handling MAP sensor monitoring
 * @param pvParameters Task parameters (not used)
 */
void MAPMonitorTask(void *pvParameters)
{
    while(1) {
        // Read current pressure
        uint32_t mapPressure = ReadMAPSensor();
        
        // Control IAC valve based on pressure
        ControlIACValve(mapPressure);
        
        // Control idle state
        ControlEngineIdle(mapPressure);
        
        // Sampling period (100ms)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Task handling physical start button
 * @param pvParameters Task parameters (not used)
 */
void StartButtonTask(void *pvParameters)
{
    int previousState = 1; // Assuming pull-up (1=not pressed)
    
    while(1) {
        // Read current button state
        int currentState = gpio_get_level(PIN_START_BUTTON);
        
        // Detect falling edge (button press)
        if (previousState == 1 && currentState == 0) {
            ESP_LOGI(TAG, "Start button pressed");
            
            // Toggle state
            if (systemState == STATE_ON) {
                StopIdle("Physical button pressed");
            } else {
                // Don't start directly, let pressure control decide
                remoteControlActive = true;
                // Next adequate pressure reading will start idle
                ESP_LOGI(TAG, "Start request received, checking conditions...");
            }
            
            // Small delay to avoid bounce
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        // Update previous state
        previousState = currentState;
        
        // Sampling period (20ms)
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * @brief Main function
 */
void app_main(void)
{
    // Show system information
    ESP_LOGI(TAG, "MAP Sensor Based Idle Control System");
    ESP_LOGI(TAG, "Version 1.0.0");
    
    // Chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "ESP32 with %d cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "Silicon revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "%dMB %s flash", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    
    // Initialize timer
    esp_timer_init();
    
    // Configure components
    ConfigureGPIO();
    ESP_LOGI(TAG, "GPIOs configured");
    
    ConfigureADC();
    ESP_LOGI(TAG, "ADC configured");
    
    ConfigurePWM();
    ESP_LOGI(TAG, "PWM configured");
    
    ConfigureBluetooth();
    ESP_LOGI(TAG, "Bluetooth configured");
    
    // Show safety warning
    ESP_LOGW(TAG, "⚠️ IMPORTANT: For manual transmission vehicles, ensure vehicle is in neutral with parking brake engaged before using this system to prevent accidents");
    
    // Create tasks
    xTaskCreate(MAPMonitorTask, "map_monitor", 4096, NULL, 5, NULL);
    xTaskCreate(StartButtonTask, "start_button", 2048, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "System started, monitoring MAP pressure");
}