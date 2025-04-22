/**
 * @file pulsadorInyectores_main.c
 * @brief Injector pulser logic for ESP32
 *
 * This file is part of the AutomotiveGuide_es project.
 *
 * > **Repositorio**: https://github.com/edgarefraindp/AutomotiveGuide_es
 * > **Para donaciones y soporte**: Visite la página del repositorio en GitHub
 *
 * @author AutomotiveGuide_es
 * @date April 2025
 *
 * Este programa simula señales de pulsos para 4 inyectores de combustible
 * utilizando diferentes órdenes de encendido comunes en motores de 4 cilindros.
 * 
 * La velocidad de pulso es controlada por un potenciómetro conectado
 * a una entrada analógica, simulando la posición del acelerador.
 * 
 * El programa aprovecha características específicas del ESP32 como el ADC 
 * de mayor resolución y el PWM configurable para un control más preciso.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_adc_cal.h"
#include "driver/uart.h"

// Etiqueta para los mensajes de log
static const char *TAG = "PULSADOR_INYECTORES";

// DEFINICIONES DE PINES
#define PIN_INJECTOR_1    GPIO_NUM_16    // Pin para Inyector #1
#define PIN_INJECTOR_2    GPIO_NUM_17    // Pin para Inyector #2
#define PIN_INJECTOR_3    GPIO_NUM_18    // Pin para Inyector #3
#define PIN_INJECTOR_4    GPIO_NUM_19    // Pin para Inyector #4
#define PIN_THROTTLE      ADC1_CHANNEL_6 // ADC1 canal 6 (GPIO34)
#define PIN_ORDER_BUTTON  GPIO_NUM_5     // Botón para cambiar orden de encendido
#define PIN_STATUS_LED    GPIO_NUM_2     // LED indicador de estado

// PARÁMETROS DEL SISTEMA
#define MIN_PULSE_TIME    2     // Tiempo mínimo de pulso en ms (acelerador al mínimo)
#define MAX_PULSE_TIME    15    // Tiempo máximo de pulso en ms (acelerador al máximo)
#define MIN_CYCLE_TIME    20    // Tiempo mínimo entre ciclos en ms (RPM máximo)
#define MAX_CYCLE_TIME    200   // Tiempo máximo entre ciclos en ms (RPM mínimo)
#define ADC_RESOLUTION    4095  // Resolución del ADC del ESP32 (12 bits = 4095)

// UART para comunicación serial
#define UART_PORT         UART_NUM_0
#define UART_TX_PIN       GPIO_NUM_1
#define UART_RX_PIN       GPIO_NUM_3
#define UART_BUF_SIZE     1024

// ÓRDENES DE ENCENDIDO PARA MOTORES DE 4 CILINDROS
// Cada orden es un array con la secuencia de inyectores que el sistema debe activar
static const uint8_t ORDER_1342[] = {0, 2, 3, 1}; // Orden 1-3-4-2 (común en muchos motores de 4 cilindros en línea)
static const uint8_t ORDER_1243[] = {0, 1, 3, 2}; // Orden 1-2-4-3 (algunos motores europeos)
static const uint8_t ORDER_1324[] = {0, 2, 1, 3}; // Orden 1-3-2-4 (algunos motores japoneses)

// Variables globales
static uint8_t currentOrder = 0;         // 0=1342, 1=1243, 2=1324
static uint8_t currentPosition = 0;      // Posición actual en la secuencia (0-3)
static int pulseWidth = 5;               // Ancho de pulso inicial en ms
static int cycleTime = 100;              // Tiempo inicial entre ciclos en ms
static bool orderChanged = false;        // Bandera para detectar cambio de orden
static uint64_t lastDebounceTime = 0;    // Para debounce del botón
static uint64_t previousTime = 0;        // Para control de tiempo
#define DEBOUNCE_DELAY 50               // Tiempo de debounce en ms

// Calibración del ADC
static esp_adc_cal_characteristics_t adc_chars;

// Pines de inyectores en un array para fácil acceso
static const gpio_num_t injectorPins[] = {
    PIN_INJECTOR_1,
    PIN_INJECTOR_2,
    PIN_INJECTOR_3,
    PIN_INJECTOR_4
};

// Prototipos de funciones
static void ConfigureInjectorPins(void);
static void ConfigureADC(void);
static void ConfigureUART(void);
static void PrintCurrentOrder(void);
static void ActivateNextInjector(void);
static void ActivateInjector(uint8_t injectorNum);
static int ReadThrottle(void);
static uint64_t GetMillis(void);
static void SendUART(const char* data);

// Tarea principal del pulsador de inyectores
void InjectorPulserTask(void* pvParameters) {
    int throttleValue;
    int reading;
    uint64_t currentTime;
    
    // El sistema inicia con un mensaje en el UART
    SendUART("Simulador de Pulsos para Inyectores - Versión ESP-IDF\r\n");
    SendUART("-------------------------------------------------\r\n");
    PrintCurrentOrder();
    
    while (1) {
        // Es necesario leer el potenciómetro y ajustar tiempo de ciclo y ancho de pulso
        throttleValue = ReadThrottle();
        
        // Se debe mapear el valor del acelerador a tiempo de ciclo y pulso
        // Nota: Cuando el acelerador aumenta, el tiempo de ciclo disminuye (RPM aumenta)
        cycleTime = (MAX_CYCLE_TIME - MIN_CYCLE_TIME) * (ADC_RESOLUTION - throttleValue) / ADC_RESOLUTION + MIN_CYCLE_TIME;
        
        // El ancho de pulso aumenta con el acelerador (más combustible)
        pulseWidth = (MAX_PULSE_TIME - MIN_PULSE_TIME) * throttleValue / ADC_RESOLUTION + MIN_PULSE_TIME;
        
        // El sistema verifica si se presionó el botón para cambiar el orden con debounce
        reading = gpio_get_level(PIN_ORDER_BUTTON);
        
        // Si el estado del botón ha cambiado (activo bajo)
        if (reading == 0 && !orderChanged) {
            // Se aplica debounce para evitar lecturas falsas
            if ((GetMillis() - lastDebounceTime) > DEBOUNCE_DELAY) {
                orderChanged = true;
                currentOrder = (currentOrder + 1) % 3; // El sistema rota entre las 3 órdenes
                PrintCurrentOrder();
                lastDebounceTime = GetMillis();
            }
        }
        
        // Es necesario resetear la bandera cuando se suelta el botón
        if (reading == 1) {
            orderChanged = false;
        }
        
        // Se realiza el control de tiempo para la secuencia de inyección
        currentTime = GetMillis();
        if (currentTime - previousTime >= cycleTime) {
            previousTime = currentTime;
            ActivateNextInjector();
        }
        
        // Pequeña pausa para no saturar el CPU
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// Función principal
void app_main(void) {
    // Es necesario configurar los pines GPIO, ADC y UART
    ConfigureInjectorPins();
    ConfigureADC();
    ConfigureUART();
    
    // Se informa el inicio del sistema
    ESP_LOGI(TAG, "Iniciando Pulsador de Inyectores");
    
    // Se crea la tarea principal
    xTaskCreate(InjectorPulserTask, "injector_pulser_task", 4096, NULL, 5, NULL);
}

// Configura los pines GPIO
static void ConfigureInjectorPins(void) {
    gpio_config_t io_conf;
    
    // Es necesario configurar pines de inyectores como salidas
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_INJECTOR_1) | (1ULL << PIN_INJECTOR_2) | 
                          (1ULL << PIN_INJECTOR_3) | (1ULL << PIN_INJECTOR_4) | 
                          (1ULL << PIN_STATUS_LED);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    // Es necesario configurar el botón de cambio de orden como entrada con pull-up
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_ORDER_BUTTON);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    
    // Es importante inicializar todos los inyectores a apagado
    for (int i = 0; i < 4; i++) {
        gpio_set_level(injectorPins[i], 0);
    }
    gpio_set_level(PIN_STATUS_LED, 0);
}

// Configura el ADC para la lectura del potenciómetro
static void ConfigureADC(void) {
    // Es importante configurar el atenuador ADC para el rango de 0-3.3V
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(PIN_THROTTLE, ADC_ATTEN_DB_11);
    
    // Se debe calibrar el ADC para obtener lecturas más precisas
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
}

// Configura el UART para la comunicación serial
static void ConfigureUART(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    
    // Es necesario instalar el driver UART
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, UART_BUF_SIZE, UART_BUF_SIZE, 0, NULL, 0);
}

// Lee el valor del potenciómetro (acelerador)
static int ReadThrottle(void) {
    uint32_t adc_reading = 0;
    
    // Es recomendable realizar múltiples lecturas y promediar para estabilidad
    for (int i = 0; i < 10; i++) {
        adc_reading += adc1_get_raw(PIN_THROTTLE);
    }
    adc_reading /= 10;
    
    // Se puede convertir a voltaje si es necesario
    // uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    
    return adc_reading;
}

// Obtiene el tiempo actual en milisegundos
static uint64_t GetMillis(void) {
    return esp_timer_get_time() / 1000;
}

// Envía datos por UART
static void SendUART(const char* data) {
    uart_write_bytes(UART_PORT, data, strlen(data));
}

// Muestra el orden de encendido actual en el monitor serial
static void PrintCurrentOrder(void) {
    char buffer[50];
    
    sprintf(buffer, "Orden de encendido: ");
    SendUART(buffer);
    
    switch(currentOrder) {
        case 0:
            SendUART("1-3-4-2\r\n");
            break;
        case 1:
            SendUART("1-2-4-3\r\n");
            break;
        case 2:
            SendUART("1-3-2-4\r\n");
            break;
    }
}

// Activa un inyector específico
static void ActivateInjector(uint8_t injectorNum) {
    if (injectorNum < 4) {
        gpio_set_level(injectorPins[injectorNum], 1);
    }
}

// Activa el siguiente inyector en la secuencia según el orden seleccionado
static void ActivateNextInjector(void) {
    char buffer[100];
    uint8_t injector = 0;
    
    // El sistema debe apagar todos los inyectores primero
    for (int i = 0; i < 4; i++) {
        gpio_set_level(injectorPins[i], 0);
    }
    
    // Se determina qué inyector activar basado en el orden actual
    switch(currentOrder) {
        case 0: // Orden 1-3-4-2
            injector = ORDER_1342[currentPosition];
            break;
        case 1: // Orden 1-2-4-3
            injector = ORDER_1243[currentPosition];
            break;
        case 2: // Orden 1-3-2-4
            injector = ORDER_1324[currentPosition];
            break;
    }
    
    // Se activa el inyector correspondiente
    ActivateInjector(injector);
    
    // Es necesario parpadear el LED de estado para visualización
    gpio_set_level(PIN_STATUS_LED, 1);
    
    // El sistema avanza a la siguiente posición en la secuencia
    currentPosition = (currentPosition + 1) % 4;
    
    // Se muestra información en el monitor serial
    sprintf(buffer, "Inyector: %d | Ciclo: %d ms | Pulso: %d ms\r\n", 
            injector + 1, // +1 para mostrar 1-4 en lugar de 0-3
            cycleTime, 
            pulseWidth);
    SendUART(buffer);
    
    // Es recomendable apagar el LED después del tiempo de pulso
    vTaskDelay(pulseWidth / portTICK_PERIOD_MS);
    gpio_set_level(PIN_STATUS_LED, 0);
}