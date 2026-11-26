#include "usos_runtime_monitor.h"
#include "usos_pal_rtos.h"
#include "usos_pal_hw.h"
#include <string.h>

static usos_runtime_stats_t s_runtime_stats;

void usos_runtime_monitor_init(void)
{
    memset(&s_runtime_stats, 0, sizeof(s_runtime_stats));
    s_runtime_stats.total_cpu_percent = 255U;
}

void usos_runtime_monitor_sample(uint32_t now_ms)
{
    if (usos_pal_get_runtime_stats(&s_runtime_stats) != 0) {
        s_runtime_stats.free_heap_bytes = usos_pal_heap_free();
        s_runtime_stats.min_free_heap_bytes = usos_pal_heap_min();
    }
    s_runtime_stats.timestamp_ms = now_ms;
    s_runtime_stats.wdt_status = usos_pal_wdt_status();
}

const usos_runtime_stats_t *usos_runtime_monitor_get(void)
{
    return &s_runtime_stats;
}

uint32_t usos_runtime_monitor_static_ram(void)
{
    return (uint32_t)sizeof(s_runtime_stats);
}
