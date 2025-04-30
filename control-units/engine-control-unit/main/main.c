/**
 * @file main.c
 * @brief Engine Control Unit main logic for ESP32-S3 with Waveshare RGB LCD
 *
 * This file is part of the AutomotiveGuide_es project.
 *
 * > **Repository**: https://github.com/edgarefraindp/AutomotiveGuide_es
 * > **For donations and support**: Please visit the GitHub repository page
 *
 * @author AutomotiveGuide_es
 * @date April 2025
 */
/*
 * engine_control_unit - Ignition ECU for ESP32-S3 and Waveshare RGB LCD screen
 *
 * Inputs:
 *  - PIN_BUTTON_IGNITION: Ignition button
 *  - PIN_BUTTON_ACCESSORY: Accessory button
 *  - PIN_POT_THROTTLE: Potentiometer for throttle
 *  - PIN_RPM_SENSOR: Digital input for position sensor (RPM)
 *
 * Outputs:
 *  - Pins to simulate ignition coils
 *
 * This code follows the style guidelines of the AutomotiveGuide_es project.
 */
#include <stdlib.h>
#include "esp_timer.h"
#include "waveshare_rgb_lcd_port.h"
#include "cluster_ui/cluster_ui.h"

// Pin definitions
#define PIN_BUTTON_IGNITION    GPIO_NUM_2
#define PIN_BUTTON_ACCESSORY   GPIO_NUM_3
#define PIN_POT_THROTTLE       ADC1_CHANNEL_0 // GPIO1
#define PIN_RPM_SENSOR         GPIO_NUM_4
#define PIN_MAP_SENSOR         ADC1_CHANNEL_3 // GPIO39 (example)
#define PIN_O2_SENSOR          ADC1_CHANNEL_6 // GPIO34 (example)
#define PIN_OLED_SDA           GPIO_NUM_21
#define PIN_OLED_SCL           GPIO_NUM_22
#define PIN_COIL_1             GPIO_NUM_12
#define PIN_COIL_2             GPIO_NUM_13
#define PIN_COIL_3             GPIO_NUM_14
#define PIN_COIL_4             GPIO_NUM_15

// Constants
#define DEBOUNCE_TIME_MS       200
#define ADC_MAX_VALUE          4095
#define THROTTLE_PERCENT_MAX   100
#define RPM_PULSE_TIMEOUT_MS   1000

// Variables
static volatile uint32_t rpmPulseCount = 0;
static volatile uint32_t lastRpmPulseTime = 0;
static bool ignitionOn = false;
static bool accessoryOn = false;

// LVGL widgets to display data
static lv_obj_t* rpmLabel = NULL;
static lv_obj_t* rpmBar = NULL;
static lv_obj_t* mapLabel = NULL;
static lv_obj_t* mapBar = NULL;
static lv_obj_t* o2Label = NULL;
static lv_obj_t* o2Bar = NULL;
static lv_obj_t* throttleLabel = NULL;
static lv_obj_t* throttleBar = NULL;

// Function prototypes
static void IRAM_ATTR RpmSensorIsrHandler(void* arg);
static void ButtonTask(void* arg);
static void ThrottleTask(void* arg);
static void RpmTask(void* arg);
static void DisplayTask(void* arg);
static void InitializeGui(void);

void app_main(void) {
    // Initialize RGB screen and LVGL (includes touch if available)
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());
    ClusterUiInit();

    // Button pin configuration
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_BUTTON_IGNITION) | (1ULL << PIN_BUTTON_ACCESSORY),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // RPM sensor pin configuration
    gpio_config_t rpm_conf = {
        .pin_bit_mask = (1ULL << PIN_RPM_SENSOR),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&rpm_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_RPM_SENSOR, RpmSensorIsrHandler, NULL);

    // ADC configuration for throttle potentiometer and additional sensors
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(PIN_POT_THROTTLE, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(PIN_MAP_SENSOR, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(PIN_O2_SENSOR, ADC_ATTEN_DB_11);

    // Create tasks for buttons, throttle, RPM, and screen
    xTaskCreate(ButtonTask, "ButtonTask", 2048, NULL, 5, NULL);
    xTaskCreate(ThrottleTask, "ThrottleTask", 2048, NULL, 5, NULL);
    xTaskCreate(RpmTask, "RpmTask", 2048, NULL, 5, NULL);
    xTaskCreate(DisplayTask, "DisplayTask", 4096, NULL, 5, NULL);

    ESP_LOGI("ECU", "Engine Control Unit started");
}

// ISR to count pulses from the RPM sensor
tick_type_t lastIsrTick = 0;
static void IRAM_ATTR RpmSensorIsrHandler(void* arg) {
    tick_type_t now = xTaskGetTickCountFromISR();
    if (now - lastIsrTick > pdMS_TO_TICKS(10)) { // Debounce
        rpmPulseCount++;
        lastRpmPulseTime = xTaskGetTickCountFromISR();
        lastIsrTick = now;
    }
}

// Task to read ignition and accessory buttons
static void ButtonTask(void* arg) {
    bool lastIgnition = false;
    bool lastAccessory = false;
    while (1) {
        bool ignitionState = gpio_get_level(PIN_BUTTON_IGNITION) == 0;
        bool accessoryState = gpio_get_level(PIN_BUTTON_ACCESSORY) == 0;
        if (ignitionState != lastIgnition) {
            ignitionOn = ignitionState;
            ESP_LOGI("ECU", "Ignition %s", ignitionOn ? "ON" : "OFF");
            lastIgnition = ignitionState;
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
        }
        if (accessoryState != lastAccessory) {
            accessoryOn = accessoryState;
            ESP_LOGI("ECU", "Accessory %s", accessoryOn ? "ON" : "OFF");
            lastAccessory = accessoryState;
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task to read the throttle potentiometer
static void ThrottleTask(void* arg) {
    while (1) {
        int adcValue = adc1_get_raw(PIN_POT_THROTTLE);
        int throttlePercent = (adcValue * THROTTLE_PERCENT_MAX) / ADC_MAX_VALUE;
        ESP_LOGI("ECU", "Throttle: %d%%", throttlePercent);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Task to calculate and display RPM
static void RpmTask(void* arg) {
    uint32_t lastPulseCount = 0;
    TickType_t lastTime = xTaskGetTickCount();
    while (1) {
        TickType_t now = xTaskGetTickCount();
        uint32_t pulseCount = rpmPulseCount;
        uint32_t deltaPulses = pulseCount - lastPulseCount;
        uint32_t deltaTimeMs = (now - lastTime) * portTICK_PERIOD_MS;
        int rpm = 0;
        if (deltaTimeMs > 0) {
            // Assuming 2 pulses per revolution
            rpm = (deltaPulses * 60000) / (2 * deltaTimeMs);
        }
        ESP_LOGI("ECU", "RPM: %d", rpm);
        lastPulseCount = pulseCount;
        lastTime = now;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Initializes the graphical interface with LVGL
static void InitializeGui(void) {
    // Create a base screen
    lv_obj_t* scr = lv_scr_act();

    // RPM
    rpmLabel = lv_label_create(scr);
    lv_label_set_text(rpmLabel, "RPM: 0");
    lv_obj_align(rpmLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    rpmBar = lv_bar_create(scr);
    lv_obj_set_size(rpmBar, 300, 20);
    lv_obj_align_to(rpmBar, rpmLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_bar_set_range(rpmBar, 0, 7000);

    // MAP
    mapLabel = lv_label_create(scr);
    lv_label_set_text(mapLabel, "MAP: 0 kPa");
    lv_obj_align(mapLabel, LV_ALIGN_TOP_LEFT, 10, 50);
    mapBar = lv_bar_create(scr);
    lv_obj_set_size(mapBar, 300, 20);
    lv_obj_align_to(mapBar, mapLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_bar_set_range(mapBar, 0, 500);

    // O2
    o2Label = lv_label_create(scr);
    lv_label_set_text(o2Label, "O2: Lean");
    lv_obj_align(o2Label, LV_ALIGN_TOP_LEFT, 10, 90);
    o2Bar = lv_bar_create(scr);
    lv_obj_set_size(o2Bar, 300, 20);
    lv_obj_align_to(o2Bar, o2Label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_bar_set_range(o2Bar, 0, 100);

    // Throttle
    throttleLabel = lv_label_create(scr);
    lv_label_set_text(throttleLabel, "Throttle: 0%");
    lv_obj_align(throttleLabel, LV_ALIGN_TOP_LEFT, 10, 130);
    throttleBar = lv_bar_create(scr);
    lv_obj_set_size(throttleBar, 300, 20);
    lv_obj_align_to(throttleBar, throttleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_bar_set_range(throttleBar, 0, 100);
}

// Task to update LVGL widgets with current values
static void DisplayTask(void* arg) {
    ClusterUiData clusterData;
    while (1) {
        int adcThrottle = adc1_get_raw(PIN_POT_THROTTLE);
        clusterData.throttlePercent = (adcThrottle * THROTTLE_PERCENT_MAX) / ADC_MAX_VALUE;
        int adcMap = adc1_get_raw(PIN_MAP_SENSOR);
        clusterData.mapKpa = (adcMap * 500) / ADC_MAX_VALUE; // 0-500 kPa
        int adcO2 = adc1_get_raw(PIN_O2_SENSOR);
        clusterData.o2Percent = (adcO2 * 100) / ADC_MAX_VALUE; // 0-100%
        extern int currentRpm;
        clusterData.rpm = currentRpm;
        ClusterUiUpdate(&clusterData);

        // Refresh LVGL
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
