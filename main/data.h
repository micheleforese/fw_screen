#pragma once

#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct AnemometerData {
  uint32_t timestamp;

  double x_kalman;
  bool autocalibrazione_asse_x;
  bool autocalibrazione_misura_x;
  double temp_sonica_x;

  double y_kalman;
  bool autocalibrazione_asse_y;
  bool autocalibrazione_misura_y;
  double temp_sonica_y;

  double z_kalman;
  bool autocalibrazione_asse_z;
  bool autocalibrazione_misura_z;
  double temp_sonica_z;
} AnemometerData;

extern AnemometerData anemometerData;

typedef struct ParticulateMatterData {
  uint32_t timestamp;

  double mass_density_pm_1_0;
  double mass_density_pm_2_5;
  double mass_density_pm_4_0;
  double mass_density_pm_10;

  double particle_count_0_5;
  double particle_count_1_0;
  double particle_count_2_5;
  double particle_count_4_0;
  double particle_count_10;

  double particle_size;

  char mass_density_unit[8];
  char particle_count_unit[8];
  char particle_size_unit[8];
} ParticulateMatterData;

extern ParticulateMatterData particulateMatterData;

typedef enum {
  PRC_PARSING_ERROR,
  PRC_UPDATED_ANEMOMETER,
  PRC_UPDATE_PARTICULATE_MATTER
} ParseReturnCode;

void anemometer_data_default(AnemometerData *anm_data);
void particulate_matter_data_default(ParticulateMatterData *pm_data);
bool parse_anemometer_data(cJSON *root, AnemometerData *anm_data);
bool parse_particulate_matter_data(cJSON *root,
                                   ParticulateMatterData *sps_data);

ParseReturnCode parse_data(cJSON *json, AnemometerData *anm_data,
                           ParticulateMatterData *pm_data);

void on_json_received(cJSON *json);