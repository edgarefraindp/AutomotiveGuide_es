/**
 * @file cluster_ui.h
 * @brief LVGL cluster UI header for engine data visualization
 *
 * Este archivo forma parte del proyecto AutomotiveGuide_es.
 *
 * > **Repositorio**: https://github.com/edgarefraindp/AutomotiveGuide_es  
 * > **Para donaciones y soporte**: Visite la página del repositorio en GitHub
 *
 * @author AutomotiveGuide_es
 * @date April 2025
 */

#ifndef CLUSTER_UI_H
#define CLUSTER_UI_H

#include "lvgl.h"

// Estructura para los datos del clúster
typedef struct {
    int rpm;
    int mapKpa;
    int o2Percent;
    int throttlePercent;
} ClusterUiData;

// Inicializa los gráficos del clúster en la pantalla
void ClusterUiInit(void);

// Actualiza los valores mostrados en el clúster
void ClusterUiUpdate(const ClusterUiData* data);

#endif // CLUSTER_UI_H
