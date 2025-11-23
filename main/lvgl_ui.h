#pragma once

#include "esp_log.h"
#include "lvgl.h"
#include "screen.h"

typedef struct WindLabels {
  lv_obj_t *timestamp;

  lv_obj_t *x_kalman;
  lv_obj_t *x_axe_autocalibration;
  lv_obj_t *x_measure_autocalibration;
  lv_obj_t *x_sonic_temp;

  lv_obj_t *y_kalman;
  lv_obj_t *y_axe_autocalibration;
  lv_obj_t *y_measure_autocalibration;
  lv_obj_t *y_sonic_temp;

  lv_obj_t *z_kalman;
  lv_obj_t *z_axe_autocalibration;
  lv_obj_t *z_measure_autocalibration;
  lv_obj_t *z_sonic_temp;
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

  lv_obj_t *mass_density_unit;
  lv_obj_t *particle_count_unit;
  lv_obj_t *particle_size_unit;
} ParticulateMatterLabels;

extern ParticulateMatterLabels particulateMatterLabels;

void lvgl_update_anemometer_data();
void lvgl_update_particulate_matter_data();
void lvgl_anemometer_ui_init(lv_obj_t *parent);