#include "lvgl_ui.h"
#include "data.h"
// #include "lv_conf.h"
#include "lvgl.h"
#include "tusb_cdc.h"

WindLabels windLabels;
ParticulateMatterLabels particulateMatterLabels;

void lvgl_update_anemometer_data(const AnemometerData *anm_data) {
  if (lvgl_lock(-1)) {
    static char buffer[64];
    snprintf(buffer, 64, "Timestamp: %lu", anm_data->timestamp);
    lv_label_set_text(windLabels.timestamp, buffer);

    snprintf(buffer, 64, "X Kalman: %.03f", anm_data->x_kalman);
    lv_label_set_text(windLabels.x_kalman, buffer);

    snprintf(buffer, 64, "X Cal Asse: %s",
             anm_data->autocalibrazione_asse_x ? "True" : "False");
    lv_label_set_text(windLabels.autocalibrazione_asse_x, buffer);

    snprintf(buffer, 64, "X Cal Misura: %s",
             anm_data->autocalibrazione_misura_x ? "True" : "False");
    lv_label_set_text(windLabels.autocalibrazione_misura_x, buffer);

    snprintf(buffer, 64, "X Temp Sonica: %.03f", anm_data->temp_sonica_x);
    lv_label_set_text(windLabels.temp_sonica_x, buffer);

    snprintf(buffer, 64, "Y Kalman: %.03f", anm_data->y_kalman);
    lv_label_set_text(windLabels.y_kalman, buffer);

    snprintf(buffer, 64, "Y Cal Asse: %s",
             anm_data->autocalibrazione_asse_y ? "True" : "False");
    lv_label_set_text(windLabels.autocalibrazione_asse_y, buffer);

    snprintf(buffer, 64, "Y Cal Misura: %s",
             anm_data->autocalibrazione_misura_y ? "True" : "False");
    lv_label_set_text(windLabels.autocalibrazione_misura_y, buffer);

    snprintf(buffer, 64, "Y Temp Sonica: %.03f", anm_data->temp_sonica_y);
    lv_label_set_text(windLabels.temp_sonica_y, buffer);

    snprintf(buffer, 64, "Z Kalmanz: %.03f", anm_data->z_kalman);
    lv_label_set_text(windLabels.z_kalman, buffer);

    snprintf(buffer, 64, "Z Cal Asse: %s",
             anm_data->autocalibrazione_asse_z ? "True" : "False");
    lv_label_set_text(windLabels.autocalibrazione_asse_z, buffer);

    snprintf(buffer, 64, "Z Cal Misura: %s",
             anm_data->autocalibrazione_misura_z ? "True" : "False");
    lv_label_set_text(windLabels.autocalibrazione_misura_z, buffer);

    snprintf(buffer, 64, "Z Temp Sonica: %.03f", anm_data->temp_sonica_z);
    lv_label_set_text(windLabels.temp_sonica_z, buffer);

    ESP_LOGI("UART", "WIND UPDATED");

    lvgl_unlock();
  }
}

void lvgl_update_particulate_matter_data(const ParticulateMatterData *pm_data) {

  if (lvgl_lock(-1)) {
    static char buffer[64];

    snprintf(buffer, 64, "timestamp: %lu", pm_data->timestamp);
    lv_label_set_text(particulateMatterLabels.timestamp, buffer);

    snprintf(buffer, 64, "Mass Density pm1.0: %.03f %s",
             pm_data->mass_density_pm_1_0, pm_data->mass_density_unit);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_1_0, buffer);

    snprintf(buffer, 64, "Mass Density pm2.5: %.03f %s",
             pm_data->mass_density_pm_2_5, pm_data->mass_density_unit);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_2_5, buffer);

    snprintf(buffer, 64, "Mass Density pm4.0: %.03f %s",
             pm_data->mass_density_pm_4_0, pm_data->mass_density_unit);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_4_0, buffer);

    snprintf(buffer, 64, "Mass Density pm10: %.03f %s",
             pm_data->mass_density_pm_10, pm_data->mass_density_unit);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_10, buffer);

    snprintf(buffer, 64, "Particle Count pm0.5: %.03f %s",
             pm_data->particle_count_0_5, pm_data->particle_count_unit);
    lv_label_set_text(particulateMatterLabels.particle_count_0_5, buffer);

    snprintf(buffer, 64, "Particle Count pm1.0: %.03f %s",
             pm_data->particle_count_1_0, pm_data->particle_count_unit);
    lv_label_set_text(particulateMatterLabels.particle_count_1_0, buffer);

    snprintf(buffer, 64, "Particle Count pm2.5: %.03f %s",
             pm_data->particle_count_2_5, pm_data->particle_count_unit);
    lv_label_set_text(particulateMatterLabels.particle_count_2_5, buffer);

    snprintf(buffer, 64, "Particle Count pm4.0: %.03f %s",
             pm_data->particle_count_4_0, pm_data->particle_count_unit);
    lv_label_set_text(particulateMatterLabels.particle_count_4_0, buffer);

    snprintf(buffer, 64, "Particle Count pm10: %.03f %s",
             pm_data->particle_count_10, pm_data->particle_count_unit);
    lv_label_set_text(particulateMatterLabels.particle_count_10, buffer);

    snprintf(buffer, 64, "Particle Size: %.03f %s", pm_data->particle_size,
             pm_data->particle_size_unit);
    lv_label_set_text(particulateMatterLabels.particle_size, buffer);

    lvgl_unlock();
  }

  ESP_LOGI("UART", "PARTICULATE MATTER UPDATED");
}

static void btn_power_off_handler(lv_event_t *e) {
  static const char *TAG = "EVENT - BTN POWER OFF";
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "command");
    cJSON_AddStringToObject(json, "command", "POWER OFF");
    tusb_json_write(json);
    cJSON_Delete(json);
  }
}

static void btn_measure_start_handler(lv_event_t *e) {
  static const char *TAG = "EVENT - BTN MEASURE START";
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "command");
    cJSON_AddStringToObject(json, "command", "MEASURE START");
    tusb_json_write(json);
    cJSON_Delete(json);
  }
}

static void btn_measure_stop_handler(lv_event_t *e) {
  static const char *TAG = "EVENT - BTN MEASURE STOP";
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "command");
    cJSON_AddStringToObject(json, "command", "MEASURE STOP");
    tusb_json_write(json);
    cJSON_Delete(json);
  }
}

void lvgl_anemometer_ui_init(lv_obj_t *parent) {
  static const char *TAG = "LVGL";

  lv_obj_t *tabview;
  tabview = lv_tabview_create(parent, LV_DIR_BOTTOM, 50);

  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");

  // -------------------------------
  // TAB 1
  // -------------------------------

  lv_obj_set_flex_flow(tab1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(tab1, 20, 0);

  windLabels.timestamp = lv_label_create(tab1);

  windLabels.x_kalman = lv_label_create(tab1);
  windLabels.autocalibrazione_asse_x = lv_label_create(tab1);
  windLabels.autocalibrazione_misura_x = lv_label_create(tab1);
  windLabels.temp_sonica_x = lv_label_create(tab1);

  windLabels.y_kalman = lv_label_create(tab1);
  windLabels.autocalibrazione_asse_y = lv_label_create(tab1);
  windLabels.autocalibrazione_misura_y = lv_label_create(tab1);
  windLabels.temp_sonica_y = lv_label_create(tab1);

  windLabels.z_kalman = lv_label_create(tab1);
  windLabels.autocalibrazione_asse_z = lv_label_create(tab1);
  windLabels.autocalibrazione_misura_z = lv_label_create(tab1);
  windLabels.temp_sonica_z = lv_label_create(tab1);

  anemometer_data_default(&anemometerData);

  lvgl_update_anemometer_data(&anemometerData);

  // -------------------------------
  // TAB 2
  // -------------------------------

  lv_obj_set_flex_flow(tab2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(tab2, 20, 0);

  particulateMatterLabels.timestamp = lv_label_create(tab2);
  particulateMatterLabels.mass_density_pm_1_0 = lv_label_create(tab2);
  particulateMatterLabels.mass_density_pm_2_5 = lv_label_create(tab2);
  particulateMatterLabels.mass_density_pm_4_0 = lv_label_create(tab2);
  particulateMatterLabels.mass_density_pm_10 = lv_label_create(tab2);
  particulateMatterLabels.particle_count_0_5 = lv_label_create(tab2);
  particulateMatterLabels.particle_count_1_0 = lv_label_create(tab2);
  particulateMatterLabels.particle_count_2_5 = lv_label_create(tab2);
  particulateMatterLabels.particle_count_4_0 = lv_label_create(tab2);
  particulateMatterLabels.particle_count_10 = lv_label_create(tab2);
  particulateMatterLabels.particle_size = lv_label_create(tab2);

  particulate_matter_data_default(&particulateMatterData);
  lvgl_update_particulate_matter_data(&particulateMatterData);

  // -------------------------------
  // TAB 3
  // -------------------------------

  lv_obj_set_flex_flow(tab3, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(tab2, 20, 0);

  lv_obj_t *power_off_btn = lv_btn_create(tab3);
  lv_obj_add_event_cb(power_off_btn, btn_power_off_handler, LV_EVENT_ALL, NULL);

  lv_tabview_set_act(tabview, 0, LV_ANIM_ON);
  lv_obj_t *power_off_label;
  power_off_label = lv_label_create(power_off_btn);
  lv_label_set_text(power_off_label, "Power OFF");
  lv_obj_center(power_off_label);

  lv_obj_t *measure_start_btn = lv_btn_create(tab3);
  lv_obj_add_event_cb(measure_start_btn, btn_measure_start_handler,
                      LV_EVENT_ALL, NULL);

  lv_tabview_set_act(tabview, 0, LV_ANIM_ON);
  lv_obj_t *measure_start_label;
  measure_start_label = lv_label_create(measure_start_btn);
  lv_label_set_text(measure_start_label, "Measure START");
  lv_obj_center(measure_start_label);

  lv_obj_t *measure_stop_btn = lv_btn_create(tab3);
  lv_obj_add_event_cb(measure_stop_btn, btn_measure_stop_handler, LV_EVENT_ALL,
                      NULL);

  lv_tabview_set_act(tabview, 0, LV_ANIM_ON);
  lv_obj_t *measure_stop_label;
  measure_stop_label = lv_label_create(measure_stop_btn);
  lv_label_set_text(measure_stop_label, "Measure STOP");
  lv_obj_center(measure_stop_label);
}
