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
                                  uint32_t stack_hwm, uint16_t cpu_permille);

/* Required by usos_resource_monitor_sample() on host builds */
uint32_t usos_startup_static_ram(void) { return 1U; }

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

static void test_struct_sizes_and_invariants(void)
{
    assert(sizeof(usos_obs_event_t) == 20U);
    assert(USOS_OBS_QUEUE_LEN == 16U);
    printf("[PASS] test_struct_sizes_and_invariants (event=%uB, q_len=%u)\n",
           (unsigned)sizeof(usos_obs_event_t), (unsigned)USOS_OBS_QUEUE_LEN);
}

static void test_sensor_monitor_metrics_and_anomalies(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    usos_observation_init();

    usos_obs_event_t ev1 = {
        .timestamp_ms = 1000U, .component_id = 0U, .domain = (uint8_t)USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 200U, .value = 25.0f, .flags = 0U
    };
    usos_observation_process_event(&ev1);

    usos_obs_event_t ev2 = {
        .timestamp_ms = 2000U, .component_id = 0U, .domain = (uint8_t)USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 400U, .value = 27.0f, .flags = 0U
    };
    usos_observation_process_event(&ev2);

    usos_obs_event_t ev3 = {
        .timestamp_ms = 3000U, .component_id = 0U, .domain = (uint8_t)USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 300U, .value = 150.0f, .flags = 0U
    };
    usos_observation_process_event(&ev3);

    usos_obs_event_t ev4 = {
        .timestamp_ms = 4000U, .component_id = 0U, .domain = (uint8_t)USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 300U, .value = 150.0f, .flags = 0U
    };
    usos_observation_process_event(&ev4);

    usos_obs_event_t ev5 = {
        .timestamp_ms = 5000U, .component_id = 0U, .domain = (uint8_t)USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 300U, .value = NAN, .flags = 0U
    };
    usos_observation_process_event(&ev5);

    usos_obs_event_t ev6 = {
        .timestamp_ms = 6000U, .component_id = 0U, .domain = (uint8_t)USOS_OBS_SENSOR,
        .status_code = -2, .latency_us = 5000U, .value = 0.0f, .flags = 0U
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

    usos_observation_tick(9000U);
    assert(st->missed_samples == 1U);
    printf("[PASS] test_sensor_monitor_metrics_and_anomalies\n");
}

static void test_hw_monitor_bus_errors_and_recovery(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    usos_observation_init();

    assert(usos_observation_get_bus(USOS_BUS_NONE) == NULL);

    for (uint32_t i = 0; i < 3U; ++i) {
        usos_obs_event_t ev = {
            .timestamp_ms = 1000U + i * 100U, .component_id = 1U,
            .domain = (uint8_t)USOS_OBS_SENSOR, .status_code = -2,
            .latency_us = 1000U, .value = 0.0f, .flags = 0U
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

    usos_obs_event_t ok = {
        .timestamp_ms = 2000U, .component_id = 1U, .domain = (uint8_t)USOS_OBS_SENSOR,
        .status_code = 0, .latency_us = 400U, .value = 1013.25f, .flags = 0U
    };
    usos_observation_process_event(&ok);
    assert(i2c->consecutive_errors == 0U);
    assert(i2c->available == true);
    printf("[PASS] test_hw_monitor_bus_errors_and_recovery\n");
}

static void test_queue_burst_peak_and_overflow_drops(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    usos_observation_init();

    usos_mock_set_runtime(98304U, 81920U, 1536U, 245U); /* 24.5% CPU */

    /* Burst fill full queue (16) + 2 overflows */
    for (uint32_t i = 0; i < USOS_OBS_QUEUE_LEN; ++i) {
        usos_obs_event_t ev = { .timestamp_ms = 100U + i, .component_id = 0U };
        assert(usos_pal_obs_queue_send(&ev));
    }
    usos_obs_event_t extra = { .timestamp_ms = 200U, .component_id = 0U };
    assert(!usos_pal_obs_queue_send(&extra));
    usos_resource_monitor_record_drop();
    assert(!usos_pal_obs_queue_send(&extra));
    usos_resource_monitor_record_drop();

    /* Drain all events BEFORE tick (simulating supervisor loop) */
    usos_obs_event_t drained;
    while (usos_pal_obs_queue_recv(&drained, 0U)) {
        usos_observation_process_event(&drained);
    }

    usos_observation_tick(1000U);

    const usos_runtime_stats_t *rt = usos_observation_get_runtime();
    const usos_resource_stats_t *res = usos_observation_get_resource();
    assert(rt->total_cpu_permille == 245U);
    assert(res->obs_queue_depth == 0U);
    assert(res->obs_queue_interval_peak == USOS_OBS_QUEUE_LEN);
    assert(res->obs_queue_peak == USOS_OBS_QUEUE_LEN);
    assert(res->obs_queue_drops == 2U);
    assert(res->usos_task_cpu_permille == 20U);

    /* Next tick with no burst resets interval peak to 0 while all-time peak stays 16 */
    usos_observation_tick(2000U);
    assert(res->obs_queue_interval_peak == 0U);
    assert(res->obs_queue_peak == USOS_OBS_QUEUE_LEN);
    printf("[PASS] test_queue_burst_peak_and_overflow_drops\n");
}

int main(void)
{
    test_struct_sizes_and_invariants();
    test_sensor_monitor_metrics_and_anomalies();
    test_hw_monitor_bus_errors_and_recovery();
    test_queue_burst_peak_and_overflow_drops();
    printf("All Stage 3 host unit tests passed.\n");
    return 0;
}
