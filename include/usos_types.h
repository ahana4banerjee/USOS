#ifndef USOS_TYPES_H
#define USOS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USOS_MAX_SENSORS
#ifdef CONFIG_USOS_MAX_SENSORS
#define USOS_MAX_SENSORS CONFIG_USOS_MAX_SENSORS
#else
#define USOS_MAX_SENSORS 8
#endif
#endif

#ifndef USOS_OBS_QUEUE_LEN
#ifdef CONFIG_USOS_OBS_QUEUE_LEN
#define USOS_OBS_QUEUE_LEN CONFIG_USOS_OBS_QUEUE_LEN
#else
#define USOS_OBS_QUEUE_LEN 32
#endif
#endif

typedef enum {
    USOS_BOARD_UNKNOWN = 0,
    USOS_BOARD_ESP32
} usos_board_type_t;

typedef enum {
    USOS_SENSOR_NONE = 0,
    USOS_SENSOR_DHT22,
    USOS_SENSOR_BMP280,
    USOS_SENSOR_HCSR04,
    USOS_SENSOR_LDR,
    USOS_SENSOR_MPU6050,
    USOS_SENSOR_CUSTOM
} usos_sensor_type_t;

typedef enum {
    USOS_BUS_NONE = 0,
    USOS_BUS_GPIO,
    USOS_BUS_I2C,
    USOS_BUS_SPI,
    USOS_BUS_UART,
    USOS_BUS_ADC
} usos_bus_type_t;

typedef enum {
    USOS_HEALTH_HEALTHY = 0,
    USOS_HEALTH_DEGRADED,
    USOS_HEALTH_FAULT,
    USOS_HEALTH_RECOVERY
} usos_health_state_t;

typedef enum {
    USOS_OBS_SENSOR = 0,
    USOS_OBS_HARDWARE,
    USOS_OBS_RUNTIME,
    USOS_OBS_RESOURCE
} usos_obs_domain_t;

/**
 * @brief Declarative per-sensor configuration provided inside USOS_ADAPTER(...).
 */
typedef struct {
    const char *id;
    usos_sensor_type_t type;
    usos_bus_type_t bus;
    uint8_t bus_port;
    uint16_t address_or_pin;
    float valid_min;
    float valid_max;
    uint32_t min_rate_ms;          /**< Minimum interval (fastest sampling) */
    uint32_t max_rate_ms;          /**< Maximum interval (slowest sampling) */
    uint32_t required_rate_ms;     /**< Fixed interval when adaptive_allowed == false */
    bool adaptive_allowed;         /**< Enable adaptive interval scaling */
    bool cache_enabled;            /**< Opt-in caching (default false = pass-through) */
    bool allow_stale_on_init_fail; /**< Permit cached return after failed init read (default false) */
    bool task_restart_safe;        /**< Opt-in for task-level recovery (default false) */
    int (*reinit_fn)(void);        /**< Optional driver reinit callback */
    int (*hw_reset_fn)(void);      /**< Optional hardware reset callback */
    const char *wrapped_symbol;    /**< Tier 1 C symbol name for --wrap lookup */
} usos_sensor_config_t;

/**
 * @brief Top-level declarative adapter configuration.
 */
typedef struct {
    usos_board_type_t board;
    const char *device_id;
    usos_sensor_config_t sensors[USOS_MAX_SENSORS];
} usos_adapter_config_t;

/**
 * @brief Fixed-size observation event pushed non-blockingly to usos_obs_queue.
 */
typedef struct {
    uint32_t timestamp_ms;
    uint8_t component_id;
    usos_obs_domain_t domain;
    int32_t status_code;
    uint32_t latency_us;
    float value;
    bool from_cache;
} usos_obs_event_t;

/**
 * @brief Static registry entry tracking runtime state of a single sensor.
 */
typedef struct {
    const usos_sensor_config_t *config;
    usos_health_state_t state;
    uint8_t health_score;
    uint32_t current_interval_ms;
    uint32_t last_success_ms;
    uint32_t fault_count;
    uint32_t recovery_count;
    float cached_value;
    uint32_t cache_ts_ms;
    bool has_valid_init_read;
    bool is_stale;
} usos_sensor_entry_t;

#ifdef __cplusplus
}
#endif

#endif /* USOS_TYPES_H */
