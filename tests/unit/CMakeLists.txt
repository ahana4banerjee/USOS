cmake_minimum_required(VERSION 3.16)
project(usos_host_tests C)

enable_testing()

# Build platform-independent USOS core + mock PAL
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." "${CMAKE_CURRENT_BINARY_DIR}/usos_core")

add_executable(test_stage2
    test_stage2.c
)
target_link_libraries(test_stage2 PRIVATE usos_core)
target_compile_features(test_stage2 PRIVATE c_std_11)

add_executable(test_stage3
    test_stage3.c
)
target_link_libraries(test_stage3 PRIVATE usos_core)
target_compile_features(test_stage3 PRIVATE c_std_11)

add_test(NAME stage2_unit_tests COMMAND test_stage2)
add_test(NAME stage3_unit_tests COMMAND test_stage3)
