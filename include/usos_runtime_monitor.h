#ifndef USOS_RUNTIME_MONITOR_H
#define USOS_RUNTIME_MONITOR_H

#include "usos_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void usos_runtime_monitor_init(void);
void usos_runtime_monitor_sample(uint32_t now_ms);
const usos_runtime_stats_t *usos_runtime_monitor_get(void);
uint32_t usos_runtime_monitor_static_ram(void);

#ifdef __cplusplus
}
#endif

#endif /* USOS_RUNTIME_MONITOR_H */
