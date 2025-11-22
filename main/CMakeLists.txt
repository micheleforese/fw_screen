idf_component_register(
    SRCS
    "main.c"
    "screen.c"
    "uart_utils.c"
    "lvgl_utils.c"
    "lvgl_ui.c"
    "data.c"
    REQUIRES esp_adc spi_flash esp_psram json
    INCLUDE_DIRS "."
)
