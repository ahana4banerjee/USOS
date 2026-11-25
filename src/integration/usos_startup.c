#include "usos_types.h"
#include "usos_registry.h"
#include "usos_pal_rtos.h"
#include <stdbool.h>
#include <stddef.h>

extern const usos_adapter_config_t __usos_adapter_config;
extern void __real_app_main(void);

static bool s_usos_initialized = false;

static void usos_supervisor_task(void *arg)
{
    (void)arg;
    usos_obs_event_t ev;

    /* Event-driven: waits on observation queue; never touches peripherals at boot */
    for (;;) {
        if (usos_pal_obs_queue_recv(&ev, 1000U)) {
            /* Stage 3+ will route events into Observation, Health, Fault, Recovery */
            (void)ev;
        }
    }
}

/**
 * @brief Linker-wrapped entry point replacing direct call to app_main().
 * Guarantees queues/registry initialize before usos_task starts, and
 * invokes __real_app_main() exactly once.
 */
void __wrap_app_main(void)
{
    if (!s_usos_initialized) {
        if (usos_registry_init(&__usos_adapter_config) != 0) {
            usos_pal_log("USOS", "FATAL: Invalid USOS_ADAPTER configuration");
            return;
        }
        if (usos_pal_rtos_init() != 0) {
            usos_pal_log("USOS", "FATAL: RTOS PAL init failed");
            return;
        }
        if (usos_pal_task_start(usos_supervisor_task, NULL) != 0) {
            usos_pal_log("USOS", "FATAL: Failed to start usos_task");
            return;
        }
        s_usos_initialized = true;
        usos_pal_log("USOS", "Initialized (%u sensors registered)",
                     (unsigned)usos_registry_count());
    }

    __real_app_main();
}
