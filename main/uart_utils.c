#include "uart_utils.h"
#include "cJSON.h"
#include "data.h"

static const char *TAG = "UART";

static char json_line_buffer[JSON_LINE_BUF_SIZE];
static size_t json_buf_pos = 0;

static json_rx_callback_t json_rx_callback = NULL;

static QueueHandle_t uart_rx_queue = NULL;
static QueueHandle_t uart_tx_queue = NULL;
static SemaphoreHandle_t uart_tx_mutex = NULL;

typedef struct {
  char *json_str;
  size_t length;
} uart_tx_item_t;

void usb_uart_init(void) {

  uart_rx_queue = xQueueCreate(USB_UART_RX_QUEUE_LENGTH, sizeof(uart_event_t));
  uart_tx_queue =
      xQueueCreate(USB_UART_TX_QUEUE_LENGTH, sizeof(uart_tx_item_t));

  uart_tx_mutex = xSemaphoreCreateMutex();

  uart_config_t uart_config = {
      .baud_rate = USB_UART_BAUD,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_ERROR_CHECK(uart_driver_install(
      USB_UART_NUM, USB_UART_BUF_SIZE, USB_UART_BUF_SIZE * 2,
      USB_UART_RX_QUEUE_LENGTH, &uart_rx_queue, 0));
  ESP_ERROR_CHECK(uart_param_config(USB_UART_NUM, &uart_config));

  ESP_ERROR_CHECK(uart_enable_pattern_det_baud_intr(
      USB_UART_NUM, USB_UART_PATTERN_ENDING_MSG_CHAR, 1, 9, 0, 0));
  ESP_ERROR_CHECK(
      uart_pattern_queue_reset(USB_UART_NUM, USB_UART_RX_QUEUE_LENGTH));

  usb_json_register_rx_callback(on_json_received);

  xTaskCreate(uart_rx_event_task, "uart_rx_event_task", USB_UART_BUF_SIZE * 4,
              NULL, USB_UART_RX_PRIORITY, NULL);
  xTaskCreate(uart_tx_event_task, "uart_tx_event_task", USB_UART_BUF_SIZE * 2,
              NULL, USB_UART_TX_PRIORITY, NULL);
}

// Register callback for received JSON messages
void usb_json_register_rx_callback(json_rx_callback_t callback) {
  json_rx_callback = callback;
}

void uart_rx_event_task(void *pvParameters) {
  static char *TAG = "UART_RX";
  uint8_t data[USB_UART_BUF_SIZE];
  uart_event_t event;

  while (1) {
    if (xQueueReceive(uart_rx_queue, &event, portMAX_DELAY)) {
      if (event.type == UART_PATTERN_DET) {
        int pos = uart_pattern_pop_pos(USB_UART_NUM);
        if (pos != -1 && pos < USB_UART_BUF_SIZE) {
          int len =
              uart_read_bytes(USB_UART_NUM, data, pos + 1, pdMS_TO_TICKS(100));
          if (len > 0) {
            data[len - 1] = '\0';

            if (strlen((char *)data) == 0) {
              continue;
            }

            // Parse JSON
            cJSON *json = cJSON_Parse((char *)data);
            if (json == NULL) {
              ESP_LOGW(TAG, "Failed to parse JSON: %s", (char *)data);
              continue;
            }

            // Call registered callback
            if (json_rx_callback != NULL) {
              json_rx_callback(json);
            }

            cJSON_Delete(json);
          }
        }
      } else if (event.type == UART_FIFO_OVF) {
        ESP_LOGW(TAG, "UART FIFO overflow");
        uart_flush_input(USB_UART_NUM);
        xQueueReset(uart_rx_queue);
      } else if (event.type == UART_BUFFER_FULL) {
        ESP_LOGW(TAG, "UART ring buffer full");
        uart_flush_input(USB_UART_NUM);
        xQueueReset(uart_rx_queue);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(UBB_UART_EVENT_TASK_DELAY));
  }
}

void uart_tx_event_task(void *pvParameters) {
  static const char *TAG = "UART_TX";
  uart_tx_item_t tx_item;

  ESP_LOGI(TAG, "TX task started");

  while (1) {
    if (xQueueReceive(uart_tx_queue, &tx_item, portMAX_DELAY)) {
      if (tx_item.json_str != NULL && tx_item.length > 0) {
        if (xSemaphoreTake(uart_tx_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {

          int written =
              uart_write_bytes(USB_UART_NUM, tx_item.json_str, tx_item.length);

          uart_write_bytes(USB_UART_NUM, "\n", 1);

          switch (uart_wait_tx_done(USB_UART_NUM, pdMS_TO_TICKS(100))) {
          case ESP_OK:
            break;
          case ESP_FAIL:
            ESP_LOGW(TAG, "Parameter error");
          case ESP_ERR_TIMEOUT:
            ESP_LOGW(TAG, "Error Timeout");
            break;
          default:
            break;
          }

          if (written != tx_item.length) {
            ESP_LOGW(TAG, "Incomplete write: %d/%d bytes", written,
                     tx_item.length);
          }

          xSemaphoreGive(uart_tx_mutex);
        } else {
          ESP_LOGE(TAG, "Failed to acquire TX mutex");
        }

        free(tx_item.json_str);
      }
    }
  }
}

esp_err_t usb_json_send(cJSON *json) {
  if (json == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Convert to string without spaces
  char *json_str = cJSON_PrintUnformatted(json);
  if (json_str == NULL) {
    ESP_LOGE(TAG, "Failed to serialize JSON");
    return ESP_ERR_NO_MEM;
  }

  size_t len = strlen(json_str);

  // Create TX queue item
  uart_tx_item_t tx_item = {.json_str = json_str, // Will be freed by TX task
                            .length = len};

  // Add to TX queue (non-blocking with timeout)
  if (xQueueSend(uart_tx_queue, &tx_item, pdMS_TO_TICKS(100)) != pdTRUE) {
    ESP_LOGW(TAG, "TX queue full, message dropped");
    cJSON_free(json_str);
    return ESP_ERR_TIMEOUT;
  }

  return ESP_OK;
}

int usb_json_get_tx_pending(void) {
  return uxQueueMessagesWaiting(uart_tx_queue);
}

void usb_json_flush_tx_queue(void) {
  uart_tx_item_t tx_item;

  // Empty the queue and free all allocated strings
  while (xQueueReceive(uart_tx_queue, &tx_item, 0) == pdTRUE) {
    if (tx_item.json_str != NULL) {
      free(tx_item.json_str);
    }
  }

  ESP_LOGI(TAG, "TX queue flushed");
}