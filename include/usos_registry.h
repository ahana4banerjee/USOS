#ifndef USOS_REGISTRY_H
#define USOS_REGISTRY_H

#include "usos_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the static sensor registry from declarative adapter config.
 * Validates sampling interval bounds and resets runtime counters.
 * @return 0 on success, negative on invalid configuration.
 */
int usos_registry_init(const usos_adapter_config_t *config);

/**
 * @brief Number of registered sensors.
 */
uint8_t usos_registry_count(void);

/**
 * @brief Get mutable registry entry by index, or NULL if out of bounds.
 */
usos_sensor_entry_t *usos_registry_get(uint8_t index);

/**
 * @brief Find sensor index by string ID, or -1 if not found.
 */
int usos_registry_find_by_id(const char *id);

/**
 * @brief Find sensor index by wrapped C symbol name, or -1 if not found.
 */
int usos_registry_find_by_symbol(const char *symbol);

/**
 * @brief Update registry state after a real driver read completes.
 */
void usos_registry_update_sample(uint8_t index, float value, int32_t status_code,
                                 uint32_t now_ms);

/**
 * @brief Evaluate whether a wrapped driver call should return a cached reading.
 * Enforces pass-through default (cache_enabled == false) and rejects cached
 * returns after a failed initial read unless allow_stale_on_init_fail == true.
 */
bool usos_registry_should_use_cache(uint8_t index, uint32_t now_ms,
                                    float *out_value, uint32_t *out_orig_ts_ms,
                                    bool *out_is_stale);

#ifdef __cplusplus
}
#endif

#endif /* USOS_REGISTRY_H */
