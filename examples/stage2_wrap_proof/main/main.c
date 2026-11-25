#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usos.h"

/* Simulated C driver functions (one wrapped, one unwrapped) */
int DHT_Read(float *temperature);
int Sensor_UnwrappedPing(void);

/* Declarative USOS adapter (pure static const; no USOS_Init() in app_main) */
USOS_ADAPTER(
    .board = USOS_BOARD_ESP32,
    .device_id = "esp32-wrap-proof",
    .sensors = {
        USOS_SENSOR_DHT22(
            "dht_proof",
            .address_or_pin = 4,
            .min_rate_ms = 500,
            .max_rate_ms = 5000,
            .adaptive_allowed = true,
            .wrapped_symbol = "DHT_Read"
        )
    }
);

/* Real driver implementations live in drivers.c (separate translation unit) */

/* Tier 1 linker wrapper for DHT_Read */
extern int __real_DHT_Read(float *temperature);

static int dht_real_trampoline(float *out_val, void *ctx)
{
    (void)ctx;
    return __real_DHT_Read(out_val);
}

int __wrap_DHT_Read(float *temperature)
{
    printf("[PROOF] __wrap_DHT_Read intercepted call!\n");
    return usos_wrap_dispatch("DHT_Read", dht_real_trampoline, NULL, temperature);
}

void app_main(void)
{
    static int app_main_calls = 0;
    app_main_calls++;
    printf("[PROOF] app_main entered (call count=%d)\n", app_main_calls);

    float temp = 0.0f;
    int rc = DHT_Read(&temp);
    int ping = Sensor_UnwrappedPing();

    printf("[PROOF] DHT_Read rc=%d temp=%.2f | UnwrappedPing=%d\n", rc, temp, ping);

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
