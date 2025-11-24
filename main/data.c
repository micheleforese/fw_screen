#include "data.h"
#include "cJSON.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lvgl_ui.h"
#include "lvgl_utils.h"
#include "string.h"

static const char *TAG = "DATA";

AnemometerData anemometerData;
ParticulateMatterData particulateMatterData;

void anemometer_data_default(AnemometerData *anm_data) {
  anm_data->x_kalman = 0;
  anm_data->y_kalman = 0;
  anm_data->z_kalman = 0;

  anm_data->autocalibrazione_asse_x = false;
  anm_data->autocalibrazione_asse_y = false;
  anm_data->autocalibrazione_asse_z = false;

  anm_data->autocalibrazione_misura_x = false;
  anm_data->autocalibrazione_misura_y = false;
  anm_data->autocalibrazione_misura_z = false;

  anm_data->temp_sonica_x = 0;
  anm_data->temp_sonica_y = 0;
  anm_data->temp_sonica_z = 0;
}

void particulate_matter_data_default(ParticulateMatterData *pm_data) {
  pm_data->timestamp = 0;

  pm_data->mass_density_pm_1_0 = 0;
  pm_data->mass_density_pm_1_0 = 0;
  pm_data->mass_density_pm_2_5 = 0;
  pm_data->mass_density_pm_10 = 0;

  pm_data->particle_count_0_5 = 0;
  pm_data->particle_count_1_0 = 0;
  pm_data->particle_count_2_5 = 0;
  pm_data->particle_count_4_0 = 0;
  pm_data->particle_count_10 = 0;

  pm_data->particle_size = 0;

  strcpy(pm_data->mass_density_unit, "");
  strcpy(pm_data->particle_size_unit, "");
  strcpy(pm_data->particle_count_unit, "");
}

bool parse_anemometer_data(cJSON *root, AnemometerData *anm_data) {
  cJSON *cjson_timestamp = cJSON_GetObjectItem(root, "timestamp");
  cJSON *cjson_x_kalman = cJSON_GetObjectItem(root, "x_kalman");
  cJSON *cjson_y_kalman = cJSON_GetObjectItem(root, "y_kalman");
  cJSON *cjson_z_kalman = cJSON_GetObjectItem(root, "z_kalman");
  cJSON *cjson_autocalibrazione_asse_x =
      cJSON_GetObjectItem(root, "autocalibrazione_asse_x");
  cJSON *cjson_autocalibrazione_asse_y =
      cJSON_GetObjectItem(root, "autocalibrazione_asse_y");
  cJSON *cjson_autocalibrazione_asse_z =
      cJSON_GetObjectItem(root, "autocalibrazione_asse_z");
  cJSON *cjson_autocalibrazione_misura_x =
      cJSON_GetObjectItem(root, "autocalibrazione_misura_x");
  cJSON *cjson_autocalibrazione_misura_y =
      cJSON_GetObjectItem(root, "autocalibrazione_misura_y");
  cJSON *cjson_autocalibrazione_misura_z =
      cJSON_GetObjectItem(root, "autocalibrazione_misura_z");

  cJSON *cjson_temp_sonica_x = cJSON_GetObjectItem(root, "temp_sonica_x");
  cJSON *cjson_temp_sonica_y = cJSON_GetObjectItem(root, "temp_sonica_y");
  cJSON *cjson_temp_sonica_z = cJSON_GetObjectItem(root, "temp_sonica_z");

  if (!cJSON_IsNumber(cjson_timestamp)) {
    ESP_LOGI(TAG, "ROOT->timestamp: NOT FOUND");
    return false;
  }

  if (!cJSON_IsNumber(cjson_x_kalman)) {
    ESP_LOGI(TAG, "ROOT->x_kalman: NOT FOUND");
    return false;
  }
  if (!cJSON_IsNumber(cjson_y_kalman)) {
    ESP_LOGI(TAG, "ROOT->y_kalman: NOT FOUND");
    return false;
  }
  if (!cJSON_IsNumber(cjson_z_kalman)) {
    ESP_LOGI(TAG, "ROOT->z_kalman: NOT FOUND");
    return false;
  }

  if (!cJSON_IsBool(cjson_autocalibrazione_asse_x)) {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_asse_x: NOT FOUND");
    return false;
  }
  if (!cJSON_IsBool(cjson_autocalibrazione_asse_y)) {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_asse_y: NOT FOUND");
    return false;
  }
  if (!cJSON_IsBool(cjson_autocalibrazione_asse_z)) {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_asse_z: NOT FOUND");
    return false;
  }

  if (!cJSON_IsBool(cjson_autocalibrazione_misura_x)) {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_misura_x: NOT FOUND");
    return false;
  }
  if (!cJSON_IsBool(cjson_autocalibrazione_misura_y)) {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_misura_y: NOT FOUND");
    return false;
  }
  if (!cJSON_IsBool(cjson_autocalibrazione_misura_z)) {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_misura_z: NOT FOUND");
    return false;
  }

  if (!cJSON_IsNumber(cjson_temp_sonica_x)) {
    ESP_LOGI(TAG, "ROOT->temp_sonica_x: NOT FOUND");
    return false;
  }
  if (!cJSON_IsNumber(cjson_temp_sonica_y)) {
    ESP_LOGI(TAG, "ROOT->temp_sonica_y: NOT FOUND");
    return false;
  }
  if (!cJSON_IsNumber(cjson_temp_sonica_z)) {
    ESP_LOGI(TAG, "ROOT->temp_sonica_z: NOT FOUND");
    return false;
  }

  anm_data->timestamp = (uint32_t)cjson_timestamp->valueint;
  anm_data->x_kalman = (double)cjson_x_kalman->valuedouble;
  anm_data->y_kalman = (double)cjson_y_kalman->valuedouble;
  anm_data->z_kalman = (double)cjson_z_kalman->valuedouble;

  anm_data->autocalibrazione_asse_x =
      cJSON_IsTrue(cjson_autocalibrazione_asse_x);
  anm_data->autocalibrazione_asse_y =
      cJSON_IsTrue(cjson_autocalibrazione_asse_y);
  anm_data->autocalibrazione_asse_z =
      cJSON_IsTrue(cjson_autocalibrazione_asse_z);

  anm_data->autocalibrazione_misura_x =
      cJSON_IsTrue(cjson_autocalibrazione_misura_x);
  anm_data->autocalibrazione_misura_y =
      cJSON_IsTrue(cjson_autocalibrazione_misura_y);
  anm_data->autocalibrazione_misura_z =
      cJSON_IsTrue(cjson_autocalibrazione_misura_z);

  anm_data->temp_sonica_x = (double)cjson_temp_sonica_x->valuedouble;
  anm_data->temp_sonica_y = (double)cjson_temp_sonica_y->valuedouble;
  anm_data->temp_sonica_z = (double)cjson_temp_sonica_z->valuedouble;

  ESP_LOGI(TAG, "ANEMOMETER DATA PARSED OK.");
  return true;
}

bool parse_particulate_matter_data(cJSON *root,
                                   ParticulateMatterData *sps_data) {

  cJSON *cjson_timestamp = cJSON_GetObjectItem(root, "timestamp");

  if (!cJSON_IsNumber(cjson_timestamp)) {
    ESP_LOGI(TAG, "ROOT->timestamp: NOT FOUND");
    return false;
  }

  cJSON *cjson_sensor_data = cJSON_GetObjectItem(root, "sensor_data");

  if (!cJSON_IsObject(cjson_sensor_data)) {
    ESP_LOGI(TAG, "ROOT->timestamp: NOT FOUND");
    return false;
  }

  cJSON *cjson_mass_density =
      cJSON_GetObjectItem(cjson_sensor_data, "mass_density");

  if (!cJSON_IsObject(cjson_mass_density)) {
    ESP_LOGI(TAG, "ROOT->sensor_data->mass_density: NOT FOUND");
    return false;
  }

  cJSON *cjson_md_pm1_0 = cJSON_GetObjectItem(cjson_mass_density, "pm1.0");

  cJSON *cjson_md_pm2_5 = cJSON_GetObjectItem(cjson_mass_density, "pm2.5");

  cJSON *cjson_md_pm4_0 = cJSON_GetObjectItem(cjson_mass_density, "pm4.0");

  cJSON *cjson_md_pm10 = cJSON_GetObjectItem(cjson_mass_density, "pm10");

  if (!cJSON_IsNumber(cjson_md_pm1_0) || !cJSON_IsNumber(cjson_md_pm2_5) ||
      !cJSON_IsNumber(cjson_md_pm4_0) || !cJSON_IsNumber(cjson_md_pm10)) {
    ESP_LOGI(TAG, "ROOT->sensor_data->mass_density->pm(?): NOT FOUND");
    return false;
  }

  cJSON *cjson_particle_count =
      cJSON_GetObjectItem(cjson_sensor_data, "particle_count");

  if (!cJSON_IsObject(cjson_particle_count)) {
    ESP_LOGI(TAG, "ROOT->sensor_data->particle_count: NOT FOUND");
    return false;
  }
  cJSON *cjson_pc_pm0_5 = cJSON_GetObjectItem(cjson_particle_count, "pm0.5");
  cJSON *cjson_pc_pm1_0 = cJSON_GetObjectItem(cjson_particle_count, "pm1.0");

  cJSON *cjson_pc_pm2_5 = cJSON_GetObjectItem(cjson_particle_count, "pm2.5");

  cJSON *cjson_pc_pm4_0 = cJSON_GetObjectItem(cjson_particle_count, "pm4.0");

  cJSON *cjson_pc_pm10 = cJSON_GetObjectItem(cjson_particle_count, "pm10");

  if (!cJSON_IsNumber(cjson_pc_pm0_5) || !cJSON_IsNumber(cjson_pc_pm1_0) ||
      !cJSON_IsNumber(cjson_pc_pm2_5) || !cJSON_IsNumber(cjson_pc_pm4_0) ||
      !cJSON_IsNumber(cjson_pc_pm10)) {
    ESP_LOGI(TAG, "ROOT->sensor_data->particle_count->pm(?): NOT FOUND");
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

  if (!cJSON_IsNumber(cjson_particle_size)) {
    ESP_LOGI(TAG, "ROOT->particle_size: NOT FOUND");
    return false;
  }

  if (!cJSON_IsString(cjson_mass_density_unit)) {
    ESP_LOGI(TAG, "ROOT->mass_density_unit: NOT FOUND");
    return false;
  }
  if (!cJSON_IsString(cjson_particle_count_unit)) {
    ESP_LOGI(TAG, "ROOT->particle_count_unit: NOT FOUND");
    return false;
  }
  if (!cJSON_IsString(cjson_particle_size_unit)) {
    ESP_LOGI(TAG, "ROOT->particle_size_unit: NOT FOUND");
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
    lvgl_update_anemometer_data(&anemometerData);
    break;
  case PRC_UPDATE_PARTICULATE_MATTER:
    lvgl_update_particulate_matter_data(&particulateMatterData);
    break;
  case PRC_PARSING_ERROR:
    ESP_LOGW(TAG, "Failed to parse data");
    break;

  default:
    ESP_LOGE(TAG, "WTF!");
    break;
  }
}