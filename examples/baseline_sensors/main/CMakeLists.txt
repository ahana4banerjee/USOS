idf_component_register(
    SRCS "main.c" "drivers.c"
    INCLUDE_DIRS "."
    REQUIRES USOS freertos esp_timer
)

usos_register_adapter(
    TARGET ${COMPONENT_LIB}
    WRAP_SYMBOLS
        DHT_Read
        BMP280_ReadPressure
        HCSR04_ReadDistance
        LDR_ReadRaw
)
