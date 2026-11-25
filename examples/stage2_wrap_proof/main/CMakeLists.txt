idf_component_register(
    SRCS "main.c" "drivers.c"
    INCLUDE_DIRS "."
    REQUIRES USOS freertos esp_timer
)

# Propagate --wrap=app_main and --wrap=DHT_Read to final executable
usos_register_adapter(
    TARGET ${COMPONENT_LIB}
    WRAP_SYMBOLS
        DHT_Read
)
