#pragma once

#include "data.h"
#include "esp_log.h"
#include "lvgl.h"
#include "screen.h"

typedef struct WindLabels {
  lv_obj_t *timestamp;

  lv_obj_t *x_kalman;
  lv_obj_t *autocalibrazione_asse_x;
  lv_obj_t *autocalibrazione_misura_x;
  lv_obj_t *temp_sonica_x;

  lv_obj_t *y_kalman;
  lv_obj_t *autocalibrazione_asse_y;
  lv_obj_t *autocalibrazione_misura_y;
  lv_obj_t *temp_sonica_y;

  lv_obj_t *z_kalman;
  lv_obj_t *autocalibrazione_asse_z;
  lv_obj_t *autocalibrazione_misura_z;
  lv_obj_t *temp_sonica_z;
} WindLabels;

extern WindLabels windLabels;

typedef struct ParticulateMatterLabels {
  lv_obj_t *timestamp;

  lv_obj_t *mass_density_pm_1_0;
  lv_obj_t *mass_density_pm_2_5;
  lv_obj_t *mass_density_pm_4_0;
  lv_obj_t *mass_density_pm_10;

  lv_obj_t *particle_count_0_5;
  lv_obj_t *particle_count_1_0;
  lv_obj_t *particle_count_2_5;
  lv_obj_t *particle_count_4_0;
  lv_obj_t *particle_count_10;

  lv_obj_t *particle_size;
} ParticulateMatterLabels;

extern ParticulateMatterLabels particulateMatterLabels;

void lvgl_update_anemometer_data(const AnemometerData *anm_data);
void lvgl_update_particulate_matter_data(const ParticulateMatterData *pm_data);
void lvgl_anemometer_ui_init(lv_obj_t *parent);