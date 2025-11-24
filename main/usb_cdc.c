#include "usb_cdc.h"
#include "cJSON.h"
#include "data.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "USB_CDC";

static char json_buffer[JSON_BUFFER_SIZE];
static size_t json_index = 0;

static QueueHandle_t json_queue;

void lvgl_json_task(void *arg) {
  char received_json[JSON_BUFFER_SIZE];

  while (1) {
    if (xQueueReceive(json_queue, received_json, portMAX_DELAY) == pdTRUE) {
      ESP_LOGI(TAG, "QUEUE RECIEVED ITEM %s", received_json);
      cJSON *json = cJSON_Parse(received_json);
      if (!json) {
        ESP_LOGW(TAG, "Invalid JSON: %s", received_json);
        continue;
      }

      on_json_received(json);
      cJSON_Delete(json);
      free(received_json);
    }
  }
}

// Task to read from USB CDC console
void usb_cdc_read_task(void *arg) {
  ESP_LOGI(TAG, "USB CDC read task started");

  uint8_t rx_buffer[RX_BUFFER_SIZE];

  while (1) {
    // Read available data
    int len = usb_serial_jtag_read_bytes(rx_buffer, RX_BUFFER_SIZE - 1,
                                         pdMS_TO_TICKS(100));

    if (len > 0) {
      // Process each byte
      for (int i = 0; i < len; i++) {
        char c = rx_buffer[i];

        // Look for newline (end of JSON message)
        if (c == '\n' || c == '\r') {
          if (json_index > 0) {

            // Null-terminate and process
            json_buffer[json_index] = '\0';

            // Skip if it's just a log line or empty
            if (json_buffer[0] == '{') {

              char *json_copy = malloc(json_index + 1);
              if (json_copy) {
                memcpy(json_copy, json_buffer, json_index + 1);
                if (xQueueSend(json_queue, &json_copy, 0) != pdTRUE) {
                  ESP_LOGW(TAG, "JSON queue full, dropping message");
                }
              }
            }

            json_index = 0;
          }
        } else if (json_index < JSON_BUFFER_SIZE - 1) {
          // Add character to buffer
          json_buffer[json_index++] = c;
        } else {
          // Buffer overflow - reset
          ESP_LOGW(TAG, "Buffer overflow, resetting");
          json_index = 0;
        }
      }
    }

    // Small delay to prevent hogging CPU
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void usb_cdc_init(void) {
  ESP_LOGI(TAG, "=== USB CDC Console Bidirectional Example ===");

  // Install USB CDC driver
  usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
      .rx_buffer_size = RX_BUFFER_SIZE,
      .tx_buffer_size = 1024,
  };

  esp_err_t ret = usb_serial_jtag_driver_install(&usb_serial_jtag_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "USB Serial JTAG driver install failed");
    return;
  }

  usb_serial_jtag_vfs_use_driver();

  json_queue = xQueueCreate(JSON_QUEUE_LENGTH, JSON_BUFFER_SIZE);
  if (!json_queue) {
    ESP_LOGE(TAG, "Failed to create JSON queue");
    return;
  }

  xTaskCreate(lvgl_json_task, "lvgl_json", 8192, NULL, 5, NULL);
  // Create task to read from USB CDC
  xTaskCreate(usb_cdc_read_task, "usb_cdc_read", 4096, NULL, 5, NULL);
}

void usb_cdc_write(const char *msg) {
  printf("%s\n", msg);
  fflush(stdout);
}

void usb_cdc_json_write(cJSON *json) {
  char *json_str = cJSON_PrintUnformatted(json);
  usb_cdc_write(json_str);

  cJSON_free(json_str);
}
