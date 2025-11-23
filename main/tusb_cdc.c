#include "tusb_cdc.h"
#include "cJSON.h"
#include "data.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "tinyusb.h"
#include "tinyusb_cdc_acm.h"
#include "tinyusb_default_config.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "DUAL_CDC";

// Buffer for receiving JSON data
#define JSON_BUFFER_SIZE 2048
static char json_rx_buffer[JSON_BUFFER_SIZE];
static size_t json_rx_index = 0;

// CDC instance numbers
// CDC0 will be used by system console (automatic when
// CONFIG_ESP_CONSOLE_USB_CDC=y) CDC1 will be for JSON data
#define CDC_ITF_DATA 1 // For JSON data communication
#define CDC_ITF_LOGS 0 // For ESP logs (system console)

// Callback for CDC data received on the DATA interface
void tusb_cdc_rx_callback(int itf, cdcacm_event_t *event) {
  if (itf != CDC_ITF_DATA) {
    return; // Only handle data interface
  }

  size_t rx_size = 0;
  uint8_t buf[64];
  esp_err_t ret = tinyusb_cdcacm_read(itf, buf, sizeof(buf), &rx_size);

  if (ret == ESP_OK && rx_size > 0) {
    // Add received data to buffer
    for (size_t i = 0; i < rx_size && json_rx_index < JSON_BUFFER_SIZE - 1;
         i++) {
      json_rx_buffer[json_rx_index++] = buf[i];
    }

    // Look for newline character
    for (size_t i = 0; i < json_rx_index; i++) {
      if (json_rx_buffer[i] == '\n') {
        // Null-terminate the JSON string
        json_rx_buffer[i] = '\0';

        // Process the JSON data here
        ESP_LOGI(TAG, "Received JSON: %s", json_rx_buffer);

        cJSON *json = cJSON_Parse(json_rx_buffer);
        if (json != NULL) {
          on_json_received(json);
          cJSON_Delete(json);
        } else {
          ESP_LOGW(TAG, "Failed to parse JSON");
          cJSON_Delete(json);
        }

        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "ok");
        tusb_json_write(response);
        cJSON_Delete(response);

        // Shift remaining data to beginning of buffer
        size_t remaining = json_rx_index - i - 1;
        if (remaining > 0) {
          memmove(json_rx_buffer, json_rx_buffer + i + 1, remaining);
        }
        json_rx_index = remaining;
        break;
      }
    }

    // Buffer overflow protection
    if (json_rx_index >= JSON_BUFFER_SIZE - 1) {
      ESP_LOGW(TAG, "JSON buffer overflow, resetting");
      json_rx_index = 0;
    }
  }
}

// Callback for CDC line state changes
void tusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event) {
  int dtr = event->line_state_changed_data.dtr;
  int rts = event->line_state_changed_data.rts;
  ESP_LOGI(TAG, "CDC%d Line state changed! dtr:%d, rts:%d", itf, dtr, rts);
}

void tiny_usb_init(void) {

  tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
  tusb_cfg.port = TINYUSB_PORT_FULL_SPEED_0;

  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
  ESP_LOGI(TAG, "TinyUSB driver installed");

  // Give USB time to enumerate
  vTaskDelay(pdMS_TO_TICKS(100));

  // Configure CDC ACM for DATA interface (ITF 1)
  tinyusb_config_cdcacm_t acm_cfg_data = {
      .cdc_port = CDC_ITF_DATA,
      .callback_rx = &tusb_cdc_rx_callback,
      .callback_rx_wanted_char = NULL,
      .callback_line_state_changed = &tusb_cdc_line_state_changed_callback,
      .callback_line_coding_changed = NULL};

  ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm_cfg_data));
  ESP_LOGI(TAG, "CDC ACM interface %d initialized", CDC_ITF_DATA);

  // Give more time for USB enumeration
  vTaskDelay(pdMS_TO_TICKS(100));

  // Configure CDC ACM for LOGS interface (ITF 0)
  // This is needed even though console is automatic, to enable the port
  tinyusb_config_cdcacm_t acm_cfg_logs = {
      .cdc_port = CDC_ITF_LOGS,
      .callback_rx = NULL, // No RX callback needed for logs
      .callback_rx_wanted_char = NULL,
      .callback_line_state_changed = &tusb_cdc_line_state_changed_callback,
      .callback_line_coding_changed = NULL};

  ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm_cfg_logs));
  ESP_LOGI(TAG, "CDC ACM interface %d initialized", CDC_ITF_LOGS);

  // Give more time for USB enumeration
  vTaskDelay(pdMS_TO_TICKS(100));

  ESP_LOGI(TAG, "USB Dual CDC initialized");
  ESP_LOGI(TAG, "CDC0 (ttyACM0): System console and logs");
  ESP_LOGI(TAG, "CDC1 (ttyACM1): JSON data port");
}

void tusb_write(const char *msg, size_t len) {
  if (msg == NULL || len == 0) {
    return;
  }
  tinyusb_cdcacm_write_queue(CDC_ITF_DATA, (uint8_t *)msg, len);
  tinyusb_cdcacm_write_flush(CDC_ITF_DATA, 0);
}

void tusb_json_write(const cJSON *json) {
  if (json == NULL) {
    return;
  }

  char *msg = cJSON_PrintUnformatted(json);
  if (msg == NULL) {
    ESP_LOGE(TAG, "Failed to serialize JSON");
    return;
  }

  size_t len = strlen(msg);
  tinyusb_cdcacm_write_queue(CDC_ITF_DATA, (uint8_t *)msg, len);

  // Add newline
  tinyusb_cdcacm_write_queue(CDC_ITF_DATA, (uint8_t *)"\n", 1);
  tinyusb_cdcacm_write_flush(CDC_ITF_DATA, 0);

  cJSON_free(msg);
}