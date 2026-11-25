cmake_minimum_required(VERSION 3.16)

if(DEFINED ESP_PLATFORM AND ESP_PLATFORM)
    idf_component_register(
        SRCS
            "src/integration/usos_startup.c"
            "src/integration/usos_wrappers.c"
            "src/registry/usos_registry.c"
            "platform/esp32_freertos/usos_pal_esp32.c"
        INCLUDE_DIRS
            "include"
        REQUIRES
            freertos
            esp_timer
            driver
    )

    # Always wrap app_main so USOS initializes deterministically before user code
    # without requiring an explicit USOS_Init() call.
    idf_build_set_property(LINK_OPTIONS "-Wl,--wrap=app_main" APPEND)
else()
    # Host build (for Unity unit testing without ESP-IDF/FreeRTOS)
    project(usos_core C)
    add_library(usos_core STATIC
        src/registry/usos_registry.c
        src/integration/usos_wrappers.c
        platform/mock/usos_pal_mock.c
    )
    target_include_directories(usos_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(usos_core PUBLIC c_std_11)
    target_compile_definitions(usos_core PUBLIC USOS_HOST_BUILD=1 USOS_CMAKE_ADAPTER_REGISTERED=1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/cmake/usos_cmake_helpers.cmake")
