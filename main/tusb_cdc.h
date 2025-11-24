
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "tinyusb_cdc_acm.h"
#include "tinyusb_default_config.h"
#include <stdint.h>

#define TUSB_CDC_LOG_ACM TINYUSB_CDC_ACM_0
#define TUSB_CDC_DATA_ACM TINYUSB_CDC_ACM_1

void tusb_cdc_rx_callback(int itf, cdcacm_event_t *event);
void tusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event);
void tusb_cdc_init(void);
void tusb_cdc_rx_task(void *param);
void tusb_write(const char *msg);
void tusb_json_write(const cJSON *json);