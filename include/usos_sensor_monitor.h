#ifndef USOS_SENSOR_MONITOR_H
#define USOS_SENSOR_MONITOR_H

#include "usos_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void usos_sensor_monitor_init(void);
void usos_sensor_monitor_process(const usos_obs_event_t *ev);
void usos_sensor_monitor_check_missed(uint32_t now_ms);
const usos_sensor_stats_t *usos_sensor_monitor_get(uint8_t sensor_index);
uint32_t usos_sensor_monitor_static_ram(void);

#ifdef __cplusplus
}
#endif

#endif /* USOS_SENSOR_MONITOR_H */
