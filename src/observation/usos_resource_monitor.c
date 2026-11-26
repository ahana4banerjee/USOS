#include "usos_resource_monitor.h"
#include "usos_sensor_monitor.h"
#include "usos_hw_monitor.h"
#include "usos_runtime_monitor.h"
#include "usos_observation.h"
#include "usos_registry.h"
#include "usos_pal_rtos.h"
#include <stddef.h>
#include <string.h>

extern uint32_t usos_startup_static_ram(void);

#if USOS_ENABLE_RESOURCE_MONITOR
static usos_resource_stats_t s_resource_stats;

void usos_resource_monitor_init(void)
{
    memset(&s_resource_stats, 0, sizeof(s_resource_stats));
    s_resource_stats.usos_task_cpu_permille = 0xFFFFU;
}

void usos_resource_monitor_record_drop(void)
{
    s_resource_stats.obs_queue_drops++;
}

static uint16_t find_usos_task_cpu_permille(const usos_runtime_stats_t *rt_stats)
{
    if (rt_stats == NULL) {
        return 0xFFFFU;
    }
    for (uint8_t i = 0U; i < rt_stats->tracked_count; ++i) {
        if (strcmp(rt_stats->tasks[i].name, "usos_task") == 0) {
            return rt_stats->tasks[i].cpu_permille;
        }
    }
    return 0xFFFFU;
}

/**
 * @brief Sample USOS self-resource overhead.
 *
 * static_ram_bytes INCLUDES:
 *   - PAL static allocations (obs queue CB + storage, usos_task TCB + stack,
 *     TaskStatus_t snapshot buffer, PAL runtime delta/handle scalars)
 *   - Core static tables (sensor registry, sensor/hw/runtime/resource monitors,
 *     observation tick scalar, startup init flag)
 * static_ram_bytes EXCLUDES:
 *   - Application-owned USOS_ADAPTER const config (.rodata in Flash)
 *   - FreeRTOS kernel internal structures outside USOS static buffers
 */
void usos_resource_monitor_sample(const usos_runtime_stats_t *rt_stats)
{
    uint32_t depth = usos_pal_obs_queue_depth();
    uint32_t interval_peak = usos_pal_obs_queue_peak_and_reset();
    if (depth > interval_peak) {
        interval_peak = depth;
    }

    s_resource_stats.obs_queue_depth = depth;
    s_resource_stats.obs_queue_interval_peak = interval_peak;
    if (interval_peak > s_resource_stats.obs_queue_peak) {
        s_resource_stats.obs_queue_peak = interval_peak;
    }

    s_resource_stats.usos_task_stack_hwm_bytes = usos_pal_get_own_stack_hwm();
    s_resource_stats.usos_task_cpu_permille = find_usos_task_cpu_permille(rt_stats);
    s_resource_stats.active_sensor_count = usos_registry_count();
    s_resource_stats.static_ram_bytes =
        usos_pal_static_ram_bytes() +
        usos_registry_static_ram() +
        usos_sensor_monitor_static_ram() +
        usos_hw_monitor_static_ram() +
        usos_runtime_monitor_static_ram() +
        usos_observation_static_ram() +
        usos_startup_static_ram() +
        (uint32_t)sizeof(s_resource_stats);
}

const usos_resource_stats_t *usos_resource_monitor_get(void)
{
    return &s_resource_stats;
}

uint32_t usos_resource_monitor_static_ram(void)
{
    return (uint32_t)sizeof(s_resource_stats);
}
#else
void usos_resource_monitor_init(void) {}
void usos_resource_monitor_record_drop(void) {}
void usos_resource_monitor_sample(const usos_runtime_stats_t *rt_stats) { (void)rt_stats; }
const usos_resource_stats_t *usos_resource_monitor_get(void) { return NULL; }
uint32_t usos_resource_monitor_static_ram(void) { return 0U; }
#endif
