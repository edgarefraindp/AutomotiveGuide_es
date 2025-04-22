/**
 * @file banqueoEcu1_main.c
 * @brief ECU test bench logic for ESP32
 *
 * This file is part of the AutomotiveGuide_es project.
 *
 * > **Repositorio**: https://github.com/edgarefraindp/AutomotiveGuide_es
 * > **Para donaciones y soporte**: Visite la página del repositorio en GitHub
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

// Definiciones de pines para emulación de sensores
#define PIN_EMU_CKP        GPIO_NUM_16    // Emulación sensor CKP (cigüeñal)
#define PIN_EMU_CMP        GPIO_NUM_17    // Emulación sensor CMP (árbol de levas)
#define PIN_EMU_MAP        GPIO_NUM_18    // Emulación sensor MAP (presión múltiple)
#define PIN_EMU_TPS        GPIO_NUM_19    // Emulación sensor TPS (posición acelerador)
#define PIN_EMU_MAF        GPIO_NUM_21    // Emulación sensor MAF (flujo aire)
#define PIN_EMU_ECT        GPIO_NUM_22    // Emulación sensor ECT (temp. refrigerante)
#define PIN_EMU_IAT        GPIO_NUM_23    // Emulación sensor IAT (temp. aire)

// Definiciones de pines para monitoreo de actuadores
#define PIN_MON_INJ1       GPIO_NUM_25    // Monitoreo inyector 1
#define PIN_MON_INJ2       GPIO_NUM_26    // Monitoreo inyector 2
#define PIN_MON_INJ3       GPIO_NUM_27    // Monitoreo inyector 3
#define PIN_MON_INJ4       GPIO_NUM_32    // Monitoreo inyector 4
#define PIN_MON_COIL       GPIO_NUM_33    // Monitoreo bobina
#define PIN_CTRL_PUMP      GPIO_NUM_34    // Control bomba combustible
#define PIN_COMM           GPIO_NUM_35    // Línea de comunicación

// Constantes para configuración de PWM
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          PIN_EMU_TPS
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // 8192 niveles de resolución
#define LEDC_FREQUENCY          5000              // Frecuencia en Hz

// Configuración de timers para generación de señales
#define TIMER_DIVIDER           16
#define TIMER_SCALE             (TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_CKP_INTERVAL_SEC  0.02              // 50Hz para 1500 RPM (4 tiempos)

// Constantes para control de motor simulado
#define RPM_MIN                 800
#define RPM_MAX                 6000
#define RPM_DEFAULT             1000
#define TPS_MIN                 0
#define TPS_MAX                 100
#define TPS_DEFAULT             15                // 15% TPS = ralentí
#define MAP_MIN                 20                // 20 kPa (alta succión)
#define MAP_MAX                 101               // 101 kPa (presión atmosférica)
#define MAF_MIN                 2                 // g/s a ralentí
#define MAF_MAX                 150               // g/s a plena carga
#define ECT_MIN                 -40               // Temperatura mínima (°C)
#define ECT_MAX                 120               // Temperatura máxima (°C)
#define ECT_DEFAULT             85                // Temperatura operación normal (°C)
#define IAT_DEFAULT             25                // Temperatura ambiente predeterminada (°C)

// Buffer para guardado de datos
#define DATA_BUFFER_SIZE        1024

// Tamaño de cola para eventos
#define QUEUE_SIZE              10

// Constantes para detección de anomalías
#define PULSE_MARGIN            0.15              // 15% margen de error aceptable
#define MIN_VALID_PULSE_US      50                // Pulso mínimo válido (μs)

static const char *TAG = "BANQUEO_ECU";

// Estructura para almacenar parámetros de motor simulado
typedef struct {
    uint16_t rpm;                // RPM del motor
    uint8_t tps;                 // Posición del acelerador (0-100%)
    uint8_t map;                 // Presión MAP (kPa)
    uint8_t maf;                 // Flujo MAF (g/s)
    int8_t ect;                  // Temperatura refrigerante (°C)
    int8_t iat;                  // Temperatura aire (°C)
    bool engineRunning;          // Estado del motor (encendido/apagado)
} EngineParams;

// Estructura para almacenar parámetros de pulsos de salida
typedef struct {
    uint32_t inj1PulseWidth;     // Ancho de pulso inyector 1 (μs)
    uint32_t inj2PulseWidth;     // Ancho de pulso inyector 2 (μs)
    uint32_t inj3PulseWidth;     // Ancho de pulso inyector 3 (μs)
    uint32_t inj4PulseWidth;     // Ancho de pulso inyector 4 (μs)
    uint32_t coilDwell;          // Tiempo dwell de bobina (μs)
    uint32_t lastUpdateTime;     // Última actualización (ms)
    bool inj1Active;             // Estado del inyector 1
    bool inj2Active;             // Estado del inyector 2
    bool inj3Active;             // Estado del inyector 3
    bool inj4Active;             // Estado del inyector 4
    bool coilActive;             // Estado de la bobina
    bool pumpActive;             // Estado de la bomba
} OutputSignals;

// Enumeración para tipos de eventos de pulso
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

// Estructura para eventos de pulso
typedef struct {
    PulseEventType type;
    uint64_t timestamp;          // Timestamp en microsegundos
} PulseEvent;

// Variables globales
static EngineParams engineParams = {
    .rpm = RPM_DEFAULT,
    .tps = TPS_DEFAULT,
    .map = MAP_MAX - 20,         // ~80 kPa en ralentí
    .maf = MAF_MIN + 3,          // 5 g/s en ralentí
    .ect = ECT_DEFAULT,
    .iat = IAT_DEFAULT,
    .engineRunning = true
};

static OutputSignals outputSignals = {0};
static uint64_t lastCkpPulseTime = 0;
static QueueHandle_t pulseEventQueue = NULL;

// Variables para emulación de dientes de CKP (tipo 60-2)
static const int TOTAL_TEETH = 60;
static const int MISSING_TEETH = 2;
static int currentTooth = 0;

/**
 * Configura el canal PWM para emular sensores analógicos
 * 
 * @return ESP_OK si la configuración fue exitosa
 */
static esp_err_t ConfigurePwm(void)
{
    // Configuración del timer para PWM
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configuración del canal para TPS
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
 * Configura los pines GPIO para emulación de sensores y monitoreo
 * 
 * @return ESP_OK si la configuración fue exitosa
 */
static esp_err_t ConfigureGPIO(void)
{
    // Configurar pines para emulación de sensores digitales
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
    
    // Configurar pines para monitoreo de actuadores con interrupciones
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
    
    // Instalar el servicio de interrupción GPIO
    gpio_install_isr_service(0);
    
    return ESP_OK;
}

/**
 * Configura el UART para la comunicación con terminal
 * 
 * @return ESP_OK si la configuración fue exitosa
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
    
    // Configurar UART para comunicación con terminal
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    return ESP_OK;
}

/**
 * Configura el timer para generación de señales CKP/CMP
 * 
 * @return ESP_OK si la configuración fue exitosa
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
    
    // Calcular intervalo para la señal CKP según RPM
    double ckp_interval = (60.0 / engineParams.rpm) / (TOTAL_TEETH - MISSING_TEETH);
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, ckp_interval * TIMER_SCALE));
    
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
    ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));
    
    return ESP_OK;
}

/**
 * Genera valor PWM de acuerdo al valor del sensor y su rango
 * 
 * @param value Valor actual del sensor
 * @param minValue Valor mínimo del rango del sensor
 * @param maxValue Valor máximo del rango del sensor
 * @return Valor de duty cycle (0-8191)
 */
static uint32_t CalculatePwmDuty(int value, int minValue, int maxValue)
{
    const uint32_t MAX_DUTY = (1 << LEDC_DUTY_RES) - 1;
    
    // Limitar el valor al rango
    if (value < minValue) value = minValue;
    if (value > maxValue) value = maxValue;
    
    // Calcular porcentaje (0.0 - 1.0)
    float percentage = (float)(value - minValue) / (float)(maxValue - minValue);
    
    // Calcular duty cycle (inversamente proporcional para algunos sensores como NTC)
    return (uint32_t)(percentage * MAX_DUTY);
}

/**
 * Actualiza la señal del sensor TPS
 */
static void UpdateTpsSensor(void)
{
    uint32_t duty = CalculatePwmDuty(engineParams.tps, TPS_MIN, TPS_MAX);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

/**
 * Actualiza la señal del sensor MAP (implementada con PWM)
 */
static void UpdateMapSensor(void)
{
    // Para MAP utilizamos frecuencia de PWM variable según presión
    // 100 Hz más alto por cada 10 kPa adicionales
    int frequency = 100 + (engineParams.map - MAP_MIN) * 10;
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency);
}

/**
 * Actualiza la emulación del sensor MAF
 */
static void UpdateMafSensor(void)
{
    // Implementación básica - en un sistema real se usaría DAC
    // Para emulación simple usamos la salida PWM con duty cycle
    uint32_t duty = CalculatePwmDuty(engineParams.maf, MAF_MIN, MAF_MAX);
    
    // Aquí se implementaría la salida a un DAC externo o PWM filtrado
}

/**
 * Emula la resistencia del sensor ECT (NTC)
 */
static void UpdateEctSensor(void)
{
    // Para sensores de temperatura se usaría un potenciómetro digital o red de resistencias
    // Esta es una implementación simulada para fines de demostración
    float resistance = 0;
    
    // Cálculo simplificado de resistencia NTC
    if (engineParams.ect > -40 && engineParams.ect < 150) {
        // Fórmula simplificada: R = R0 * e^(B*(1/T - 1/T0))
        // Donde R0 = 2.5kOhm a 25°C, B = 3380
        float t1 = 1.0f / (273.15f + (float)engineParams.ect);
        float t0 = 1.0f / (273.15f + 25.0f);  // Temperatura de referencia (25°C)
        resistance = 2500.0f * expf(3380.0f * (t1 - t0));
    }
    
    ESP_LOGI(TAG, "ECT: %d°C, Resistencia: %.2f Ohm", engineParams.ect);
}

/**
 * Emula la resistencia del sensor IAT (NTC)
 */
static void UpdateIatSensor(void)
{
    // Implementación similar a ECT
    float resistance = 0;
    
    if (engineParams.iat > -40 && engineParams.iat < 150) {
        float t1 = 1.0f / (273.15f + (float)engineParams.iat);
        float t0 = 1.0f / (273.15f + 25.0f);
        resistance = 2500.0f * expf(3380.0f * (t1 - t0));
    }
    
    ESP_LOGI(TAG, "IAT: %d°C, Resistencia: %.2f Ohm", engineParams.iat);
}

/**
 * Handler para el timer de generación de señales CKP/CMP
 * 
 * @param para Puntero a parámetros (no utilizado)
 */
static void IRAM_ATTR TimerGroupIsr(void *para)
{
    // Limpiar bandera de interrupción
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    
    // Incrementar posición de diente
    currentTooth = (currentTooth + 1) % TOTAL_TEETH;
    
    // Verificar si estamos en la posición de dientes faltantes (58-59 en un 60-2)
    bool generatePulse = true;
    if (currentTooth >= (TOTAL_TEETH - MISSING_TEETH)) {
        generatePulse = false;
    }
    
    // Generar pulso CKP
    if (engineParams.engineRunning) {
        gpio_set_level(PIN_EMU_CKP, generatePulse ? 1 : 0);
        
        // En determinado diente, generar también pulso CMP (por ejemplo, en el diente 10)
        if (currentTooth == 10) {
            gpio_set_level(PIN_EMU_CMP, 1);
        } else if (currentTooth == 11) {
            gpio_set_level(PIN_EMU_CMP, 0);
        }
    }
    
    // Recalcular intervalo basado en RPM actuales
    double ckp_interval = (60.0 / engineParams.rpm) / (TOTAL_TEETH - MISSING_TEETH);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, ckp_interval * TIMER_SCALE);
}

/**
 * ISR (Interrupt Service Routine) para monitorear pines de entrada
 * 
 * @param arg Argumento personalizado (número de pin en este caso)
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
 * Actualiza todos los valores de los sensores emulados
 */
static void UpdateAllSensors(void)
{
    UpdateTpsSensor();
    UpdateMapSensor();
    UpdateMafSensor();
    UpdateEctSensor();
    UpdateIatSensor();
    
    ESP_LOGI(TAG, "Sensores actualizados - RPM: %d, TPS: %d%%, MAP: %dkPa", 
             engineParams.rpm, engineParams.tps, engineParams.map);
}

/**
 * Calcula los valores de sensores en base a relaciones físicas
 * Por ejemplo, a mayor TPS (acelerador), menor MAP (presión de succión)
 */
static void CalculateSensorRelationships(void)
{
    // Correlacionar TPS con MAP (a mayor apertura de acelerador, mayor presión en múltiple)
    if (engineParams.tps < 10) {
        // Ralentí - alta succión
        engineParams.map = MAP_MIN + 15;
    } else if (engineParams.tps > 90) {
        // Acelerador casi a fondo - presión cercana a la atmosférica
        engineParams.map = MAP_MAX - 5;
    } else {
        // Interpolación lineal entre mínimo y máximo
        engineParams.map = MAP_MIN + ((engineParams.tps * (MAP_MAX - MAP_MIN)) / 100);
    }
    
    // Correlacionar TPS con MAF (a mayor apertura, mayor flujo de aire)
    if (engineParams.tps < 10) {
        // Flujo mínimo a ralentí
        engineParams.maf = MAF_MIN + 3;
    } else {
        // Interpolación no lineal (el flujo crece exponencialmente con TPS)
        float tpsPercentage = (float)engineParams.tps / 100.0f;
        float mafRange = MAF_MAX - MAF_MIN;
        engineParams.maf = MAF_MIN + (uint8_t)(mafRange * tpsPercentage * tpsPercentage);
    }
}

/**
 * Procesa los eventos de pulsos de salida de la ECU
 * 
 * @param lastEvent El último evento de pulso detectado
 * @param state Estado actual de las salidas
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
                    ESP_LOGI(TAG, "Inyector 1: %llu μs", pulseWidth);
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
                    ESP_LOGI(TAG, "Inyector 2: %llu μs", pulseWidth);
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
                    ESP_LOGI(TAG, "Inyector 3: %llu μs", pulseWidth);
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
                    ESP_LOGI(TAG, "Inyector 4: %llu μs", pulseWidth);
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
                    ESP_LOGI(TAG, "Dwell Bobina: %llu μs", pulseWidth);
                }
            }
            state->coilActive = false;
            break;
    }
    
    state->lastUpdateTime = esp_timer_get_time() / 1000; // tiempo en ms
}

/**
 * Analiza las salidas de la ECU y detecta anomalías
 * 
 * @param state Estado actual de las señales de salida
 */
static void DetectAnomalies(const OutputSignals* state)
{
    static uint32_t lastAnomalyTime = 0;
    uint32_t currentTime = esp_timer_get_time() / 1000;
    
    // Solo verificar cada segundo para no saturar la consola
    if (currentTime - lastAnomalyTime < 1000) {
        return;
    }
    
    lastAnomalyTime = currentTime;
    
    // 1. Verificar si hay pulsos faltantes en los inyectores
    if (engineParams.engineRunning && engineParams.rpm > 0) {
        if (state->inj1PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALÍA: No hay pulsos en Inyector 1");
        }
        
        if (state->inj2PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALÍA: No hay pulsos en Inyector 2");
        }
        
        if (state->inj3PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALÍA: No hay pulsos en Inyector 3");
        }
        
        if (state->inj4PulseWidth == 0 && currentTime > 2000) {
            ESP_LOGE(TAG, "ANOMALÍA: No hay pulsos en Inyector 4");
        }
    }
    
    // 2. Verificar equilibrio entre inyectores (no deberían tener más de 15% de diferencia)
    if (state->inj1PulseWidth > 0 && state->inj2PulseWidth > 0) {
        float ratio = (float)state->inj1PulseWidth / (float)state->inj2PulseWidth;
        if (ratio < (1.0 - PULSE_MARGIN) || ratio > (1.0 + PULSE_MARGIN)) {
            ESP_LOGW(TAG, "ANOMALÍA: Desequilibrio entre inyector 1 (%lu μs) e inyector 2 (%lu μs)", 
                    state->inj1PulseWidth, state->inj2PulseWidth);
        }
    }
    
    // 3. Verificar que los tiempos de inyección aumenten con TPS alto
    if (engineParams.tps > 50 && state->inj1PulseWidth < 3000) {
        ESP_LOGW(TAG, "ANOMALÍA: TPS alto (%d%%) pero tiempo de inyección bajo (%lu μs)", 
                engineParams.tps, state->inj1PulseWidth);
    }
    
    // 4. Verificar la correlación entre RPM y dwell de bobina
    // A mayor RPM, menor tiempo de dwell
    if (engineParams.rpm > 3000 && state->coilDwell > 4000) {
        ESP_LOGW(TAG, "ANOMALÍA: RPM alto (%d) pero dwell de bobina muy largo (%lu μs)",
                engineParams.rpm, state->coilDwell);
    }
}

/**
 * Tarea para manejar los eventos de pulso desde la ISR
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
static void PulseMonitorTask(void *pvParameters)
{
    PulseEvent event;
    
    // Inicializar timestamps para evitar falsos positivos
    outputSignals.lastUpdateTime = esp_timer_get_time() / 1000;
    
    while (1) {
        if (xQueueReceive(pulseEventQueue, &event, portMAX_DELAY)) {
            // Procesar el evento de pulso
            ProcessPulseEvent(event, &outputSignals);
            
            // Analizar las salidas para detectar anomalías
            DetectAnomalies(&outputSignals);
        }
    }
}

/**
 * Procesa comandos recibidos por el puerto serie
 * 
 * @param cmdBuffer Buffer que contiene el comando recibido
 * @param length Longitud del comando
 * @return true si el comando fue reconocido y procesado
 */
static bool ProcessCommand(const char* cmdBuffer, size_t length)
{
    if (strncmp(cmdBuffer, "rpm:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= RPM_MIN && value <= RPM_MAX) {
            engineParams.rpm = value;
            ESP_LOGI(TAG, "RPM ajustado a: %d", engineParams.rpm);
            return true;
        }
    } else if (strncmp(cmdBuffer, "tps:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= TPS_MIN && value <= TPS_MAX) {
            engineParams.tps = value;
            ESP_LOGI(TAG, "TPS ajustado a: %d%%", engineParams.tps);
            return true;
        }
    } else if (strncmp(cmdBuffer, "ect:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= ECT_MIN && value <= ECT_MAX) {
            engineParams.ect = value;
            ESP_LOGI(TAG, "ECT ajustado a: %d°C", engineParams.ect);
            return true;
        }
    } else if (strncmp(cmdBuffer, "iat:", 4) == 0 && length > 4) {
        int value = atoi(cmdBuffer + 4);
        if (value >= ECT_MIN && value <= ECT_MAX) { // Mismo rango que ECT
            engineParams.iat = value;
            ESP_LOGI(TAG, "IAT ajustado a: %d°C", engineParams.iat);
            return true;
        }
    } else if (strcmp(cmdBuffer, "status") == 0) {
        ESP_LOGI(TAG, "=== Estado del Sistema ===");
        ESP_LOGI(TAG, "RPM: %d", engineParams.rpm);
        ESP_LOGI(TAG, "TPS: %d%%", engineParams.tps);
        ESP_LOGI(TAG, "MAP: %d kPa", engineParams.map);
        ESP_LOGI(TAG, "MAF: %d g/s", engineParams.maf);
        ESP_LOGI(TAG, "ECT: %d°C", engineParams.ect);
        ESP_LOGI(TAG, "IAT: %d°C", engineParams.iat);
        ESP_LOGI(TAG, "Motor: %s", engineParams.engineRunning ? "Encendido" : "Apagado");
        ESP_LOGI(TAG, "Inyector 1: %lu μs", outputSignals.inj1PulseWidth);
        ESP_LOGI(TAG, "Inyector 2: %lu μs", outputSignals.inj2PulseWidth);
        ESP_LOGI(TAG, "Inyector 3: %lu μs", outputSignals.inj3PulseWidth);
        ESP_LOGI(TAG, "Inyector 4: %lu μs", outputSignals.inj4PulseWidth);
        ESP_LOGI(TAG, "Dwell Bobina: %lu μs", outputSignals.coilDwell);
        return true;
    } else if (strcmp(cmdBuffer, "start") == 0) {
        engineParams.engineRunning = true;
        ESP_LOGI(TAG, "Motor arrancado");
        return true;
    } else if (strcmp(cmdBuffer, "stop") == 0) {
        engineParams.engineRunning = false;
        ESP_LOGI(TAG, "Motor detenido");
        return true;
    } else if (strcmp(cmdBuffer, "help") == 0) {
        ESP_LOGI(TAG, "Comandos disponibles:");
        ESP_LOGI(TAG, "  rpm:VALOR     - Ajustar RPM (800-6000)");
        ESP_LOGI(TAG, "  tps:VALOR     - Ajustar posición acelerador (0-100%)");
        ESP_LOGI(TAG, "  ect:VALOR     - Ajustar temperatura refrigerante (-40 a 120°C)");
        ESP_LOGI(TAG, "  iat:VALOR     - Ajustar temperatura aire (-40 a 120°C)");
        ESP_LOGI(TAG, "  start         - Arrancar motor (iniciar señales)");
        ESP_LOGI(TAG, "  stop          - Detener motor (parar señales)");
        ESP_LOGI(TAG, "  status        - Mostrar estado actual");
        ESP_LOGI(TAG, "  help          - Mostrar esta ayuda");
        return true;
    }
    
    return false;
}

/**
 * Tarea para manejar la interfaz de usuario por puerto serie
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
static void SerialInterfaceTask(void *pvParameters)
{
    char cmdBuffer[64];
    size_t cmdIndex = 0;
    
    ESP_LOGI(TAG, "\n=== Banqueo ECU - Interfaz de Control ===");
    ESP_LOGI(TAG, "Escriba 'help' para ver los comandos disponibles");
    
    while (1) {
        // Leer datos del puerto serie
        uint8_t c;
        int len = uart_read_bytes(UART_NUM_0, &c, 1, 20 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            // Eco local del carácter
            uart_write_bytes(UART_NUM_0, (const char*)&c, 1);
            
            if (c == '\n' || c == '\r') {
                // Fin de comando
                uart_write_bytes(UART_NUM_0, "\n", 1);
                if (cmdIndex > 0) {
                    cmdBuffer[cmdIndex] = '\0';
                    if (!ProcessCommand(cmdBuffer, cmdIndex)) {
                        ESP_LOGW(TAG, "Comando no reconocido: %s", cmdBuffer);
                    }
                    cmdIndex = 0;
                }
            } else if (cmdIndex < sizeof(cmdBuffer) - 1) {
                // Agregar carácter al buffer
                cmdBuffer[cmdIndex++] = c;
            }
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/**
 * Tarea para actualizar la simulación del motor
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
static void EngineSimulationTask(void *pvParameters)
{
    while (1) {
        // Actualizar relaciones entre sensores
        CalculateSensorRelationships();
        
        // Actualizar todos los sensores
        UpdateAllSensors();
        
        // Esperar antes de la siguiente actualización
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

/**
 * Inicializa los componentes del sistema
 * 
 * @return ESP_OK si la inicialización fue exitosa
 */
static esp_err_t InitializeSystem(void)
{
    esp_err_t ret;
    
    // Inicializar cola de eventos
    pulseEventQueue = xQueueCreate(QUEUE_SIZE, sizeof(PulseEvent));
    if (!pulseEventQueue) {
        ESP_LOGE(TAG, "Error creando cola de eventos");
        return ESP_FAIL;
    }
    
    // Configurar GPIO
    ret = ConfigureGPIO();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en la configuración de GPIO");
        return ret;
    }
    
    // Configurar UART
    ret = ConfigureUART();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en la configuración de UART");
        return ret;
    }
    
    // Configurar PWM
    ret = ConfigurePwm();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en la configuración de PWM");
        return ret;
    }
    
    // Configurar timer para señales de CKP/CMP
    ret = ConfigureTimer();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en la configuración del timer");
        return ret;
    }
    
    // Adjuntar los handlers de interrupción para los pines de monitoreo
    gpio_isr_handler_add(PIN_MON_INJ1, GpioIsrHandler, (void*) PIN_MON_INJ1);
    gpio_isr_handler_add(PIN_MON_INJ2, GpioIsrHandler, (void*) PIN_MON_INJ2);
    gpio_isr_handler_add(PIN_MON_INJ3, GpioIsrHandler, (void*) PIN_MON_INJ3);
    gpio_isr_handler_add(PIN_MON_INJ4, GpioIsrHandler, (void*) PIN_MON_INJ4);
    gpio_isr_handler_add(PIN_MON_COIL, GpioIsrHandler, (void*) PIN_MON_COIL);
    
    return ESP_OK;
}

/**
 * Punto de entrada principal de la aplicación
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando sistema de banqueo de ECU...");
    
    // Inicializar el sistema
    esp_err_t ret = InitializeSystem();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en la inicialización del sistema");
        return;
    }
    
    // Crear tareas para la aplicación
    xTaskCreate(PulseMonitorTask, "pulse_monitor", 4096, NULL, 10, NULL);
    xTaskCreate(SerialInterfaceTask, "serial_interface", 4096, NULL, 5, NULL);
    xTaskCreate(EngineSimulationTask, "engine_simulation", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Sistema de banqueo de ECU iniciado correctamente");
}