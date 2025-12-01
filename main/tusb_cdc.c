#include "tusb_cdc.h"
#include "cJSON.h"
#include "data.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "tinyusb.h"
#include "tinyusb_cdc_acm.h"
#include "tinyusb_console.h"
#include "tinyusb_default_config.h"
#include <stdint.h>

static const char *TAG = "TUSB_CDC";
static uint8_t rx_buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];

typedef struct {
  uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1]; // Data buffer
  size_t buf_len;                                 // Number of bytes received
  uint8_t itf; // Index of CDC device interface
} app_message_t;
static QueueHandle_t app_queue;

/**
 * @brief CDC device RX callback
 *
 * CDC device signals, that new data were received
 *
 * @param[in] itf   CDC device index
 * @param[in] event CDC event type
 */
void tusb_cdc_rx_callback(int itf, cdcacm_event_t *event) {
  /* initialization */
  size_t rx_size = 0;

  /* read */
  esp_err_t ret = tinyusb_cdcacm_read(
      itf, rx_buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE * 4, &rx_size);
  if (ret == ESP_OK) {

    app_message_t tx_msg = {
        .buf_len = rx_size,
        .itf = itf,
    };

    memcpy(tx_msg.buf, rx_buf, rx_size);
    xQueueSend(app_queue, &tx_msg, 0);
  } else {
    ESP_LOGE(TAG, "Read Error");
  }
}

/**
 * @brief CDC device line change callback
 *
 * CDC device signals, that the DTR, RTS states changed
 *
 * @param[in] itf   CDC device index
 * @param[in] event CDC event type
 */
void tusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event) {
  int dtr = event->line_state_changed_data.dtr;
  int rts = event->line_state_changed_data.rts;
  ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr,
           rts);
}

void tusb_cdc_init(void) {
  app_queue = xQueueCreate(10, sizeof(app_message_t));

  ESP_LOGI(TAG, "USB initialization");
  const tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

  tinyusb_config_cdcacm_t acm_log_cfg = {.cdc_port = TUSB_CDC_LOG_ACM,
                                         .callback_rx = NULL,
                                         .callback_rx_wanted_char = NULL,
                                         .callback_line_state_changed = NULL,
                                         .callback_line_coding_changed = NULL};

  ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm_log_cfg));
  /* the second way to register a callback */
  ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
      TUSB_CDC_LOG_ACM, CDC_EVENT_LINE_STATE_CHANGED,
      &tusb_cdc_line_state_changed_callback));

  ESP_ERROR_CHECK(tinyusb_console_init(TINYUSB_CDC_ACM_0)); // log to usb

  tinyusb_config_cdcacm_t acm_data_cfg = {.cdc_port = TUSB_CDC_DATA_ACM,
                                          .callback_rx = &tusb_cdc_rx_callback,
                                          .callback_rx_wanted_char = NULL,
                                          .callback_line_state_changed = NULL,
                                          .callback_line_coding_changed = NULL};
  ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm_data_cfg));
  //   ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
  //       TUSB_CDC_DATA_ACM, CDC_EVENT_LINE_STATE_CHANGED,
  //       &tusb_cdc_line_state_changed_callback));

  xTaskCreate(tusb_cdc_rx_task, "tusb_cdc_rx", 1024 * 4, NULL, 5, NULL);
}

void tusb_cdc_rx_task(void *param) {
  app_message_t msg;

  ESP_LOGI(TAG, "USB initialization DONE");
  while (1) {
    if (xQueueReceive(app_queue, &msg, portMAX_DELAY)) {
      if (msg.buf_len) {

        /* Print received data*/
        ESP_LOGI(TAG, "Data from channel %d:", msg.itf);

        cJSON *json = cJSON_Parse(msg.buf);
        if (!json) {
          continue;
        }
        on_json_received(json);
        ESP_LOGI(TAG, "Message Received");
        cJSON_Delete(json);
      }
    }
    vTaskDelay(10);
  }
}

void tusb_write(const char *msg) {
  if (tud_cdc_n_connected(TUSB_CDC_DATA_ACM)) {
    char data_msg[64];
    int len = snprintf(data_msg, sizeof(data_msg), "%s\n", msg);
    tinyusb_cdcacm_write_queue(TUSB_CDC_DATA_ACM, (uint8_t *)data_msg, len);
    tinyusb_cdcacm_write_flush(TUSB_CDC_DATA_ACM, 0);
  }
}

void tusb_json_write(const cJSON *json) {
  char *json_printed = cJSON_PrintUnformatted(json);
  tusb_write(json_printed);
  cJSON_free(json_printed);
}