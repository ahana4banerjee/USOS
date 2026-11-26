#ifndef USOS_RESOURCE_MONITOR_H
#define USOS_RESOURCE_MONITOR_H

#include "usos_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void usos_resource_monitor_init(void);
void usos_resource_monitor_record_drop(void);
void usos_resource_monitor_sample(const usos_runtime_stats_t *rt_stats);
const usos_resource_stats_t *usos_resource_monitor_get(void);
uint32_t usos_resource_monitor_static_ram(void);

#ifdef __cplusplus
}
#endif

#endif /* USOS_RESOURCE_MONITOR_H */
