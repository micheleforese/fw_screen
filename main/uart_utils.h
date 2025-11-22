#pragma once

#include "data.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_utils.h"
#include "uart_utils.h"

#define USB_UART_NUM UART_NUM_0
#define USB_UART_BAUD 115200
#define USB_UART_BUF_SIZE 2048
#define JSON_LINE_BUF_SIZE 2048
#define USB_UART_RX_QUEUE_LENGTH 10
#define USB_UART_TX_QUEUE_LENGTH 10
#define USB_UART_PATTERN_ENDING_MSG_CHAR ('\n')
#define USB_UART_RX_PRIORITY 10
#define USB_UART_TX_PRIORITY 9
#define UBB_UART_EVENT_TASK_DELAY 10 // ms

#define MAX_MESSAGE_LENGTH (512)

typedef void (*json_rx_callback_t)(cJSON *json);

void usb_uart_init(void);
void usb_json_register_rx_callback(json_rx_callback_t callback);
void uart_rx_event_task(void *pvParameters);
void uart_tx_event_task(void *pvParameters);
esp_err_t usb_json_send(cJSON *json);
int usb_json_get_tx_pending(void);
void usb_json_flush_tx_queue(void);
