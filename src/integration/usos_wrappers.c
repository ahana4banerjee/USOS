#include "usos_types.h"
#include "usos_registry.h"
#include "usos_resource_monitor.h"
#include "usos_pal_rtos.h"
#include <stdbool.h>
#include <stddef.h>

typedef int (*usos_real_read_fn_t)(float *out_val, void *ctx);

static int resolve_sensor_index(const char *symbol_or_id)
{
    int idx = usos_registry_find_by_symbol(symbol_or_id);
    if (idx < 0) {
        idx = usos_registry_find_by_id(symbol_or_id);
    }
    return idx;
}

int usos_wrap_dispatch(const char *symbol_or_id, usos_real_read_fn_t real_fn,
                       void *ctx, float *out_val)
{
    if (real_fn == NULL || out_val == NULL) {
        return -1;
    }

    int idx = resolve_sensor_index(symbol_or_id);
    uint32_t now_ms = usos_pal_tick_ms();

    if (idx >= 0) {
        float cached = 0.0f;
        uint32_t orig_ts = 0U;
        bool is_stale = false;
        if (usos_registry_should_use_cache((uint8_t)idx, now_ms, &cached,
                                           &orig_ts, &is_stale)) {
            *out_val = cached;
            usos_obs_event_t ev = {
                .timestamp_ms = orig_ts,
                .component_id = (uint8_t)idx,
                .domain = USOS_OBS_SENSOR,
                .status_code = 0,
                .latency_us = 0U,
                .value = cached,
                .from_cache = is_stale
            };
            if (!usos_pal_obs_queue_send(&ev)) {
                usos_resource_monitor_record_drop();
            }
            return 0;
        }
    }

    uint32_t t0_us = usos_pal_tick_us();
    int rc = real_fn(out_val, ctx);
    uint32_t latency_us = usos_pal_tick_us() - t0_us;
    uint32_t done_ms = usos_pal_tick_ms();

    if (idx >= 0) {
        usos_registry_update_sample((uint8_t)idx, *out_val, rc, done_ms);
        usos_obs_event_t ev = {
            .timestamp_ms = done_ms,
            .component_id = (uint8_t)idx,
            .domain = USOS_OBS_SENSOR,
            .status_code = rc,
            .latency_us = latency_us,
            .value = *out_val,
            .from_cache = false
        };
        if (!usos_pal_obs_queue_send(&ev)) {
            usos_resource_monitor_record_drop();
        }
    }

    return rc;
}
