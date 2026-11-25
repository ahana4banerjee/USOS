# USOS CMake Integration Helpers
# Propagates -Wl,--wrap=<symbol> flags to the final ESP-IDF application executable
# and marks the build as properly configured for USOS.

function(usos_register_adapter)
    set(options)
    set(oneValueArgs TARGET)
    set(multiValueArgs WRAP_SYMBOLS)
    cmake_parse_arguments(USOS_ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_wrap_flags "-Wl,--wrap=app_main")
    foreach(_sym IN LISTS USOS_ARG_WRAP_SYMBOLS)
        list(APPEND _wrap_flags "-Wl,--wrap=${_sym}")
    endforeach()

    if(DEFINED ESP_PLATFORM AND ESP_PLATFORM)
        # Propagate to the final ESP-IDF application executable
        foreach(_flag IN LISTS _wrap_flags)
            idf_build_set_property(LINK_OPTIONS "${_flag}" APPEND)
        endforeach()
        idf_build_set_property(COMPILE_DEFINITIONS "USOS_CMAKE_ADAPTER_REGISTERED=1" APPEND)
    elseif(USOS_ARG_TARGET)
        target_link_options(${USOS_ARG_TARGET} PUBLIC ${_wrap_flags})
        target_compile_definitions(${USOS_ARG_TARGET} PUBLIC USOS_CMAKE_ADAPTER_REGISTERED=1)
    endif()
endfunction()
