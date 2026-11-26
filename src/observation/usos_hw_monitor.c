#include "usos_hw_monitor.h"
#include <stddef.h>
#include <string.h>

static usos_hw_bus_stats_t s_bus_stats[USOS_BUS_COUNT];

void usos_hw_monitor_init(void)
{
    memset(s_bus_stats, 0, sizeof(s_bus_stats));
    for (uint8_t i = 0U; i < USOS_BUS_COUNT; ++i) {
        s_bus_stats[i].bus = (usos_bus_type_t)i;
        s_bus_stats[i].available = true;
    }
}

void usos_hw_monitor_record_tx(usos_bus_type_t bus, int32_t status_code, uint32_t now_ms)
{
    if ((uint8_t)bus >= USOS_BUS_COUNT || bus == USOS_BUS_NONE) {
        return;
    }

    usos_hw_bus_stats_t *st = &s_bus_stats[(uint8_t)bus];
    st->tx_count++;

    if (status_code == 0) {
        st->consecutive_errors = 0U;
        st->available = true;
    } else {
        st->error_count++;
        st->consecutive_errors++;
        st->last_error_code = status_code;
        st->last_error_ms = now_ms;
        if (status_code <= -2) {
            st->nack_or_timeout_count++;
        }
        if (st->consecutive_errors >= 3U) {
            st->available = false;
        }
    }
}

void usos_hw_monitor_process(const usos_obs_event_t *ev)
{
    if (ev == NULL) {
        return;
    }
    usos_bus_type_t bus = (usos_bus_type_t)ev->component_id;
    usos_hw_monitor_record_tx(bus, ev->status_code, ev->timestamp_ms);
}

const usos_hw_bus_stats_t *usos_hw_monitor_get(usos_bus_type_t bus)
{
    if ((uint8_t)bus >= USOS_BUS_COUNT) {
        return NULL;
    }
    return &s_bus_stats[(uint8_t)bus];
}

uint32_t usos_hw_monitor_static_ram(void)
{
    return (uint32_t)sizeof(s_bus_stats);
}
