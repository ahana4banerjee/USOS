#ifndef USOS_PAL_RTOS_H
#define USOS_PAL_RTOS_H

#include "usos_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*usos_task_fn_t)(void *arg);

/**
 * @brief Initialize static RTOS primitives (observation queue, locks).
 * Must be called before starting the USOS supervisor task.
 * @return 0 on success, negative on error.
 */
int usos_pal_rtos_init(void);

/**
 * @brief Start the statically allocated USOS supervisor task.
 * @return 0 on success, negative on error.
 */
int usos_pal_task_start(usos_task_fn_t fn, void *arg);

/**
 * @brief Non-blocking push of an observation event to the USOS queue.
 * Safe to call from application tasks without blocking.
 */
bool usos_pal_obs_queue_send(const usos_obs_event_t *event);

/**
 * @brief Blocking/timed receive of an observation event in usos_task.
 */
bool usos_pal_obs_queue_recv(usos_obs_event_t *out_event, uint32_t timeout_ms);

/**
 * @brief Current monotonic time in milliseconds.
 */
uint32_t usos_pal_tick_ms(void);

/**
 * @brief Current monotonic time in microseconds (for driver latency).
 */
uint32_t usos_pal_tick_us(void);

/**
 * @brief Free heap and minimum-ever free heap in bytes.
 */
uint32_t usos_pal_heap_free(void);
uint32_t usos_pal_heap_min(void);

/**
 * @brief Query current observation queue depth (number of queued items).
 */
uint32_t usos_pal_obs_queue_depth(void);

/**
 * @brief Read and reset the observation queue peak depth for the last interval.
 */
uint32_t usos_pal_obs_queue_peak_and_reset(void);

/**
 * @brief Query calling task's stack high-water mark in bytes.
 */
uint32_t usos_pal_get_own_stack_hwm(void);

/**
 * @brief Populate FreeRTOS task, CPU %, and heap snapshot.
 * @return 0 on success, negative on error.
 */
int usos_pal_get_runtime_stats(usos_runtime_stats_t *out_stats);

/**
 * @brief Return static RAM bytes owned by the PAL (queues + task stack/TCB).
 */
uint32_t usos_pal_static_ram_bytes(void);

/**
 * @brief Lightweight diagnostic logging hook.
 */
void usos_pal_log(const char *tag, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* USOS_PAL_RTOS_H */
