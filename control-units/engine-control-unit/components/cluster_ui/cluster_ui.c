/**
 * @file cluster_ui.c
 * @brief LVGL cluster UI widgets for engine data visualization
 *
 * This file is part of the AutomotiveGuide_es project.
 *
 * > **Repositorio**: https://github.com/edgarefraindp/AutomotiveGuide_es
 * > **Para donaciones y soporte**: Visite la página del repositorio en GitHub
 *
 * @author AutomotiveGuide_es
 * @date April 2025
 */

#include "cluster_ui.h"
#include "lvgl.h"

// Widgets estáticos para el clúster
static lv_obj_t* rpmLabel = NULL;
static lv_obj_t* rpmBar = NULL;
static lv_obj_t* mapLabel = NULL;
static lv_obj_t* mapBar = NULL;
static lv_obj_t* o2Label = NULL;
static lv_obj_t* o2Bar = NULL;
static lv_obj_t* throttleLabel = NULL;
static lv_obj_t* throttleBar = NULL;

void ClusterUiInit(void) {
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
    lv_label_set_text(o2Label, "O2: Pobre");
    lv_obj_align(o2Label, LV_ALIGN_TOP_LEFT, 10, 90);
    o2Bar = lv_bar_create(scr);
    lv_obj_set_size(o2Bar, 300, 20);
    lv_obj_align_to(o2Bar, o2Label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_bar_set_range(o2Bar, 0, 100);

    // Throttle
    throttleLabel = lv_label_create(scr);
    lv_label_set_text(throttleLabel, "Acelerador: 0%");
    lv_obj_align(throttleLabel, LV_ALIGN_TOP_LEFT, 10, 130);
    throttleBar = lv_bar_create(scr);
    lv_obj_set_size(throttleBar, 300, 20);
    lv_obj_align_to(throttleBar, throttleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_bar_set_range(throttleBar, 0, 100);
}

void ClusterUiUpdate(const ClusterUiData* data) {
    char buffer[32];
    // Actualizar RPM
    snprintf(buffer, sizeof(buffer), "RPM: %d", data->rpm);
    lv_label_set_text(rpmLabel, buffer);
    lv_bar_set_value(rpmBar, data->rpm, LV_ANIM_OFF);
    // Actualizar MAP
    snprintf(buffer, sizeof(buffer), "MAP: %d kPa", data->mapKpa);
    lv_label_set_text(mapLabel, buffer);
    lv_bar_set_value(mapBar, data->mapKpa, LV_ANIM_OFF);
    // Actualizar O2
    snprintf(buffer, sizeof(buffer), "O2: %s", data->o2Percent > 50 ? "Rica" : "Pobre");
    lv_label_set_text(o2Label, buffer);
    lv_bar_set_value(o2Bar, data->o2Percent, LV_ANIM_OFF);
    // Actualizar acelerador
    snprintf(buffer, sizeof(buffer), "Acelerador: %d%%", data->throttlePercent);
    lv_label_set_text(throttleLabel, buffer);
    lv_bar_set_value(throttleBar, data->throttlePercent, LV_ANIM_OFF);
}
