#ifndef USOS_OBSERVATION_H
#define USOS_OBSERVATION_H

#include "usos_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Observation Engine and all four sub-monitors.
 */
void usos_observation_init(void);

/**
 * @brief Process a single observation event from usos_obs_queue.
 */
void usos_observation_process_event(const usos_obs_event_t *ev);

/**
 * @brief Periodic observation tick (missed-sample checks, runtime, resource).
 */
void usos_observation_tick(uint32_t now_ms);

/**
 * @brief Read-only accessors for downstream Reliability / Adaptive engines.
 */
const usos_sensor_stats_t *usos_observation_get_sensor(uint8_t sensor_index);
const usos_hw_bus_stats_t *usos_observation_get_bus(usos_bus_type_t bus);
const usos_runtime_stats_t *usos_observation_get_runtime(void);
const usos_resource_stats_t *usos_observation_get_resource(void);
uint32_t usos_observation_static_ram(void);

#ifdef __cplusplus
}
#endif

#endif /* USOS_OBSERVATION_H */
