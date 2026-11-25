cmake_minimum_required(VERSION 3.16)

# Register USOS root as an extra ESP-IDF component
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../..")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(usos_stage2_wrap_proof)
