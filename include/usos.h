#ifndef USOS_H
#define USOS_H

#include "usos_types.h"
#include "usos_registry.h"
#include "usos_pal_rtos.h"
#include "usos_pal_hw.h"

#ifndef USOS_CMAKE_ADAPTER_REGISTERED
#error "USOS requires usos_register_adapter() in CMakeLists.txt so --wrap=app_main and wrapped symbols are linked."
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief External symbol defined by USOS_ADAPTER(...) and consumed by __wrap_app_main.
 */
extern const usos_adapter_config_t __usos_adapter_config;

/**
 * @brief Declarative adapter macro placed once in the application's main translation unit.
 * Expands to a pure compile-time const struct (no GCC constructors).
 */
#define USOS_ADAPTER(...) \
    const usos_adapter_config_t __usos_adapter_config = { __VA_ARGS__ }

/* Convenience sensor descriptor macros (pass-through by default: cache_enabled = false) */
#define USOS_SENSOR_DHT22(sensor_id, ...) \
    { .id = (sensor_id), .type = USOS_SENSOR_DHT22, .bus = USOS_BUS_GPIO, \
      .valid_min = -40.0f, .valid_max = 80.0f, __VA_ARGS__ }

#define USOS_SENSOR_BMP280(sensor_id, ...) \
    { .id = (sensor_id), .type = USOS_SENSOR_BMP280, .bus = USOS_BUS_I2C, \
      .valid_min = 300.0f, .valid_max = 1100.0f, __VA_ARGS__ }

#define USOS_SENSOR_HCSR04(sensor_id, ...) \
    { .id = (sensor_id), .type = USOS_SENSOR_HCSR04, .bus = USOS_BUS_GPIO, \
      .valid_min = 2.0f, .valid_max = 400.0f, __VA_ARGS__ }

#define USOS_SENSOR_LDR(sensor_id, ...) \
    { .id = (sensor_id), .type = USOS_SENSOR_LDR, .bus = USOS_BUS_ADC, \
      .valid_min = 0.0f, .valid_max = 4095.0f, __VA_ARGS__ }

#define USOS_SENSOR_MPU6050(sensor_id, ...) \
    { .id = (sensor_id), .type = USOS_SENSOR_MPU6050, .bus = USOS_BUS_I2C, \
      .valid_min = -16.0f, .valid_max = 16.0f, __VA_ARGS__ }

/**
 * @brief Generic wrapper dispatch used by Tier 1 (--wrap) and Tier 2 driver shims.
 */
typedef int (*usos_real_read_fn_t)(float *out_val, void *ctx);

int usos_wrap_dispatch(const char *symbol_or_id, usos_real_read_fn_t real_fn,
                       void *ctx, float *out_val);

#ifdef __cplusplus
}
#endif

#endif /* USOS_H */
