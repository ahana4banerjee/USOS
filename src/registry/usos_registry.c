#include "usos_registry.h"
#include <stddef.h>
#include <string.h>

static usos_sensor_entry_t s_registry[USOS_MAX_SENSORS];
static uint8_t s_sensor_count = 0;

static uint32_t resolve_initial_interval(const usos_sensor_config_t *cfg)
{
    if (!cfg->adaptive_allowed && cfg->required_rate_ms > 0U) {
        return cfg->required_rate_ms;
    }
    if (cfg->min_rate_ms > 0U) {
        return cfg->min_rate_ms;
    }
    return (cfg->max_rate_ms > 0U) ? cfg->max_rate_ms : 1000U;
}

int usos_registry_init(const usos_adapter_config_t *config)
{
    if (config == NULL) {
        return -1;
    }

    memset(s_registry, 0, sizeof(s_registry));
    s_sensor_count = 0;

    for (uint8_t i = 0; i < USOS_MAX_SENSORS; ++i) {
        const usos_sensor_config_t *cfg = &config->sensors[i];
        if (cfg->id == NULL) {
            continue;
        }
        if (cfg->max_rate_ms > 0U && cfg->min_rate_ms > cfg->max_rate_ms) {
            return -2; /* Invalid interval bounds: min_rate_ms > max_rate_ms */
        }

        usos_sensor_entry_t *entry = &s_registry[s_sensor_count++];
        entry->config = cfg;
        entry->state = USOS_HEALTH_HEALTHY;
        entry->health_score = 100U;
        entry->current_interval_ms = resolve_initial_interval(cfg);
        entry->has_valid_init_read = false;
        entry->is_stale = false;
    }

    return 0;
}

uint8_t usos_registry_count(void)
{
    return s_sensor_count;
}

usos_sensor_entry_t *usos_registry_get(uint8_t index)
{
    if (index >= s_sensor_count) {
        return NULL;
    }
    return &s_registry[index];
}

int usos_registry_find_by_id(const char *id)
{
    if (id == NULL) {
        return -1;
    }
    for (uint8_t i = 0; i < s_sensor_count; ++i) {
        if (s_registry[i].config != NULL && s_registry[i].config->id != NULL &&
            strcmp(s_registry[i].config->id, id) == 0) {
            return (int)i;
        }
    }
    return -1;
}

int usos_registry_find_by_symbol(const char *symbol)
{
    if (symbol == NULL) {
        return -1;
    }
    for (uint8_t i = 0; i < s_sensor_count; ++i) {
        if (s_registry[i].config != NULL &&
            s_registry[i].config->wrapped_symbol != NULL &&
            strcmp(s_registry[i].config->wrapped_symbol, symbol) == 0) {
            return (int)i;
        }
    }
    return -1;
}

void usos_registry_update_sample(uint8_t index, float value, int32_t status_code,
                                 uint32_t now_ms)
{
    usos_sensor_entry_t *entry = usos_registry_get(index);
    if (entry == NULL) {
        return;
    }

    if (status_code == 0) {
        entry->last_success_ms = now_ms;
        entry->cached_value = value;
        entry->cache_ts_ms = now_ms;
        entry->has_valid_init_read = true;
        entry->is_stale = false;
    } else {
        entry->fault_count++;
    }
}

bool usos_registry_should_use_cache(uint8_t index, uint32_t now_ms,
                                    float *out_value, uint32_t *out_orig_ts_ms,
                                    bool *out_is_stale)
{
    usos_sensor_entry_t *entry = usos_registry_get(index);
    if (entry == NULL || entry->config == NULL) {
        return false;
    }

    /* Default is pass-through unless explicitly opted in */
    if (!entry->config->cache_enabled) {
        return false;
    }

    /* Never return cached data after a failed initial read unless permitted */
    if (!entry->has_valid_init_read && !entry->config->allow_stale_on_init_fail) {
        return false;
    }

    if ((now_ms - entry->cache_ts_ms) < entry->current_interval_ms) {
        entry->is_stale = true;
        if (out_value != NULL) {
            *out_value = entry->cached_value;
        }
        if (out_orig_ts_ms != NULL) {
            *out_orig_ts_ms = entry->cache_ts_ms;
        }
        if (out_is_stale != NULL) {
            *out_is_stale = true;
        }
        return true;
    }

    return false;
}

uint32_t usos_registry_static_ram(void)
{
    return (uint32_t)(sizeof(s_registry) + sizeof(s_sensor_count));
}
