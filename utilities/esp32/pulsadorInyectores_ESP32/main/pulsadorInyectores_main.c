/**
 * @file pulsadorInyectores_main.c
 * @brief Injector pulser logic for ESP32
 *
 * This file is part of the AutomotiveGuide_es project.
 *
 * > **Repository**: https://github.com/edgarefraindp/AutomotiveGuide_es
 * > **For donations and support**: Please visit the GitHub repository page
 *
 * @author AutomotiveGuide_es
 * @date April 2025
 *
 * This program simulates pulse signals for 4 fuel injectors
 * using different firing orders common in 4-cylinder engines.
 * 
 * The pulse speed is controlled by a potentiometer connected
 * to an analog input, simulating the throttle position.
 * 
 * The program leverages specific features of the ESP32 such as higher 
 * resolution ADC and configurable PWM for more precise control.
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

// Tag for log messages
static const char *TAG = "INJECTOR_PULSER";

// PIN DEFINITIONS
#define PIN_INJECTOR_1    GPIO_NUM_16    // Pin for Injector #1
#define PIN_INJECTOR_2    GPIO_NUM_17    // Pin for Injector #2
#define PIN_INJECTOR_3    GPIO_NUM_18    // Pin for Injector #3
#define PIN_INJECTOR_4    GPIO_NUM_19    // Pin for Injector #4
#define PIN_THROTTLE      ADC1_CHANNEL_6 // ADC1 channel 6 (GPIO34)
#define PIN_ORDER_BUTTON  GPIO_NUM_5     // Button to change firing order
#define PIN_STATUS_LED    GPIO_NUM_2     // Status indicator LED

// SYSTEM PARAMETERS
#define MIN_PULSE_TIME    2     // Minimum pulse time in ms (throttle at minimum)
#define MAX_PULSE_TIME    15    // Maximum pulse time in ms (throttle at maximum)
#define MIN_CYCLE_TIME    20    // Minimum time between cycles in ms (maximum RPM)
#define MAX_CYCLE_TIME    200   // Maximum time between cycles in ms (minimum RPM)
#define ADC_RESOLUTION    4095  // ESP32 ADC resolution (12 bits = 4095)

// UART for serial communication
#define UART_PORT         UART_NUM_0
#define UART_TX_PIN       GPIO_NUM_1
#define UART_RX_PIN       GPIO_NUM_3
#define UART_BUF_SIZE     1024

// FIRING ORDERS FOR 4-CYLINDER ENGINES
// Each order is an array with the sequence of injectors the system should activate
static const uint8_t ORDER_1342[] = {0, 2, 3, 1}; // Order 1-3-4-2 (common in many inline 4-cylinder engines)
static const uint8_t ORDER_1243[] = {0, 1, 3, 2}; // Order 1-2-4-3 (some European engines)
static const uint8_t ORDER_1324[] = {0, 2, 1, 3}; // Order 1-3-2-4 (some Japanese engines)

// Global variables
static uint8_t currentOrder = 0;         // 0=1342, 1=1243, 2=1324
static uint8_t currentPosition = 0;      // Current position in the sequence (0-3)
static int pulseWidth = 5;               // Initial pulse width in ms
static int cycleTime = 100;              // Initial time between cycles in ms
static bool orderChanged = false;        // Flag to detect order change
static uint64_t lastDebounceTime = 0;    // For button debounce
static uint64_t previousTime = 0;        // For time control
#define DEBOUNCE_DELAY 50               // Debounce time in ms

// ADC calibration
static esp_adc_cal_characteristics_t adc_chars;

// Injector pins in an array for easy access
static const gpio_num_t injectorPins[] = {
    PIN_INJECTOR_1,
    PIN_INJECTOR_2,
    PIN_INJECTOR_3,
    PIN_INJECTOR_4
};

// Function prototypes
static void ConfigureInjectorPins(void);
static void ConfigureADC(void);
static void ConfigureUART(void);
static void PrintCurrentOrder(void);
static void ActivateNextInjector(void);
static void ActivateInjector(uint8_t injectorNum);
static int ReadThrottle(void);
static uint64_t GetMillis(void);
static void SendUART(const char* data);

// Main task for injector pulser
void InjectorPulserTask(void* pvParameters) {
    int throttleValue;
    int reading;
    uint64_t currentTime;
    
    // The system starts with a message on the UART
    SendUART("Injector Pulse Simulator - ESP-IDF Version\r\n");
    SendUART("-------------------------------------------------\r\n");
    PrintCurrentOrder();
    
    while (1) {
        // Read the potentiometer and adjust cycle time and pulse width
        throttleValue = ReadThrottle();
        
        // Map the throttle value to cycle time and pulse width
        // Note: As throttle increases, cycle time decreases (RPM increases)
        cycleTime = (MAX_CYCLE_TIME - MIN_CYCLE_TIME) * (ADC_RESOLUTION - throttleValue) / ADC_RESOLUTION + MIN_CYCLE_TIME;
        
        // Pulse width increases with throttle (more fuel)
        pulseWidth = (MAX_PULSE_TIME - MIN_PULSE_TIME) * throttleValue / ADC_RESOLUTION + MIN_PULSE_TIME;
        
        // Check if the button was pressed to change the order with debounce
        reading = gpio_get_level(PIN_ORDER_BUTTON);
        
        // If the button state has changed (active low)
        if (reading == 0 && !orderChanged) {
            // Apply debounce to avoid false readings
            if ((GetMillis() - lastDebounceTime) > DEBOUNCE_DELAY) {
                orderChanged = true;
                currentOrder = (currentOrder + 1) % 3; // Rotate between the 3 orders
                PrintCurrentOrder();
                lastDebounceTime = GetMillis();
            }
        }
        
        // Reset the flag when the button is released
        if (reading == 1) {
            orderChanged = false;
        }
        
        // Perform time control for the injection sequence
        currentTime = GetMillis();
        if (currentTime - previousTime >= cycleTime) {
            previousTime = currentTime;
            ActivateNextInjector();
        }
        
        // Small pause to avoid CPU saturation
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// Main function
void app_main(void) {
    // Configure GPIO pins, ADC, and UART
    ConfigureInjectorPins();
    ConfigureADC();
    ConfigureUART();
    
    // Log system startup
    ESP_LOGI(TAG, "Starting Injector Pulser");
    
    // Create the main task
    xTaskCreate(InjectorPulserTask, "injector_pulser_task", 4096, NULL, 5, NULL);
}

// Configures GPIO pins
static void ConfigureInjectorPins(void) {
    gpio_config_t io_conf;
    
    // Configure injector pins as outputs
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_INJECTOR_1) | (1ULL << PIN_INJECTOR_2) | 
                          (1ULL << PIN_INJECTOR_3) | (1ULL << PIN_INJECTOR_4) | 
                          (1ULL << PIN_STATUS_LED);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    // Configure the order change button as input with pull-up
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_ORDER_BUTTON);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    
    // Initialize all injectors to off
    for (int i = 0; i < 4; i++) {
        gpio_set_level(injectorPins[i], 0);
    }
    gpio_set_level(PIN_STATUS_LED, 0);
}

// Configures the ADC for potentiometer reading
static void ConfigureADC(void) {
    // Configure ADC attenuator for 0-3.3V range
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(PIN_THROTTLE, ADC_ATTEN_DB_11);
    
    // Calibrate the ADC for more accurate readings
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
}

// Configures UART for serial communication
static void ConfigureUART(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    
    // Install the UART driver
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, UART_BUF_SIZE, UART_BUF_SIZE, 0, NULL, 0);
}

// Reads the potentiometer value (throttle)
static int ReadThrottle(void) {
    uint32_t adc_reading = 0;
    
    // Perform multiple readings and average for stability
    for (int i = 0; i < 10; i++) {
        adc_reading += adc1_get_raw(PIN_THROTTLE);
    }
    adc_reading /= 10;
    
    // Convert to voltage if necessary
    // uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    
    return adc_reading;
}

// Gets the current time in milliseconds
static uint64_t GetMillis(void) {
    return esp_timer_get_time() / 1000;
}

// Sends data via UART
static void SendUART(const char* data) {
    uart_write_bytes(UART_PORT, data, strlen(data));
}

// Displays the current firing order on the serial monitor
static void PrintCurrentOrder(void) {
    char buffer[50];
    
    sprintf(buffer, "Firing Order: ");
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

// Activates a specific injector
static void ActivateInjector(uint8_t injectorNum) {
    if (injectorNum < 4) {
        gpio_set_level(injectorPins[injectorNum], 1);
    }
}

// Activates the next injector in the sequence based on the selected order
static void ActivateNextInjector(void) {
    char buffer[100];
    uint8_t injector = 0;
    
    // Turn off all injectors first
    for (int i = 0; i < 4; i++) {
        gpio_set_level(injectorPins[i], 0);
    }
    
    // Determine which injector to activate based on the current order
    switch(currentOrder) {
        case 0: // Order 1-3-4-2
            injector = ORDER_1342[currentPosition];
            break;
        case 1: // Order 1-2-4-3
            injector = ORDER_1243[currentPosition];
            break;
        case 2: // Order 1-3-2-4
            injector = ORDER_1324[currentPosition];
            break;
    }
    
    // Activate the corresponding injector
    ActivateInjector(injector);
    
    // Blink the status LED for visualization
    gpio_set_level(PIN_STATUS_LED, 1);
    
    // Advance to the next position in the sequence
    currentPosition = (currentPosition + 1) % 4;
    
    // Display information on the serial monitor
    sprintf(buffer, "Injector: %d | Cycle: %d ms | Pulse: %d ms\r\n", 
            injector + 1, // +1 to show 1-4 instead of 0-3
            cycleTime, 
            pulseWidth);
    SendUART(buffer);
    
    // Turn off the LED after the pulse time
    vTaskDelay(pulseWidth / portTICK_PERIOD_MS);
    gpio_set_level(PIN_STATUS_LED, 0);
}