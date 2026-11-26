#ifndef USOS_TYPES_H
#define USOS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#if defined(ESP_PLATFORM) && !defined(USOS_HOST_BUILD)
#include "sdkconfig.h"
#endif

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
#define USOS_OBS_QUEUE_LEN 16
#endif
#endif

#ifndef USOS_OBS_PERIOD_MS
#ifdef CONFIG_USOS_OBS_PERIOD_MS
#define USOS_OBS_PERIOD_MS CONFIG_USOS_OBS_PERIOD_MS
#else
#define USOS_OBS_PERIOD_MS 1000
#endif
#endif

#ifndef USOS_MAX_TRACKED_TASKS
#ifdef CONFIG_USOS_MAX_TRACKED_TASKS
#define USOS_MAX_TRACKED_TASKS CONFIG_USOS_MAX_TRACKED_TASKS
#else
#define USOS_MAX_TRACKED_TASKS 24
#endif
#endif

#ifndef USOS_ENABLE_RUNTIME_MONITOR
#ifdef CONFIG_USOS_ENABLE_RUNTIME_MONITOR
#define USOS_ENABLE_RUNTIME_MONITOR CONFIG_USOS_ENABLE_RUNTIME_MONITOR
#else
#define USOS_ENABLE_RUNTIME_MONITOR 1
#endif
#endif

#ifndef USOS_ENABLE_RESOURCE_MONITOR
#ifdef CONFIG_USOS_ENABLE_RESOURCE_MONITOR
#define USOS_ENABLE_RESOURCE_MONITOR CONFIG_USOS_ENABLE_RESOURCE_MONITOR
#else
#define USOS_ENABLE_RESOURCE_MONITOR 1
#endif
#endif

#ifndef USOS_OBS_LOG_SUMMARY
#ifdef CONFIG_USOS_OBS_LOG_SUMMARY
#define USOS_OBS_LOG_SUMMARY CONFIG_USOS_OBS_LOG_SUMMARY
#else
#define USOS_OBS_LOG_SUMMARY 1
#endif
#endif

#define USOS_ACTIVE_BUS_COUNT 5

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

#define USOS_OBS_FLAG_FROM_CACHE (1U << 0)

/**
 * @brief Fixed-size observation event (20 bytes packed) pushed non-blockingly to usos_obs_queue.
 */
typedef struct {
    uint32_t timestamp_ms;
    int32_t status_code;
    uint32_t latency_us;
    float value;
    uint8_t component_id;
    uint8_t domain;                /**< usos_obs_domain_t */
    uint8_t flags;                 /**< Bit 0: USOS_OBS_FLAG_FROM_CACHE */
    uint8_t reserved;
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

/**
 * @brief Per-sensor observation statistics (Stage 3).
 */
typedef struct {
    uint32_t total_reads;
    uint32_t success_reads;
    uint32_t error_reads;
    uint32_t timeout_count;
    uint32_t consecutive_errors;
    uint32_t missed_samples;
    uint32_t out_of_range_count;
    uint32_t nan_count;
    uint32_t stuck_count;
    uint32_t min_latency_us;
    uint32_t max_latency_us;
    uint32_t avg_latency_us;
    float last_value;
    float prev_value;
    float rate_of_change;          /**< |dx/dt| in units per second */
    uint32_t last_read_ms;
    bool has_reading;
} usos_sensor_stats_t;

/**
 * @brief Per-bus hardware observation statistics (Stage 3).
 */
typedef struct {
    usos_bus_type_t bus;
    uint32_t tx_count;
    uint32_t error_count;
    uint32_t nack_or_timeout_count;
    uint32_t consecutive_errors;
    int32_t last_error_code;
    uint32_t last_error_ms;
    bool available;
} usos_hw_bus_stats_t;

/**
 * @brief FreeRTOS task snapshot entry (Stage 3).
 */
typedef struct {
    char name[16];
    uint8_t state;                 /**< 0=Running,1=Ready,2=Blocked,3=Suspended,4=Deleted */
    uint16_t cpu_permille;         /**< 0..1000 (0.1% resolution); 0xFFFF if unavailable */
    uint32_t stack_hwm_bytes;
} usos_task_info_t;

/**
 * @brief System-wide FreeRTOS runtime snapshot (Stage 3).
 */
typedef struct {
    uint32_t timestamp_ms;
    uint16_t task_count;
    uint16_t total_cpu_permille;   /**< 0..1000 (0.1% resolution); 0xFFFF if unavailable */
    uint8_t tracked_count;
    uint32_t free_heap_bytes;
    uint32_t min_free_heap_bytes;
    int32_t wdt_status;
    usos_task_info_t tasks[USOS_MAX_TRACKED_TASKS];
} usos_runtime_stats_t;

/**
 * @brief USOS middleware self-resource overhead metrics (FR-005, Stage 3).
 */
typedef struct {
    uint32_t static_ram_bytes;     /**< Sum of USOS-owned .bss static structures */
    uint32_t obs_queue_depth;      /**< Instantaneous queue depth at sample time */
    uint32_t obs_queue_interval_peak; /**< Peak queue depth during last sample interval */
    uint32_t obs_queue_peak;       /**< All-time peak queue depth since boot */
    uint32_t obs_queue_drops;      /**< Total events dropped due to full queue */
    uint32_t usos_task_stack_hwm_bytes;
    uint16_t usos_task_cpu_permille; /**< 0..1000 (0.1% resolution); 0xFFFF if unavailable */
    uint8_t active_sensor_count;
} usos_resource_stats_t;

#ifdef __cplusplus
}
#endif

#endif /* USOS_TYPES_H */
