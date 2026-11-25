#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usos.h"

/* Existing application driver prototypes (unchanged) */
int DHT_Read(float *temperature);
int BMP280_ReadPressure(float *pressure_hpa);
int HCSR04_ReadDistance(float *distance_cm);
int LDR_ReadRaw(float *adc_raw);

/* Single declarative integration boundary */
USOS_ADAPTER(
    .board = USOS_BOARD_ESP32,
    .device_id = "esp32-baseline-01",
    .sensors = {
        USOS_SENSOR_DHT22("dht_main", .address_or_pin = 4,
                          .min_rate_ms = 2000, .max_rate_ms = 10000,
                          .adaptive_allowed = true, .wrapped_symbol = "DHT_Read"),
        USOS_SENSOR_BMP280("bmp_env", .bus_port = 0, .address_or_pin = 0x76,
                           .min_rate_ms = 500, .max_rate_ms = 5000,
                           .adaptive_allowed = true, .wrapped_symbol = "BMP280_ReadPressure"),
        USOS_SENSOR_HCSR04("ultrasonic", .address_or_pin = 5,
                           .min_rate_ms = 200, .max_rate_ms = 2000,
                           .adaptive_allowed = false, .required_rate_ms = 500,
                           .wrapped_symbol = "HCSR04_ReadDistance"),
        USOS_SENSOR_LDR("ldr_light", .address_or_pin = 34,
                        .min_rate_ms = 1000, .max_rate_ms = 10000,
                        .adaptive_allowed = true, .wrapped_symbol = "LDR_ReadRaw")
    }
);

/* Stub drivers live in drivers.c (separate translation unit for --wrap) */

extern int __real_DHT_Read(float *);
extern int __real_BMP280_ReadPressure(float *);
extern int __real_HCSR04_ReadDistance(float *);
extern int __real_LDR_ReadRaw(float *);

static int tramp_dht(float *v, void *c)  { (void)c; return __real_DHT_Read(v); }
static int tramp_bmp(float *v, void *c)  { (void)c; return __real_BMP280_ReadPressure(v); }
static int tramp_hc(float *v, void *c)   { (void)c; return __real_HCSR04_ReadDistance(v); }
static int tramp_ldr(float *v, void *c)  { (void)c; return __real_LDR_ReadRaw(v); }

int __wrap_DHT_Read(float *v)            { return usos_wrap_dispatch("DHT_Read", tramp_dht, NULL, v); }
int __wrap_BMP280_ReadPressure(float *v) { return usos_wrap_dispatch("BMP280_ReadPressure", tramp_bmp, NULL, v); }
int __wrap_HCSR04_ReadDistance(float *v) { return usos_wrap_dispatch("HCSR04_ReadDistance", tramp_hc, NULL, v); }
int __wrap_LDR_ReadRaw(float *v)         { return usos_wrap_dispatch("LDR_ReadRaw", tramp_ldr, NULL, v); }

void app_main(void)
{
    /* Existing application logic remains unchanged — zero USOS_* API calls */
    float temperature = 0.0f, pressure = 0.0f, distance = 0.0f, light = 0.0f;

    while (1) {
        DHT_Read(&temperature);
        BMP280_ReadPressure(&pressure);
        HCSR04_ReadDistance(&distance);
        LDR_ReadRaw(&light);

        printf("App readings -> T=%.1fC P=%.1fhPa D=%.1fcm L=%.0f\n",
               temperature, pressure, distance, light);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
