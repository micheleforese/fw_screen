#include "cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// CDC instance numbers
#define CDC_ITF_LOGS 0 // For ESP logs
#define CDC_ITF_DATA 1 // For JSON data communication

void tusb_cdc_rx_callback(int itf, cdcacm_event_t *event);
void tusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event);
void tiny_usb_init(void);
void tusb_write(const char *msg, size_t len);
void tusb_json_write(const cJSON *json);
