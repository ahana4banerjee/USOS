#include "usos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

/* Mock PAL test helpers from platform/mock/usos_pal_mock.c */
extern void usos_mock_reset(void);
extern void usos_mock_set_time_ms(uint32_t ms);
extern uint32_t usos_mock_obs_count(void);

static int s_real_calls = 0;
static int s_next_rc = 0;
static float s_next_val = 25.0f;

static int mock_real_read(float *out_val, void *ctx)
{
    (void)ctx;
    s_real_calls++;
    *out_val = s_next_val;
    return s_next_rc;
}

/* Required by usos.h link contract */
USOS_ADAPTER(
    .board = USOS_BOARD_ESP32,
    .device_id = "host-test",
    .sensors = {
        USOS_SENSOR_DHT22("dht_passthrough", .min_rate_ms = 1000, .max_rate_ms = 5000,
                          .adaptive_allowed = true, .wrapped_symbol = "DHT_Read"),
        USOS_SENSOR_BMP280("bmp_cached", .min_rate_ms = 1000, .max_rate_ms = 5000,
                           .adaptive_allowed = true, .cache_enabled = true,
                           .wrapped_symbol = "BMP_Read")
    }
);

static void test_registry_init_and_bounds(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    assert(usos_registry_count() == 2);

    usos_sensor_entry_t *e0 = usos_registry_get(0);
    assert(e0 != NULL);
    assert(e0->health_score == 100U);
    assert(e0->state == USOS_HEALTH_HEALTHY);
    assert(e0->current_interval_ms == 1000U);
    assert(e0->config->cache_enabled == false); /* Pass-through default */

    /* Invalid bounds: min_rate_ms > max_rate_ms */
    usos_adapter_config_t bad_cfg = {
        .board = USOS_BOARD_ESP32,
        .sensors = {
            USOS_SENSOR_DHT22("bad", .min_rate_ms = 5000, .max_rate_ms = 1000)
        }
    };
    assert(usos_registry_init(&bad_cfg) == -2);
    printf("[PASS] test_registry_init_and_bounds\n");
}

static void test_passthrough_default(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    s_real_calls = 0;
    s_next_rc = 0;
    s_next_val = 23.5f;

    float v = 0.0f;
    usos_mock_set_time_ms(100);
    assert(usos_wrap_dispatch("DHT_Read", mock_real_read, NULL, &v) == 0);
    usos_mock_set_time_ms(200); /* Faster than 1000ms interval */
    assert(usos_wrap_dispatch("DHT_Read", mock_real_read, NULL, &v) == 0);

    /* Pass-through must reach real driver every time */
    assert(s_real_calls == 2);
    assert(usos_mock_obs_count() == 2);
    printf("[PASS] test_passthrough_default\n");
}

static void test_opt_in_caching_and_init_fail_guard(void)
{
    usos_mock_reset();
    assert(usos_registry_init(&__usos_adapter_config) == 0);
    s_real_calls = 0;

    float v = 0.0f;
    /* 1. Failed initial read must NOT populate cache or allow cached return */
    s_next_rc = -1;
    s_next_val = -999.0f;
    usos_mock_set_time_ms(100);
    assert(usos_wrap_dispatch("BMP_Read", mock_real_read, NULL, &v) == -1);
    assert(s_real_calls == 1);

    /* 2. Subsequent call within interval still hits real driver because init failed */
    s_next_rc = 0;
    s_next_val = 1012.0f;
    usos_mock_set_time_ms(200);
    assert(usos_wrap_dispatch("BMP_Read", mock_real_read, NULL, &v) == 0);
    assert(s_real_calls == 2);
    assert(fabsf(v - 1012.0f) < 0.01f);

    /* 3. Now within 1000ms interval -> should return cached value without calling real driver */
    s_next_val = 9999.0f;
    usos_mock_set_time_ms(500);
    assert(usos_wrap_dispatch("BMP_Read", mock_real_read, NULL, &v) == 0);
    assert(s_real_calls == 2);
    assert(fabsf(v - 1012.0f) < 0.01f);

    /* Verify original timestamp (200ms) was preserved on cached observation */
    usos_obs_event_t ev;
    assert(usos_pal_obs_queue_recv(&ev, 0)); /* t=100 */
    assert(usos_pal_obs_queue_recv(&ev, 0)); /* t=200 */
    assert(usos_pal_obs_queue_recv(&ev, 0)); /* cached event */
    assert(ev.from_cache == true);
    assert(ev.timestamp_ms == 200U);

    /* 4. After interval expires (200 + 1000 = 1200ms) -> hits real driver again */
    usos_mock_set_time_ms(1250);
    assert(usos_wrap_dispatch("BMP_Read", mock_real_read, NULL, &v) == 0);
    assert(s_real_calls == 3);
    assert(fabsf(v - 9999.0f) < 0.01f);
    printf("[PASS] test_opt_in_caching_and_init_fail_guard\n");
}

int main(void)
{
    test_registry_init_and_bounds();
    test_passthrough_default();
    test_opt_in_caching_and_init_fail_guard();
    printf("All Stage 2 host unit tests passed.\n");
    return 0;
}
