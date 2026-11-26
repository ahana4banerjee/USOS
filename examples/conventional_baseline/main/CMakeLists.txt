idf_component_register(
    SRCS "main.c" "drivers.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_timer esp_system
)
