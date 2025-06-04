idf_component_register(
    SRCS 
        "src/main.c"
        "src/firebase.c"
        "src/lcd_display.c"
        "src/rfid.c"
        "src/wifi.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi esp_event nvs_flash rc522 esp_lcd esp_http_client json
)
