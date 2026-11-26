#include "usos_sensor_monitor.h"
#include "usos_registry.h"
#include <stddef.h>
#include <string.h>

#define USOS_TIMEOUT_STATUS_CODE (-2)
#define USOS_STUCK_EPSILON       (1e-5f)

static inline float usos_absf(float x)
{
    return (x < 0.0f) ? -x : x;
}

static inline bool usos_is_invalid_float(float x)
{
    /* IEEE-754: NaN != NaN; Inf - Inf is NaN */
    return (x != x) || ((x - x) != 0.0f);
}

static usos_sensor_stats_t s_sensor_stats[USOS_MAX_SENSORS];

void usos_sensor_monitor_init(void)
{
    memset(s_sensor_stats, 0, sizeof(s_sensor_stats));
}

static void update_latency_stats(usos_sensor_stats_t *st, uint32_t latency_us)
{
    if (st->total_reads == 1U) {
        st->min_latency_us = latency_us;
        st->max_latency_us = latency_us;
        st->avg_latency_us = latency_us;
        return;
    }
    if (latency_us < st->min_latency_us) {
        st->min_latency_us = latency_us;
    }
    if (latency_us > st->max_latency_us) {
        st->max_latency_us = latency_us;
    }
    /* Integer EMA (alpha = 1/8) to avoid overflow on long uptimes */
    st->avg_latency_us = (uint32_t)(((uint64_t)st->avg_latency_us * 7ULL + latency_us) / 8ULL);
}

static void update_value_metrics(usos_sensor_stats_t *st,
                                 const usos_sensor_config_t *cfg,
                                 const usos_obs_event_t *ev)
{
    if (usos_is_invalid_float(ev->value)) {
        st->nan_count++;
        return;
    }

    if (cfg != NULL && cfg->valid_max > cfg->valid_min) {
        if (ev->value < cfg->valid_min || ev->value > cfg->valid_max) {
            st->out_of_range_count++;
        }
    }

    bool from_cache = (ev->flags & USOS_OBS_FLAG_FROM_CACHE) != 0U;
    if (st->has_reading) {
        if (!from_cache && usos_absf(ev->value - st->last_value) <= USOS_STUCK_EPSILON) {
            st->stuck_count++;
        } else if (!from_cache) {
            st->stuck_count = 0U;
        }

        uint32_t dt_ms = (ev->timestamp_ms > st->last_read_ms)
                             ? (ev->timestamp_ms - st->last_read_ms)
                             : 0U;
        if (dt_ms > 0U) {
            st->rate_of_change = usos_absf(ev->value - st->last_value) * 1000.0f / (float)dt_ms;
        }
        st->prev_value = st->last_value;
    }

    st->last_value = ev->value;
    st->has_reading = true;
}

void usos_sensor_monitor_process(const usos_obs_event_t *ev)
{
    if (ev == NULL || ev->component_id >= usos_registry_count()) {
        return;
    }

    usos_sensor_stats_t *st = &s_sensor_stats[ev->component_id];
    const usos_sensor_entry_t *entry = usos_registry_get(ev->component_id);
    const usos_sensor_config_t *cfg = (entry != NULL) ? entry->config : NULL;

    st->total_reads++;
    if ((ev->flags & USOS_OBS_FLAG_FROM_CACHE) == 0U) {
        update_latency_stats(st, ev->latency_us);
    }

    if (ev->status_code == 0) {
        st->success_reads++;
        st->consecutive_errors = 0U;
        update_value_metrics(st, cfg, ev);
    } else {
        st->error_reads++;
        st->consecutive_errors++;
        if (ev->status_code == USOS_TIMEOUT_STATUS_CODE) {
            st->timeout_count++;
        }
    }

    st->last_read_ms = ev->timestamp_ms;
}

void usos_sensor_monitor_check_missed(uint32_t now_ms)
{
    uint8_t count = usos_registry_count();
    for (uint8_t i = 0U; i < count; ++i) {
        const usos_sensor_entry_t *entry = usos_registry_get(i);
        usos_sensor_stats_t *st = &s_sensor_stats[i];
        if (entry == NULL || st->total_reads == 0U || entry->current_interval_ms == 0U) {
            continue;
        }
        uint32_t elapsed = now_ms - st->last_read_ms;
        if (elapsed > (entry->current_interval_ms * 2U)) {
            st->missed_samples++;
        }
    }
}

const usos_sensor_stats_t *usos_sensor_monitor_get(uint8_t sensor_index)
{
    if (sensor_index >= usos_registry_count()) {
        return NULL;
    }
    return &s_sensor_stats[sensor_index];
}

uint32_t usos_sensor_monitor_static_ram(void)
{
    return (uint32_t)sizeof(s_sensor_stats);
}
