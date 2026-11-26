#ifndef USOS_HW_MONITOR_H
#define USOS_HW_MONITOR_H

#include "usos_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void usos_hw_monitor_init(void);
void usos_hw_monitor_record_tx(usos_bus_type_t bus, int32_t status_code, uint32_t now_ms);
void usos_hw_monitor_process(const usos_obs_event_t *ev);
const usos_hw_bus_stats_t *usos_hw_monitor_get(usos_bus_type_t bus);
uint32_t usos_hw_monitor_static_ram(void);

#ifdef __cplusplus
}
#endif

#endif /* USOS_HW_MONITOR_H */
