#include "lvgl_ui.h"

WindLabels windLabels;
ParticulateMatterLabels particulateMatterLabels;

void lvgl_update_anemometer_data() {
  if (lvgl_lock(-1)) {
    static char buffer[64];
    snprintf(buffer, 64, "timestamp: %lu", anemometerData.timestamp);
    lv_label_set_text(windLabels.timestamp, buffer);

    snprintf(buffer, 64, "x_kalman: %f", anemometerData.x_kalman);
    lv_label_set_text(windLabels.x_kalman, buffer);

    snprintf(buffer, 64, "y_kalman: %f", anemometerData.y_kalman);
    lv_label_set_text(windLabels.y_kalman, buffer);

    snprintf(buffer, 64, "z_kalman: %f", anemometerData.z_kalman);
    lv_label_set_text(windLabels.z_kalman, buffer);

    ESP_LOGI("UART", "WIND UPDATED");

    lvgl_unlock();
  }
}

void lvgl_update_particulate_matter_data() {

  if (lvgl_lock(-1)) {
    static char buffer[64];

    snprintf(buffer, 64, "timestamp: %lu", particulateMatterData.timestamp);
    lv_label_set_text(particulateMatterLabels.timestamp, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.mass_density_pm_1_0);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_1_0, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.mass_density_pm_2_5);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_2_5, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.mass_density_pm_4_0);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_4_0, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.mass_density_pm_10);
    lv_label_set_text(particulateMatterLabels.mass_density_pm_10, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.particle_count_0_5);
    lv_label_set_text(particulateMatterLabels.particle_count_0_5, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.particle_count_1_0);
    lv_label_set_text(particulateMatterLabels.particle_count_1_0, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.particle_count_2_5);
    lv_label_set_text(particulateMatterLabels.particle_count_2_5, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.particle_count_4_0);
    lv_label_set_text(particulateMatterLabels.particle_count_4_0, buffer);

    snprintf(buffer, 64, "z: %f", particulateMatterData.particle_count_10);
    lv_label_set_text(particulateMatterLabels.particle_count_10, buffer);

    snprintf(buffer, 64, "mass_density_unit: %s",
             particulateMatterData.mass_density_unit);
    lv_label_set_text(particulateMatterLabels.mass_density_unit, buffer);

    snprintf(buffer, 64, "particle_count_unit: %s",
             particulateMatterData.particle_count_unit);
    lv_label_set_text(particulateMatterLabels.particle_count_unit, buffer);

    snprintf(buffer, 64, "particle_size_unit: %s",
             particulateMatterData.particle_size_unit);
    lv_label_set_text(particulateMatterLabels.particle_size_unit, buffer);

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
    usb_json_send(json);
    cJSON_Delete(json);

    ESP_LOGI(TAG, "Power OFF");
  }
}

static void btn_measure_start_handler(lv_event_t *e) {
  static const char *TAG = "EVENT - BTN MEASURE START";
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "command");
    cJSON_AddStringToObject(json, "command", "MEASURE START");
    usb_json_send(json);
    cJSON_Delete(json);

    ESP_LOGI(TAG, "MEASURE START");
  }
}

static void btn_measure_stop_handler(lv_event_t *e) {
  static const char *TAG = "EVENT - BTN MEASURE STOP";
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "command");
    cJSON_AddStringToObject(json, "command", "MEASURE STOP");
    usb_json_send(json);
    cJSON_Delete(json);

    ESP_LOGI(TAG, "MEASURE STOP");
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
  lv_label_set_text(windLabels.timestamp, "timestamp: NOT SET");

  windLabels.x_kalman = lv_label_create(tab1);
  lv_label_set_text(windLabels.x_kalman, "x_kalman: NOT SET");

  windLabels.y_kalman = lv_label_create(tab1);
  lv_label_set_text(windLabels.y_kalman, "y_kalman: NOT SET");

  windLabels.z_kalman = lv_label_create(tab1);
  lv_label_set_text(windLabels.z_kalman, "z_kalman: NOT SET");

  // -------------------------------
  // TAB 2
  // -------------------------------

  lv_obj_set_flex_flow(tab2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(tab2, 20, 0);

  particulateMatterLabels.timestamp = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.timestamp, "timestamp: NOT SET");

  particulateMatterLabels.mass_density_pm_1_0 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.mass_density_pm_1_0,
                    "mass_density_pm_1_0: NOT SET");

  particulateMatterLabels.mass_density_pm_2_5 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.mass_density_pm_2_5,
                    "mass_density_pm_2_5: NOT SET");

  particulateMatterLabels.mass_density_pm_4_0 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.mass_density_pm_4_0,
                    "mass_density_pm_4_0: NOT SET");

  particulateMatterLabels.mass_density_pm_10 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.mass_density_pm_10,
                    "mass_density_pm_10: NOT SET");

  particulateMatterLabels.particle_count_0_5 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.particle_count_0_5,
                    "particle_count_0_5: NOT SET");

  particulateMatterLabels.particle_count_1_0 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.particle_count_1_0,
                    "particle_count_1_0: NOT SET");

  particulateMatterLabels.particle_count_2_5 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.particle_count_2_5,
                    "particle_count_2_5: NOT SET");

  particulateMatterLabels.particle_count_4_0 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.particle_count_4_0,
                    "particle_count_4_0: NOT SET");

  particulateMatterLabels.particle_count_10 = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.particle_count_10,
                    "particle_count_10: NOT SET");

  particulateMatterLabels.mass_density_unit = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.mass_density_unit,
                    "mass_density_unit: NOT SET");

  particulateMatterLabels.particle_count_unit = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.particle_count_unit,
                    "particle_count_unit: NOT SET");

  particulateMatterLabels.particle_size_unit = lv_label_create(tab2);
  lv_label_set_text(particulateMatterLabels.particle_size_unit,
                    "particle_size_unit: NOT SET");

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
