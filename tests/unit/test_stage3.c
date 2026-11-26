#include "usos.h"
#include "usos_observation.h"
#include "usos_sensor_monitor.h"
#include "usos_hw_monitor.h"
#include "usos_runtime_monitor.h"
#include "usos_resource_monitor.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

extern void usos_mock_reset(void);
extern void usos_mock_set_time_ms(uint32_t ms);
extern void usos_mock_set_runtime(uint32_t heap_free, uint32_t heap_min,
                                  uint32_t stack_hwm, uint8_t cpu_pct);

USOS_ADAPTER(
    .board = USOS_BOARD_ESP32,
    .device_id = "host-stage3-test",
    .sensors = {
        USOS_SENSOR_DHT22("dht_obs", .min_rate_ms = 1000, .max_rate_ms = 5000,
                          .adaptive_allowed = true, .wrapped_symbol = "DHT_Read"),
        USOS_SENSOR_BMP280("bmp_obs", .min_rate_ms = 500, .max_rate_ms = 5000,
                           .adaptive_allowed = true, .wrapped_symbol = "BMP_Read")
    }
);

static void test_sensor_monitor_metrics_and_anomalies(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    usos_observation_init();

    /* Normal read 1: t=1000ms, val=25.0, latency=200us */
    usos_obs_event_t ev1 = {
        .timestamp_ms = 1000U, .component_id = 0U, .domain = USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 200U, .value = 25.0f, .from_cache = false
    };
    usos_observation_process_event(&ev1);

    /* Normal read 2: t=2000ms, val=27.0 (+2.0/s), latency=400us */
    usos_obs_event_t ev2 = {
        .timestamp_ms = 2000U, .component_id = 0U, .domain = USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 400U, .value = 27.0f, .from_cache = false
    };
    usos_observation_process_event(&ev2);

    /* Out-of-range read (DHT22 valid_max is 80.0) */
    usos_obs_event_t ev3 = {
        .timestamp_ms = 3000U, .component_id = 0U, .domain = USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 300U, .value = 150.0f, .from_cache = false
    };
    usos_observation_process_event(&ev3);

    /* Stuck read (same 150.0f) */
    usos_obs_event_t ev4 = {
        .timestamp_ms = 4000U, .component_id = 0U, .domain = USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 300U, .value = 150.0f, .from_cache = false
    };
    usos_observation_process_event(&ev4);

    /* NaN read */
    usos_obs_event_t ev5 = {
        .timestamp_ms = 5000U, .component_id = 0U, .domain = USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 300U, .value = NAN, .from_cache = false
    };
    usos_observation_process_event(&ev5);

    /* Timeout error (-2) */
    usos_obs_event_t ev6 = {
        .timestamp_ms = 6000U, .component_id = 0U, .domain = USOS_OBS_SENSOR,
        .status_code = -2, .latency_us = 5000U, .value = 0.0f, .from_cache = false
    };
    usos_observation_process_event(&ev6);

    const usos_sensor_stats_t *st = usos_observation_get_sensor(0U);
    assert(st != NULL);
    assert(st->total_reads == 6U);
    assert(st->success_reads == 5U);
    assert(st->error_reads == 1U);
    assert(st->timeout_count == 1U);
    assert(st->consecutive_errors == 1U);
    assert(st->out_of_range_count == 2U);
    assert(st->stuck_count == 1U);
    assert(st->nan_count == 1U);
    assert(st->min_latency_us == 200U);
    assert(st->max_latency_us == 5000U);

    /* Missed sample detection: interval is 1000ms, tick at 9000ms (> 2x) */
    usos_observation_tick(9000U);
    assert(st->missed_samples == 1U);
    printf("[PASS] test_sensor_monitor_metrics_and_anomalies\n");
}

static void test_hw_monitor_bus_errors_and_recovery(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    usos_observation_init();

    /* 3 consecutive I2C errors via sensor 1 (BMP280 on I2C) -> bus unavailable */
    for (uint32_t i = 0; i < 3U; ++i) {
        usos_obs_event_t ev = {
            .timestamp_ms = 1000U + i * 100U, .component_id = 1U,
            .domain = USOS_OBS_SENSOR, .status_code = -2,
            .latency_us = 1000U, .value = 0.0f, .from_cache = false
        };
        usos_observation_process_event(&ev);
    }

    const usos_hw_bus_stats_t *i2c = usos_observation_get_bus(USOS_BUS_I2C);
    assert(i2c != NULL);
    assert(i2c->tx_count == 3U);
    assert(i2c->error_count == 3U);
    assert(i2c->nack_or_timeout_count == 3U);
    assert(i2c->consecutive_errors == 3U);
    assert(i2c->available == false);

    /* Successful I2C read restores bus availability */
    usos_obs_event_t ok = {
        .timestamp_ms = 2000U, .component_id = 1U, .domain = USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 400U, .value = 1013.25f, .from_cache = false
    };
    usos_observation_process_event(&ok);
    assert(i2c->consecutive_errors == 0U);
    assert(i2c->available == true);
    printf("[PASS] test_hw_monitor_bus_errors_and_recovery\n");
}

static void test_runtime_and_resource_monitors(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    usos_observation_init();

    usos_mock_set_runtime(98304U, 81920U, 1536U, 24U);

    /* Fill queue with 3 events and record 1 simulated drop */
    for (uint32_t i = 0; i < 3U; ++i) {
        usos_obs_event_t ev = { .timestamp_ms = 100U + i, .component_id = 0U };
        assert(usos_pal_obs_queue_send(&ev));
    }
    usos_resource_monitor_record_drop();

    usos_observation_tick(1000U);

    const usos_runtime_stats_t *rt = usos_observation_get_runtime();
    const usos_resource_stats_t *res = usos_observation_get_resource();
    assert(rt->free_heap_bytes == 98304U);
    assert(rt->min_free_heap_bytes == 81920U);
    assert(rt->total_cpu_percent == 24U);
    assert(res->obs_queue_depth == 3U);
    assert(res->obs_queue_peak == 3U);
    assert(res->obs_queue_drops == 1U);
    assert(res->usos_task_stack_hwm_bytes == 1536U);
    assert(res->usos_task_cpu_percent == 2U);
    assert(res->active_sensor_count == 2U);
    assert(res->static_ram_bytes > 0U);
    printf("[PASS] test_runtime_and_resource_monitors\n");
}

int main(void)
{
    test_sensor_monitor_metrics_and_anomalies();
    test_hw_monitor_bus_errors_and_recovery();
    test_runtime_and_resource_monitors();
    printf("All Stage 3 host unit tests passed.\n");
    return 0;
}
