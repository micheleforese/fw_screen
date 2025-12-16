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
ImuData imuData;

void anemometer_data_default(AnemometerData *anm_data) {
  anm_data->x_vout = 0;
  anm_data->y_vout = 0;
  anm_data->z_vout = 0;

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

void imu_data_default(ImuData *imu_data) {
  imu_data->timestamp = 0;

  imu_data->acc_top_x = 0;
  imu_data->acc_top_y = 0;
  imu_data->acc_top_z = 0;

  imu_data->acc_x = 0;
  imu_data->acc_y = 0;
  imu_data->acc_z = 0;

  imu_data->mag_x = 0;
  imu_data->mag_y = 0;
  imu_data->mag_z = 0;

  imu_data->gyr_x = 0;
  imu_data->gyr_y = 0;
  imu_data->gyr_z = 0;

  strcpy(imu_data->acc_top_unit, "");
  strcpy(imu_data->acc_unit, "");
  strcpy(imu_data->mag_unit, "");
  strcpy(imu_data->gyr_unit, "");
}

bool parse_anemometer_data(cJSON *root, AnemometerData *anm_data) {

  // ----------------------------------------
  // Timestamp
  // ----------------------------------------
  cJSON *cjson_timestamp = cJSON_GetObjectItem(root, "timestamp");

  if (cJSON_IsNumber(cjson_timestamp)) {
    anm_data->timestamp = (double)cjson_timestamp->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->timestamp: NOT FOUND");
  }

  // ----------------------------------------
  // v_out
  // ----------------------------------------
  cJSON *cjson_x_vout = cJSON_GetObjectItem(root, "x_vout");
  if (cJSON_IsNumber(cjson_x_vout)) {
    anm_data->x_vout = (double)cjson_x_vout->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->x_vout: NOT FOUND");
  }

  cJSON *cjson_y_vout = cJSON_GetObjectItem(root, "y_vout");
  if (cJSON_IsNumber(cjson_y_vout)) {
    anm_data->y_vout = (double)cjson_y_vout->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->y_vout: NOT FOUND");
  }

  cJSON *cjson_z_vout = cJSON_GetObjectItem(root, "z_vout");
  if (cJSON_IsNumber(cjson_z_vout)) {
    anm_data->z_vout = (double)cjson_z_vout->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->z_vout: NOT FOUND");
  }

  // ----------------------------------------
  // Axis Autocalibration
  // ----------------------------------------
  cJSON *cjson_autocalibrazione_asse_x =
      cJSON_GetObjectItem(root, "autocalibrazione_asse_x");
  if (cJSON_IsBool(cjson_autocalibrazione_asse_x)) {
    anm_data->autocalibrazione_asse_x =
        cJSON_IsTrue(cjson_autocalibrazione_asse_x);
  } else {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_asse_x: NOT FOUND");
  }

  cJSON *cjson_autocalibrazione_asse_y =
      cJSON_GetObjectItem(root, "autocalibrazione_asse_y");
  if (cJSON_IsBool(cjson_autocalibrazione_asse_y)) {
    anm_data->autocalibrazione_asse_y =
        cJSON_IsTrue(cjson_autocalibrazione_asse_y);
  } else {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_asse_y: NOT FOUND");
  }

  cJSON *cjson_autocalibrazione_asse_z =
      cJSON_GetObjectItem(root, "autocalibrazione_asse_z");
  if (cJSON_IsBool(cjson_autocalibrazione_asse_z)) {
    anm_data->autocalibrazione_asse_z =
        cJSON_IsTrue(cjson_autocalibrazione_asse_z);
  } else {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_asse_z: NOT FOUND");
  }

  // ----------------------------------------
  // Measure Autocalibration
  // ----------------------------------------
  cJSON *cjson_autocalibrazione_misura_x =
      cJSON_GetObjectItem(root, "autocalibrazione_misura_x");
  if (cJSON_IsBool(cjson_autocalibrazione_misura_x)) {
    anm_data->autocalibrazione_misura_x =
        cJSON_IsTrue(cjson_autocalibrazione_misura_x);
  } else {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_misura_x: NOT FOUND");
  }

  cJSON *cjson_autocalibrazione_misura_y =
      cJSON_GetObjectItem(root, "autocalibrazione_misura_y");
  if (cJSON_IsBool(cjson_autocalibrazione_misura_y)) {
    anm_data->autocalibrazione_misura_y =
        cJSON_IsTrue(cjson_autocalibrazione_misura_y);
  } else {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_misura_y: NOT FOUND");
  }

  cJSON *cjson_autocalibrazione_misura_z =
      cJSON_GetObjectItem(root, "autocalibrazione_misura_z");
  if (cJSON_IsBool(cjson_autocalibrazione_misura_z)) {
    anm_data->autocalibrazione_misura_z =
        cJSON_IsTrue(cjson_autocalibrazione_misura_z);
  } else {
    ESP_LOGI(TAG, "ROOT->autocalibrazione_misura_z: NOT FOUND");
  }

  // ----------------------------------------
  // Sonic Temperature
  // ----------------------------------------
  cJSON *cjson_temp_sonica_x = cJSON_GetObjectItem(root, "temp_sonica_x");
  if (cJSON_IsNumber(cjson_temp_sonica_x)) {
    anm_data->temp_sonica_x = (double)cjson_temp_sonica_x->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->temp_sonica_x: NOT FOUND");
  }

  cJSON *cjson_temp_sonica_y = cJSON_GetObjectItem(root, "temp_sonica_y");
  if (cJSON_IsNumber(cjson_temp_sonica_y)) {
    anm_data->temp_sonica_y = (double)cjson_temp_sonica_y->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->temp_sonica_y: NOT FOUND");
  }

  cJSON *cjson_temp_sonica_z = cJSON_GetObjectItem(root, "temp_sonica_z");
  if (cJSON_IsNumber(cjson_temp_sonica_z)) {
    anm_data->temp_sonica_z = (double)cjson_temp_sonica_z->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->temp_sonica_z: NOT FOUND");
  }

  ESP_LOGI(TAG, "ANEMOMETER DATA PARSED OK.");
  return true;
}

bool parse_particulate_matter_data(cJSON *root,
                                   ParticulateMatterData *sps_data) {

  // ----------------------------------------
  // Timestamp
  // ----------------------------------------
  cJSON *cjson_timestamp = cJSON_GetObjectItem(root, "timestamp");

  if (cJSON_IsNumber(cjson_timestamp)) {
    particulateMatterData.timestamp = (double)cjson_timestamp->valuedouble;
  } else {
    ESP_LOGW(TAG, "ROOT->timestamp: NOT FOUND");
  }

  // ----------------------------------------
  // sensor_data
  // ----------------------------------------
  cJSON *cjson_sensor_data = cJSON_GetObjectItem(root, "sensor_data");

  if (!cJSON_IsObject(cjson_sensor_data)) {
    ESP_LOGI(TAG, "ROOT->sensor_data: NOT FOUND");
  } else {
    // ----------------------------------------
    // sensor_data -> mass_density
    // ----------------------------------------
    cJSON *cjson_mass_density =
        cJSON_GetObjectItem(cjson_sensor_data, "mass_density");

    if (!cJSON_IsObject(cjson_mass_density)) {
      ESP_LOGI(TAG, "ROOT->sensor_data->mass_density: NOT FOUND");
    } else {
      // ----------------------------------------
      // sensor_data -> mass_density -> pm 1.0
      // ----------------------------------------
      cJSON *cjson_md_pm1_0 = cJSON_GetObjectItem(cjson_mass_density, "pm1.0");
      if (cJSON_IsNumber(cjson_md_pm1_0)) {
        particulateMatterData.mass_density_pm_1_0 =
            (double)cjson_md_pm1_0->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->mass_density->pm1.0: NOT FOUND");
      }

      // ----------------------------------------
      // sensor_data -> mass_density -> pm 2.5
      // ----------------------------------------
      cJSON *cjson_md_pm2_5 = cJSON_GetObjectItem(cjson_mass_density, "pm2.5");
      if (cJSON_IsNumber(cjson_md_pm2_5)) {
        particulateMatterData.mass_density_pm_2_5 =
            (double)cjson_md_pm2_5->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->mass_density->pm2.5: NOT FOUND");
      }

      // ----------------------------------------
      // sensor_data -> mass_density -> pm 4.0
      // ----------------------------------------
      cJSON *cjson_md_pm4_0 = cJSON_GetObjectItem(cjson_mass_density, "pm4.0");
      if (cJSON_IsNumber(cjson_md_pm4_0)) {
        particulateMatterData.mass_density_pm_4_0 =
            (double)cjson_md_pm4_0->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->mass_density->pm4.0: NOT FOUND");
      }

      // ----------------------------------------
      // sensor_data -> mass_density -> pm 10
      // ----------------------------------------
      cJSON *cjson_md_pm10 = cJSON_GetObjectItem(cjson_mass_density, "pm10");
      if (cJSON_IsNumber(cjson_md_pm10)) {
        particulateMatterData.mass_density_pm_10 =
            (double)cjson_md_pm10->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->mass_density->pm10: NOT FOUND");
      }
    }

    // ----------------------------------------
    // sensor_data -> particle_count
    // ----------------------------------------
    cJSON *cjson_particle_count =
        cJSON_GetObjectItem(cjson_sensor_data, "particle_count");

    if (!cJSON_IsObject(cjson_particle_count)) {
      ESP_LOGI(TAG, "ROOT->sensor_data->particle_count: NOT FOUND");
    } else {
      // ----------------------------------------
      // sensor_data -> particle_count -> pm0.5
      // ----------------------------------------
      cJSON *cjson_pc_pm0_5 =
          cJSON_GetObjectItem(cjson_particle_count, "pm0.5");
      if (cJSON_IsNumber(cjson_pc_pm0_5)) {
        particulateMatterData.particle_count_0_5 =
            (double)cjson_pc_pm0_5->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->particle_count->pm0.5: NOT FOUND");
      }

      // ----------------------------------------
      // sensor_data -> particle_count -> pm1.0
      // ----------------------------------------
      cJSON *cjson_pc_pm1_0 =
          cJSON_GetObjectItem(cjson_particle_count, "pm1.0");
      if (cJSON_IsNumber(cjson_pc_pm1_0)) {
        particulateMatterData.particle_count_1_0 =
            (double)cjson_pc_pm1_0->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->particle_count->pm1.0: NOT FOUND");
      }

      // ----------------------------------------
      // sensor_data -> particle_count -> pm2.5
      // ----------------------------------------
      cJSON *cjson_pc_pm2_5 =
          cJSON_GetObjectItem(cjson_particle_count, "pm2.5");
      if (cJSON_IsNumber(cjson_pc_pm2_5)) {
        particulateMatterData.particle_count_2_5 =
            (double)cjson_pc_pm2_5->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->particle_count->pm2.5: NOT FOUND");
      }

      // ----------------------------------------
      // sensor_data -> particle_count -> pm4.0
      // ----------------------------------------
      cJSON *cjson_pc_pm4_0 =
          cJSON_GetObjectItem(cjson_particle_count, "pm4.0");
      if (cJSON_IsNumber(cjson_pc_pm4_0)) {
        particulateMatterData.particle_count_4_0 =
            (double)cjson_pc_pm4_0->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->particle_count->pm4.0: NOT FOUND");
      }

      // ----------------------------------------
      // sensor_data -> particle_count -> pm10
      // ----------------------------------------
      cJSON *cjson_pc_pm10 = cJSON_GetObjectItem(cjson_particle_count, "pm10");
      if (cJSON_IsNumber(cjson_pc_pm10)) {
        particulateMatterData.particle_count_10 =
            (double)cjson_pc_pm10->valuedouble;
      } else {
        ESP_LOGI(TAG, "ROOT->sensor_data->particle_count->pm10: NOT FOUND");
      }
    }

    // ----------------------------------------
    // sensor_data -> particle_size
    // ----------------------------------------
    cJSON *cjson_particle_size =
        cJSON_GetObjectItem(cjson_sensor_data, "particle_size");
    if (cJSON_IsNumber(cjson_particle_size)) {
      particulateMatterData.particle_size =
          (double)cjson_particle_size->valuedouble;
    } else {
      ESP_LOGI(TAG, "ROOT->particle_size: NOT FOUND");
    }

    // ----------------------------------------
    // sensor_data -> mass_density_unit
    // ----------------------------------------
    cJSON *cjson_mass_density_unit =
        cJSON_GetObjectItem(cjson_sensor_data, "mass_density_unit");
    if (cJSON_IsString(cjson_mass_density_unit)) {
      strcpy(particulateMatterData.mass_density_unit,
             cjson_mass_density_unit->valuestring);
    } else {
      ESP_LOGI(TAG, "ROOT->mass_density_unit: NOT FOUND");
    }
    // ----------------------------------------
    // sensor_data -> particle_count_unit
    // ----------------------------------------
    cJSON *cjson_particle_count_unit =
        cJSON_GetObjectItem(cjson_sensor_data, "particle_count_unit");
    if (cJSON_IsString(cjson_particle_count_unit)) {
      strcpy(particulateMatterData.particle_count_unit,
             cjson_particle_count_unit->valuestring);
    } else {
      ESP_LOGI(TAG, "ROOT->particle_count_unit: NOT FOUND");
    }

    // ----------------------------------------
    // sensor_data -> particle_size_unit
    // ----------------------------------------
    cJSON *cjson_particle_size_unit =
        cJSON_GetObjectItem(cjson_sensor_data, "particle_size_unit");
    if (cJSON_IsString(cjson_particle_size_unit)) {
      strcpy(particulateMatterData.particle_size_unit,
             cjson_particle_size_unit->valuestring);
    } else {
      ESP_LOGI(TAG, "ROOT->particle_size_unit: NOT FOUND");
    }
  }

  ESP_LOGI(TAG, "SPS DATA PARSED OK.");
  return true;
}

bool parse_imu_data(cJSON *root, ImuData *imu_data) {

  // ----------------------------------------
  // timestamp
  // ----------------------------------------
  cJSON *cjson_timestamp = cJSON_GetObjectItem(root, "timestamp");
  if (cJSON_IsNumber(cjson_timestamp)) {
    imu_data->timestamp = (double)cjson_timestamp->valuedouble;
  } else {
    ESP_LOGI(TAG, "ROOT->timestamp: NOT FOUND");
  }

  // ----------------------------------------
  // sensor_data
  // ----------------------------------------
  cJSON *cjson_sensor_data_array = cJSON_GetObjectItem(root, "sensor_data");
  if (!cJSON_IsArray(cjson_sensor_data_array)) {
    ESP_LOGI(TAG, "ROOT->sensor_data: NOT FOUND");
  } else {
    int sensor_data_array_size = cJSON_GetArraySize(cjson_sensor_data_array);

    for (size_t index = 0; index < sensor_data_array_size; index++) {
      cJSON *cjson_sensor_data =
          cJSON_GetArrayItem(cjson_sensor_data_array, index);

      if (!cJSON_IsObject(cjson_sensor_data)) {
        ESP_LOGI(TAG, "ROOT->sensor_data[%d]: NOT FOUND", index);
      } else {
        // ----------------------------------------
        // sensor_data[] -> dev
        // ----------------------------------------
        cJSON *dev = cJSON_GetObjectItem(cjson_sensor_data, "dev");
        if (!cJSON_IsString(dev)) {
          ESP_LOGI(TAG, "ROOT->sensor_data[%d]->dev: NOT FOUND", index);

        } else {
          // ----------------------------------------
          // sensor_data[] -> dev[acctop]
          // ----------------------------------------
          if (strcmp(dev->valuestring, "acctop") == 0) {

            // ----------------------------------------
            // sensor_data[] -> dev[acctop] -> unit
            // ----------------------------------------
            cJSON *cjson_acctop_unit =
                cJSON_GetObjectItem(cjson_sensor_data, "unit");
            if (cJSON_IsString(cjson_acctop_unit)) {
              strcpy(imu_data->acc_top_unit, cjson_acctop_unit->valuestring);
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acctop: unit NOT FOUND",
                       index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[acctop] -> x
            // ----------------------------------------
            cJSON *cjson_acctop_x = cJSON_GetObjectItem(cjson_sensor_data, "x");
            if (cJSON_IsNumber(cjson_acctop_x)) {
              imu_data->acc_top_x = cjson_acctop_x->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acctop: x NOT FOUND",
                       index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[acctop] -> y
            // ----------------------------------------
            cJSON *cjson_acctop_y = cJSON_GetObjectItem(cjson_sensor_data, "y");
            if (cJSON_IsNumber(cjson_acctop_y)) {
              imu_data->acc_top_y = cjson_acctop_y->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acctop: y NOT FOUND",
                       index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[acctop] -> z
            // ----------------------------------------
            cJSON *cjson_acctop_z = cJSON_GetObjectItem(cjson_sensor_data, "z");
            if (cJSON_IsNumber(cjson_acctop_z)) {
              imu_data->acc_top_z = cjson_acctop_z->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acctop: z NOT FOUND",
                       index);
            }

          } else if (strcmp(dev->valuestring, "acc") == 0) {
            // ----------------------------------------
            // sensor_data[] -> dev[acc] -> unit
            // ----------------------------------------
            cJSON *cjson_acc_unit =
                cJSON_GetObjectItem(cjson_sensor_data, "unit");
            if (cJSON_IsString(cjson_acc_unit)) {
              strcpy(imu_data->acc_unit, cjson_acc_unit->valuestring);
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acc: unit NOT FOUND",
                       index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[acc] -> x
            // ----------------------------------------
            cJSON *cjson_acc_x = cJSON_GetObjectItem(cjson_sensor_data, "x");
            if (cJSON_IsNumber(cjson_acc_x)) {
              imu_data->acc_x = (double)cjson_acc_x->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acc: x NOT FOUND", index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[acc] -> y
            // ----------------------------------------
            cJSON *cjson_acc_y = cJSON_GetObjectItem(cjson_sensor_data, "y");
            if (cJSON_IsNumber(cjson_acc_y)) {
              imu_data->acc_y = (double)cjson_acc_y->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acc: y NOT FOUND", index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[acc] -> z
            // ----------------------------------------
            cJSON *cjson_acc_z = cJSON_GetObjectItem(cjson_sensor_data, "z");
            if (cJSON_IsNumber(cjson_acc_z)) {
              imu_data->acc_z = (double)cjson_acc_z->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->acc: z NOT FOUND", index);
            }

          } else if (strcmp(dev->valuestring, "mag") == 0) {
            // ----------------------------------------
            // sensor_data[] -> dev[mag] -> unit
            // ----------------------------------------
            cJSON *cjson_mag_unit =
                cJSON_GetObjectItem(cjson_sensor_data, "unit");

            if (cJSON_IsString(cjson_mag_unit)) {
              strcpy(imu_data->mag_unit, cjson_mag_unit->valuestring);
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->mag: unit NOT FOUND",
                       index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[mag] -> x
            // ----------------------------------------
            cJSON *cjson_mag_x = cJSON_GetObjectItem(cjson_sensor_data, "x");
            if (cJSON_IsNumber(cjson_mag_x)) {
              imu_data->mag_x = (double)cjson_mag_x->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->mag: x NOT FOUND", index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[mag] -> y
            // ----------------------------------------
            cJSON *cjson_mag_y = cJSON_GetObjectItem(cjson_sensor_data, "y");
            if (cJSON_IsNumber(cjson_mag_y)) {
              imu_data->mag_y = (double)cjson_mag_y->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->mag: y NOT FOUND", index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[mag] -> z
            // ----------------------------------------
            cJSON *cjson_mag_z = cJSON_GetObjectItem(cjson_sensor_data, "z");
            if (cJSON_IsNumber(cjson_mag_z)) {
              imu_data->mag_z = (double)cjson_mag_z->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->mag: z NOT FOUND", index);
            }

          } else if (strcmp(dev->valuestring, "gyr") == 0) {
            // ----------------------------------------
            // sensor_data[] -> dev[gyr] -> unit
            // ----------------------------------------
            cJSON *cjson_gyr_unit =
                cJSON_GetObjectItem(cjson_sensor_data, "unit");

            if (cJSON_IsString(cjson_gyr_unit)) {
              strcpy(imu_data->gyr_unit, cjson_gyr_unit->valuestring);
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->gyr: unit NOT FOUND",
                       index);
              strcpy(imu_data->gyr_unit, "");
            }

            // ----------------------------------------
            // sensor_data[] -> dev[gyr] -> x
            // ----------------------------------------
            cJSON *cjson_gyr_x = cJSON_GetObjectItem(cjson_sensor_data, "x");
            if (cJSON_IsNumber(cjson_gyr_x)) {
              imu_data->gyr_x = (double)cjson_gyr_x->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->gyr: x NOT FOUND", index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[gyr] -> y
            // ----------------------------------------
            cJSON *cjson_gyr_y = cJSON_GetObjectItem(cjson_sensor_data, "y");
            if (cJSON_IsNumber(cjson_gyr_y)) {
              imu_data->gyr_y = (double)cjson_gyr_y->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->gyr: y NOT FOUND", index);
            }

            // ----------------------------------------
            // sensor_data[] -> dev[gyr] -> z
            // ----------------------------------------
            cJSON *cjson_gyr_z = cJSON_GetObjectItem(cjson_sensor_data, "z");
            if (cJSON_IsNumber(cjson_gyr_z)) {
              imu_data->gyr_z = (double)cjson_gyr_z->valuedouble;
            } else {
              ESP_LOGI(TAG, "ROOT->sensor_data[%d]->gyr: z NOT FOUND", index);
            }
          }
        }
      }
    }
  }

  ESP_LOGI(TAG, "IMU DATA PARSED OK.");
  return true;
}

bool parse_status_data(cJSON *root) {

  cJSON *msg = cJSON_GetObjectItem(root, "msg");

  if (!cJSON_IsString(msg)) {
    ESP_LOGI(TAG, "ROOT->msg: NOT FOUND.");
    return false;
  }

  add_text_to_status_list(msg->valuestring);
  return true;
}

ParseReturnCode parse_data(cJSON *json, AnemometerData *anm_data,
                           ParticulateMatterData *pm_data, ImuData *imu_data) {
  static const char *TAG = "PARSE_DATA";

  ESP_LOGI(TAG, "JSON RECEIVED.");

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

  if (strcmp(topic->valuestring, "imu") == 0) {
    if (parse_imu_data(json, imu_data))
      return PRC_UPDATE_IMU;
  }

  if (strcmp(topic->valuestring, "status") == 0) {
    if (parse_status_data(json))
      return PRC_STATUS;
  }

  if (strcmp(topic->valuestring, "type") == 0) {
    ESP_LOGI(TAG, "COMMAND");
  }

  ESP_LOGI(TAG, "Unknown topic: %s", topic->valuestring);
  return PRC_PARSING_ERROR;
}

void on_json_received(cJSON *json) {

  switch (parse_data(json, &anemometerData, &particulateMatterData, &imuData)) {
  case PRC_UPDATED_ANEMOMETER:
    lvgl_update_anemometer_data(&anemometerData);
    break;
  case PRC_UPDATE_PARTICULATE_MATTER:
    lvgl_update_particulate_matter_data(&particulateMatterData);
    break;
  case PRC_UPDATE_IMU:
    lvgl_update_imu_data(&imuData);
    break;
  case PRC_STATUS:
    break;
  case PRC_PARSING_ERROR:
    ESP_LOGW(TAG, "Failed to parse data");
    break;

  default:
    ESP_LOGE(TAG, "WTF!");
    break;
  }
}