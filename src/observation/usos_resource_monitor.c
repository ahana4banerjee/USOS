#include "usos_resource_monitor.h"
#include "usos_sensor_monitor.h"
#include "usos_hw_monitor.h"
#include "usos_runtime_monitor.h"
#include "usos_registry.h"
#include "usos_pal_rtos.h"
#include <string.h>

static usos_resource_stats_t s_resource_stats;

void usos_resource_monitor_init(void)
{
    memset(&s_resource_stats, 0, sizeof(s_resource_stats));
}

void usos_resource_monitor_record_drop(void)
{
    s_resource_stats.obs_queue_drops++;
}

static uint8_t find_usos_task_cpu(const usos_runtime_stats_t *rt_stats)
{
    if (rt_stats == NULL) {
        return 255U;
    }
    for (uint8_t i = 0U; i < rt_stats->tracked_count; ++i) {
        if (strcmp(rt_stats->tasks[i].name, "usos_task") == 0) {
            return rt_stats->tasks[i].cpu_percent;
        }
    }
    return 255U;
}

void usos_resource_monitor_sample(const usos_runtime_stats_t *rt_stats)
{
    uint32_t depth = usos_pal_obs_queue_depth();
    s_resource_stats.obs_queue_depth = depth;
    if (depth > s_resource_stats.obs_queue_peak) {
        s_resource_stats.obs_queue_peak = depth;
    }

    s_resource_stats.usos_task_stack_hwm_bytes = usos_pal_get_own_stack_hwm();
    s_resource_stats.usos_task_cpu_percent = find_usos_task_cpu(rt_stats);
    s_resource_stats.active_sensor_count = usos_registry_count();
    s_resource_stats.static_ram_bytes =
        usos_pal_static_ram_bytes() +
        usos_sensor_monitor_static_ram() +
        usos_hw_monitor_static_ram() +
        usos_runtime_monitor_static_ram() +
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
