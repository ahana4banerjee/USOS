#include "usos_observation.h"
#include "usos_sensor_monitor.h"
#include "usos_hw_monitor.h"
#include "usos_runtime_monitor.h"
#include "usos_resource_monitor.h"
#include "usos_registry.h"
#include "usos_pal_rtos.h"
#include <stddef.h>

static uint32_t s_last_tick_ms = 0U;

void usos_observation_init(void)
{
    usos_sensor_monitor_init();
    usos_hw_monitor_init();
    usos_runtime_monitor_init();
    usos_resource_monitor_init();
    s_last_tick_ms = 0U;
}

void usos_observation_process_event(const usos_obs_event_t *ev)
{
    if (ev == NULL) {
        return;
    }

    switch (ev->domain) {
        case USOS_OBS_SENSOR: {
            usos_sensor_monitor_process(ev);
            /* Propagate non-cached sensor transaction status to its underlying bus */
            if ((ev->flags & USOS_OBS_FLAG_FROM_CACHE) == 0U) {
                const usos_sensor_entry_t *entry = usos_registry_get(ev->component_id);
                if (entry != NULL && entry->config != NULL) {
                    usos_hw_monitor_record_tx(entry->config->bus, ev->status_code,
                                              ev->timestamp_ms);
                }
            }
            break;
        }
        case USOS_OBS_HARDWARE:
            usos_hw_monitor_process(ev);
            break;
        case USOS_OBS_RUNTIME:
        case USOS_OBS_RESOURCE:
        default:
            break;
    }
}

void usos_observation_tick(uint32_t now_ms)
{
    if (s_last_tick_ms != 0U && (now_ms - s_last_tick_ms) < (uint32_t)USOS_OBS_PERIOD_MS) {
        return;
    }
    s_last_tick_ms = now_ms;

    usos_sensor_monitor_check_missed(now_ms);
    usos_runtime_monitor_sample(now_ms);
    const usos_runtime_stats_t *rt = usos_runtime_monitor_get();
    usos_resource_monitor_sample(rt);

#if USOS_OBS_LOG_SUMMARY
    const usos_resource_stats_t *res = usos_resource_monitor_get();
    if (rt != NULL && res != NULL) {
        uint16_t cpu_whole = (rt->total_cpu_permille == 0xFFFFU) ? 255U : (rt->total_cpu_permille / 10U);
        uint16_t cpu_tenths = (rt->total_cpu_permille == 0xFFFFU) ? 0U : (rt->total_cpu_permille % 10U);
        usos_pal_log("USOS_OBS",
                     "tasks=%u cpu=%u.%u%% heap=%luB(min=%luB) | ram=%luB q=%lu/%lu/%u drops=%lu hwm=%luB",
                     (unsigned)rt->task_count,
                     (unsigned)cpu_whole,
                     (unsigned)cpu_tenths,
                     (unsigned long)rt->free_heap_bytes,
                     (unsigned long)rt->min_free_heap_bytes,
                     (unsigned long)res->static_ram_bytes,
                     (unsigned long)res->obs_queue_interval_peak,
                     (unsigned long)res->obs_queue_peak,
                     (unsigned)USOS_OBS_QUEUE_LEN,
                     (unsigned long)res->obs_queue_drops,
                     (unsigned long)res->usos_task_stack_hwm_bytes);
    }
#endif
}

uint32_t usos_observation_static_ram(void)
{
    return (uint32_t)sizeof(s_last_tick_ms);
}

const usos_sensor_stats_t *usos_observation_get_sensor(uint8_t sensor_index)
{
    return usos_sensor_monitor_get(sensor_index);
}

const usos_hw_bus_stats_t *usos_observation_get_bus(usos_bus_type_t bus)
{
    return usos_hw_monitor_get(bus);
}

const usos_runtime_stats_t *usos_observation_get_runtime(void)
{
    return usos_runtime_monitor_get();
}

const usos_resource_stats_t *usos_observation_get_resource(void)
{
    return usos_resource_monitor_get();
}
