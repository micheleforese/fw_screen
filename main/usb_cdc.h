
#include "cJSON.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#define RX_BUFFER_SIZE 2048
#define JSON_BUFFER_SIZE 2048
#define JSON_QUEUE_LENGTH 10

void process_json_command(const char *json_str);
void usb_cdc_read_task(void *arg);
void usb_cdc_init(void);
void usb_cdc_json_write(cJSON *json);
void usb_cdc_write(const char *msg);