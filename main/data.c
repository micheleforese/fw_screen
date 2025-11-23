#include "data.h"
#include "cJSON.h"
#include "esp_log.h"
#include "lvgl_ui.h"
#include "lvgl_utils.h"
#include "string.h"

static const char *TAG = "DATA";

AnemometerData anemometerData;
ParticulateMatterData particulateMatterData;

bool parse_anemometer_data(cJSON *root, AnemometerData *anm_data) {
  cJSON *cjson_timestamp = cJSON_GetObjectItem(root, "timestamp");
  cJSON *cjson_x_kalman = cJSON_GetObjectItem(root, "x_kalman");
  cJSON *cjson_y_kalman = cJSON_GetObjectItem(root, "y_kalman");
  cJSON *cjson_z_kalman = cJSON_GetObjectItem(root, "z_kalman");
  cJSON *cjson_x_axe_autocalibration =
      cJSON_GetObjectItem(root, "x_axe_autocalibration");
  cJSON *cjson_y_axe_autocalibration =
      cJSON_GetObjectItem(root, "y_axe_autocalibration");
  cJSON *cjson_z_axe_autocalibration =
      cJSON_GetObjectItem(root, "z_axe_autocalibration");
  cJSON *cjson_x_measure_autocalibration =
      cJSON_GetObjectItem(root, "x_measure_autocalibration");
  cJSON *cjson_y_measure_autocalibration =
      cJSON_GetObjectItem(root, "y_measure_autocalibration");
  cJSON *cjson_z_measure_autocalibration =
      cJSON_GetObjectItem(root, "z_measure_autocalibration");

  cJSON *cjson_x_sonic_temp = cJSON_GetObjectItem(root, "x_sonic_temp");
  cJSON *cjson_y_sonic_temp = cJSON_GetObjectItem(root, "y_sonic_temp");
  cJSON *cjson_z_sonic_temp = cJSON_GetObjectItem(root, "z_sonic_temp");

  if (!cJSON_IsNumber(cjson_timestamp) || !cJSON_IsNumber(cjson_x_kalman) ||
      !cJSON_IsNumber(cjson_y_kalman) || !cJSON_IsNumber(cjson_z_kalman)

      || !cJSON_IsBool(cjson_x_axe_autocalibration) ||
      !cJSON_IsBool(cjson_y_axe_autocalibration) ||
      !cJSON_IsBool(cjson_z_axe_autocalibration)

      || !cJSON_IsBool(cjson_x_measure_autocalibration) ||
      !cJSON_IsBool(cjson_y_measure_autocalibration) ||
      !cJSON_IsBool(cjson_z_measure_autocalibration)

      || !cJSON_IsNumber(cjson_x_sonic_temp) ||
      !cJSON_IsNumber(cjson_y_sonic_temp) ||
      !cJSON_IsNumber(cjson_z_sonic_temp)) {
    ESP_LOGI(TAG, "JSON value missing.");
    return false;
  }

  anm_data->timestamp = (uint32_t)cjson_timestamp->valueint;
  anm_data->x_kalman = (bool)cjson_x_kalman->valuedouble;
  anm_data->y_kalman = (bool)cjson_y_kalman->valuedouble;
  anm_data->z_kalman = (bool)cjson_z_kalman->valuedouble;

  ESP_LOGI(TAG, "ANEMOMETER DATA PARSED OK.");
  return true;
}

bool parse_particulate_matter_data(cJSON *root,
                                   ParticulateMatterData *sps_data) {

  cJSON *cjson_timestamp = cJSON_GetObjectItem(root, "timestamp");

  cJSON *cjson_sensor_data = cJSON_GetObjectItem(root, "sensor_data");

  if (!cJSON_IsObject(cjson_sensor_data)) {
    return false;
  }

  cJSON *cjson_mass_density =
      cJSON_GetObjectItem(cjson_sensor_data, "mass_density");

  if (!cJSON_IsObject(cjson_mass_density)) {
    return false;
  }

  cJSON *cjson_md_pm1_0 = cJSON_GetObjectItem(cjson_mass_density, "pm1.0");

  cJSON *cjson_md_pm2_5 = cJSON_GetObjectItem(cjson_mass_density, "pm2_5");

  cJSON *cjson_md_pm4_0 = cJSON_GetObjectItem(cjson_mass_density, "pm4_0");

  cJSON *cjson_md_pm10 = cJSON_GetObjectItem(cjson_mass_density, "pm10");

  if (!cJSON_IsNumber(cjson_md_pm1_0) || !cJSON_IsNumber(cjson_md_pm2_5) ||
      !cJSON_IsNumber(cjson_md_pm4_0) || !cJSON_IsNumber(cjson_md_pm10)) {
    return false;
  }

  cJSON *cjson_particle_count =
      cJSON_GetObjectItem(cjson_sensor_data, "particle_count");

  if (!cJSON_IsObject(cjson_particle_count)) {
    return false;
  }
  cJSON *cjson_pc_pm0_5 = cJSON_GetObjectItem(cjson_particle_count, "pm0_5");
  cJSON *cjson_pc_pm1_0 = cJSON_GetObjectItem(cjson_particle_count, "pm1.0");

  cJSON *cjson_pc_pm2_5 = cJSON_GetObjectItem(cjson_particle_count, "pm2_5");

  cJSON *cjson_pc_pm4_0 = cJSON_GetObjectItem(cjson_particle_count, "pm4_0");

  cJSON *cjson_pc_pm10 = cJSON_GetObjectItem(cjson_particle_count, "pm10");

  if (!cJSON_IsNumber(cjson_pc_pm0_5) || !cJSON_IsNumber(cjson_pc_pm1_0) ||
      !cJSON_IsNumber(cjson_pc_pm2_5) || !cJSON_IsNumber(cjson_pc_pm4_0) ||
      !cJSON_IsNumber(cjson_pc_pm10)) {
    return false;
  }

  cJSON *cjson_particle_size =
      cJSON_GetObjectItem(cjson_sensor_data, "particle_size");

  cJSON *cjson_mass_density_unit =
      cJSON_GetObjectItem(cjson_sensor_data, "mass_density_unit");

  cJSON *cjson_particle_count_unit =
      cJSON_GetObjectItem(cjson_sensor_data, "particle_count_unit");

  cJSON *cjson_particle_size_unit =
      cJSON_GetObjectItem(cjson_sensor_data, "particle_size_unit");

  if (!cJSON_IsNumber(cjson_particle_size) ||
      !cJSON_IsString(cjson_mass_density_unit) ||
      !cJSON_IsString(cjson_particle_count_unit) ||
      !cJSON_IsString(cjson_particle_size_unit)) {
    return false;
  }

  particulateMatterData.timestamp = (uint32_t)cjson_timestamp->valueint;
  particulateMatterData.mass_density_pm_1_0 =
      (double)cjson_md_pm1_0->valuedouble;
  particulateMatterData.mass_density_pm_2_5 =
      (double)cjson_md_pm2_5->valuedouble;
  particulateMatterData.mass_density_pm_4_0 =
      (double)cjson_md_pm4_0->valuedouble;
  particulateMatterData.mass_density_pm_10 = (double)cjson_md_pm10->valuedouble;

  particulateMatterData.particle_count_0_5 =
      (double)cjson_pc_pm0_5->valuedouble;
  particulateMatterData.particle_count_1_0 =
      (double)cjson_pc_pm1_0->valuedouble;
  particulateMatterData.particle_count_2_5 =
      (double)cjson_pc_pm2_5->valuedouble;
  particulateMatterData.particle_count_4_0 =
      (double)cjson_pc_pm4_0->valuedouble;
  particulateMatterData.particle_count_10 = (double)cjson_pc_pm10->valuedouble;

  particulateMatterData.particle_size =
      (double)cjson_particle_size->valuedouble;

  strcpy(particulateMatterData.mass_density_unit,
         cjson_mass_density_unit->valuestring);

  strcpy(particulateMatterData.particle_count_unit,
         cjson_particle_count_unit->valuestring);

  strcpy(particulateMatterData.particle_size_unit,
         cjson_particle_size_unit->valuestring);
  return true;
}

ParseReturnCode parse_data(cJSON *json, AnemometerData *anm_data,
                           ParticulateMatterData *pm_data) {
  static const char *TAG = "PARSE_DATA";

  ESP_LOGI(TAG, "JSON RECEIVED. %s", cJSON_Print(json));

  cJSON *topic = cJSON_GetObjectItem(json, "topic");

  if (!cJSON_IsString(topic)) {
    ESP_LOGI(TAG, "JSON topic missing.");
    return PRC_PARSING_ERROR;
  }

  if (strcmp(topic->valuestring, "anm") == 0) {
    if (parse_anemometer_data(json, anm_data))
      return PRC_UPDATED_ANEMOMETER;
  }

  if (strcmp(topic->valuestring, "sps") == 0) {
    if (parse_particulate_matter_data(json, pm_data))
      return PRC_UPDATE_PARTICULATE_MATTER;
  }

  if (strcmp(topic->valuestring, "type") == 0) {
    ESP_LOGI(TAG, "COMMAND");
  }

  ESP_LOGI(TAG, "Unknown topic: %s", topic->valuestring);
  return PRC_PARSING_ERROR;
}

void on_json_received(cJSON *json) {

  switch (parse_data(json, &anemometerData, &particulateMatterData)) {
  case PRC_UPDATED_ANEMOMETER:
    lvgl_update_anemometer_data();
    break;
  case PRC_UPDATE_PARTICULATE_MATTER:
    lvgl_update_particulate_matter_data();
    break;
  case PRC_PARSING_ERROR:
    ESP_LOGW(TAG, "Failed to parse data");
    break;

  default:
    ESP_LOGE(TAG, "WTF!");
    break;
  }
}