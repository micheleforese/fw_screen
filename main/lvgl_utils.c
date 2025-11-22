#include "lvgl_utils.h"
#include "esp_timer.h"
#include "lvgl.h"

#include "config.h"
#include "data.h"

#define LV_COLOR_DEPTH 32 // o 32 se il tuo display lo supporta
#define LV_COLOR_16_SWAP 0

static const char *TAG = "lvgl_ui";
SemaphoreHandle_t lvgl_api_mux = NULL;
lv_timer_t *qmi8658_timer;

lv_obj_t *label_gpio_read_status;

void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area,
                   lv_color_t *color_map) {
  int offsetx1 = area->x1;
  int offsetx2 = area->x2;
  int offsety1 = area->y1;
  int offsety2 = area->y2;
  // copy a buffer's content to a specific area of the display

  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1,
                            offsety2 + 1, color_map);
}

void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  uint16_t touchpad_x[1] = {0};
  uint16_t touchpad_y[1] = {0};
  uint8_t touchpad_cnt = 0;
  esp_lcd_touch_read_data(tp);
  /* Get coordinates */
  bool touchpad_pressed = esp_lcd_touch_get_coordinates(
      tp, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

  if (touchpad_pressed && touchpad_cnt > 0) {
    data->point.x = touchpad_x[0];
    data->point.y = touchpad_y[0];
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void lv_port_disp_init(void) {

  static lv_disp_draw_buf_t draw_buf;

  lv_color_t *buf1 = heap_caps_malloc(LCD_BUF_LENGTH * sizeof(lv_color_t),
                                      MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

  if (!buf1) {
    ESP_LOGE("lv_port_disp_init", "ERRORE FATALE: impossibile allocare buf1!");
    abort();
  }

  lv_color_t *buf2 = heap_caps_malloc(LCD_BUF_LENGTH * sizeof(lv_color_t),
                                      MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

  if (!buf2) {
    ESP_LOGE("lv_port_disp_init", "ERRORE FATALE: impossibile allocare buf2!");
    abort();
  }
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2,
                        LCD_BUF_LENGTH); /*Initialize the display buffer*/

  /*-----------------------------------
   * Register the display in LVGL
   *----------------------------------*/

  lv_disp_drv_init(&disp_drv); /*Basic initialization*/

  /*Set up the functions to access to your display*/

  /*Set the resolution of the display*/
  disp_drv.hor_res = LCD_H_RES;
  disp_drv.ver_res = LCD_V_RES;

  /*Used to copy the buffer's content to the display*/
  disp_drv.flush_cb = lvgl_flush_cb;

  /*Set a display buffer*/
  disp_drv.draw_buf = &draw_buf;

  /*Required for Example 3)*/
  disp_drv.full_refresh = 0;
  // disp_drv.direct_mode = 1;

  /* Fill a memory array with a color if you have GPU.
   * Note that, in lv_conf.h you can enable GPUs that has built-in support in
   * LVGL. But if you have a different GPU you can use with this callback.*/
  // disp_drv.gpu_fill_cb = gpu_fill;

  /*Finally register the driver*/
  lv_disp_drv_register(&disp_drv);
}

void lv_port_indev_init(void) {
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_touch_cb;
  indev_drv.user_data = tp;

  lv_indev_drv_register(&indev_drv);
}

void lvgl_increase_tick(void *arg) {
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void lvgl_tick_timer_init(uint32_t ms) {
  ESP_LOGI(TAG, "Install LVGL tick timer");
  // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
  const esp_timer_create_args_t lvgl_tick_timer_args = {
      .callback = &lvgl_increase_tick, .name = "lvgl_tick"};
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, ms * 1000));
}

void lvgl_unlock(void) { xSemaphoreGiveRecursive(lvgl_api_mux); }

bool lvgl_lock(int timeout_ms) {
  // Convert timeout in milliseconds to FreeRTOS ticks
  // If `timeout_ms` is set to -1, the program will block until the condition is
  // met
  const TickType_t timeout_ticks =
      (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(lvgl_api_mux, timeout_ticks) == pdTRUE;
}

void task(void *param) {
  while (1) {
    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    while (1) {
      // Lock the mutex due to the LVGL APIs are not thread-safe
      if (lvgl_lock(-1)) {
        task_delay_ms = lv_timer_handler();
        // Release the mutex
        lvgl_unlock();
      }
      if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
        task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
      } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
        task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
      }
      vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
  }
}