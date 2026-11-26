#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

int DHT_Read(float *temperature);
int BMP280_ReadPressure(float *pressure_hpa);
int HCSR04_ReadDistance(float *distance_cm);
int LDR_ReadRaw(float *adc_raw);

static uint8_t measure_cpu_percent(uint16_t *out_tasks)
{
    static TaskStatus_t buf[24];
    uint32_t total_runtime = 0;
    UBaseType_t n = uxTaskGetSystemState(buf, 24, &total_runtime);
    *out_tasks = (uint16_t)uxTaskGetNumberOfTasks();

    if (n == 0 || total_runtime == 0) {
        return 255;
    }

    uint32_t idle_pct = 0;
    for (UBaseType_t i = 0; i < n; ++i) {
        if (strncmp(buf[i].pcTaskName, "IDLE", 4) == 0) {
            idle_pct += (uint32_t)(((uint64_t)buf[i].ulRunTimeCounter * 100ULL) / total_runtime);
        }
    }
    return (idle_pct > 100U) ? 0U : (uint8_t)(100U - idle_pct);
}

void app_main(void)
{
    float temperature = 0.0f, pressure = 0.0f, distance = 0.0f, light = 0.0f;

    while (1) {
        DHT_Read(&temperature);
        BMP280_ReadPressure(&pressure);
        HCSR04_ReadDistance(&distance);
        LDR_ReadRaw(&light);

        uint16_t tasks = 0;
        uint8_t cpu = measure_cpu_percent(&tasks);
        printf("App readings -> T=%.1fC P=%.1fhPa D=%.1fcm L=%.0f\n",
               temperature, pressure, distance, light);
        printf("[BASELINE_NO_USOS] tasks=%u cpu=%u%% heap=%luB(min=%luB)\n",
               (unsigned)tasks, (unsigned)cpu,
               (unsigned long)esp_get_free_heap_size(),
               (unsigned long)esp_get_minimum_free_heap_size());

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
