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

// Interface indices
enum {
  ITF_NUM_CDC_LOGS = 0,
  ITF_NUM_CDC_LOGS_DATA,
  ITF_NUM_CDC_DATA,
  ITF_NUM_CDC_DATA_DATA,
  ITF_NUM_TOTAL
};

// Endpoint addresses
#define EPNUM_CDC_LOGS_NOTIF 0x81
#define EPNUM_CDC_LOGS_OUT 0x02
#define EPNUM_CDC_LOGS_IN 0x82

#define EPNUM_CDC_DATA_NOTIF 0x83
#define EPNUM_CDC_DATA_OUT 0x04
#define EPNUM_CDC_DATA_IN 0x84

// USB PID for dual CDC
#define USB_TUSB_PID (0x4002)

// Device Descriptor
static const tusb_desc_device_t dual_cdc_device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A, // Espressif VID
    .idProduct = USB_TUSB_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01};

// Configuration Descriptor
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + 2 * TUD_CDC_DESC_LEN)

static const uint8_t dual_cdc_configuration_descriptor[] = {
    // Config: interface count, string index, total length, attribute, power in
    // mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 500),

    // Interface 0: CDC Logs Port
    // Interface number, string index, EP notification, EP data out, EP data in
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_LOGS, 4, EPNUM_CDC_LOGS_NOTIF, 8,
                       EPNUM_CDC_LOGS_OUT, EPNUM_CDC_LOGS_IN, 64),

    // Interface 2: CDC Data Port
    // Interface number, string index, EP notification, EP data out, EP data in
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_DATA, 5, EPNUM_CDC_DATA_NOTIF, 8,
                       EPNUM_CDC_DATA_OUT, EPNUM_CDC_DATA_IN, 64),
};

// String Descriptors
static const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04},   // 0: Language (English)
    "Espressif",                  // 1: Manufacturer
    "ESP32-S3 Dual CDC Device",   // 2: Product
    "12345678",                   // 3: Serial Number
    "ESP32-S3 Logs Console",      // 4: CDC Interface 0 (Logs)
    "ESP32-S3 JSON Data Channel", // 5: CDC Interface 1 (Data)
};

// Descriptor configuration structure
static const tinyusb_desc_config_t dual_cdc_descriptor_config = {
    .device = &dual_cdc_device_descriptor,
    .string = string_desc_arr,
    .string_count = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]),
    .full_speed_config = dual_cdc_configuration_descriptor,
    .qualifier = NULL,         // Not needed for Full-Speed device
    .high_speed_config = NULL, // Not using High-Speed
};

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
  tusb_cfg.phy.skip_setup = true;
  tusb_cfg.descriptor = dual_cdc_descriptor_config;

  vTaskDelay(pdMS_TO_TICKS(1000));

  esp_err_t ret = tinyusb_driver_install(&tusb_cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "TinyUSB driver install failed: %s", esp_err_to_name(ret));
    return;
  }
  ESP_LOGI(TAG, "TinyUSB driver installed");

  vTaskDelay(pdMS_TO_TICKS(100));

  // Configure CDC ACM for LOGS interface (ITF 0)
  // This is needed even though console is automatic, to enable the port
  tinyusb_config_cdcacm_t acm_cfg_logs = {
      .cdc_port = CDC_ITF_LOGS,
      .callback_rx = NULL, // No RX callback needed for logs
      .callback_rx_wanted_char = NULL,
      .callback_line_state_changed = &tusb_cdc_line_state_changed_callback,
      .callback_line_coding_changed = NULL};

  ret = tinyusb_cdcacm_init(&acm_cfg_logs);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "CDC0 init failed: %s", esp_err_to_name(ret));
    return;
  }
  ESP_LOGI(TAG, "CDC0 (Data) initialized");

  vTaskDelay(pdMS_TO_TICKS(100));

  // Configure CDC ACM for DATA interface (ITF 1)
  tinyusb_config_cdcacm_t acm_cfg_data = {
      .cdc_port = CDC_ITF_DATA,
      .callback_rx = &tusb_cdc_rx_callback,
      .callback_rx_wanted_char = NULL,
      .callback_line_state_changed = &tusb_cdc_line_state_changed_callback,
      .callback_line_coding_changed = NULL};

  ret = tinyusb_cdcacm_init(&acm_cfg_data);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "CDC1 init failed: %s", esp_err_to_name(ret));
    return;
  }
  ESP_LOGI(TAG, "CDC1 (Data) initialized");

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