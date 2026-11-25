#ifndef USOS_PAL_HW_H
#define USOS_PAL_HW_H

#include "usos_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Attempt bus-level recovery (e.g., I2C clock pulse / driver reset).
 */
int usos_pal_bus_recover(usos_bus_type_t bus, uint8_t port);

/**
 * @brief Pulse a GPIO reset or power-cycle pin.
 */
int usos_pal_gpio_reset(uint16_t pin, uint32_t pulse_ms);

/**
 * @brief Query watchdog health status (0 = OK, negative = fault/timeout).
 */
int usos_pal_wdt_status(void);

#ifdef __cplusplus
}
#endif

#endif /* USOS_PAL_HW_H */
