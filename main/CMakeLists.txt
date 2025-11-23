idf_component_register(
    SRCS
    "main.c"
    "screen.c"
    "lvgl_utils.c"
    "lvgl_ui.c"
    "data.c"
    "tusb_cdc.c"
    REQUIRES spi_flash esp_psram json tinyusb
    INCLUDE_DIRS "."
)
