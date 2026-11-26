#include "usos_pal_rtos.h"
#include "usos_pal_hw.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef CONFIG_USOS_TASK_STACK_SIZE
#define CONFIG_USOS_TASK_STACK_SIZE 4096
#endif

#ifndef CONFIG_USOS_TASK_PRIORITY
#define CONFIG_USOS_TASK_PRIORITY 3
#endif

#define USOS_STACK_WORDS (CONFIG_USOS_TASK_STACK_SIZE / sizeof(StackType_t))

static StaticQueue_t s_obs_queue_cb;
static uint8_t s_obs_queue_storage[USOS_OBS_QUEUE_LEN * sizeof(usos_obs_event_t)];
static QueueHandle_t s_obs_queue = NULL;

static StaticTask_t s_usos_task_tcb;
static StackType_t s_usos_task_stack[USOS_STACK_WORDS];
static TaskHandle_t s_usos_task_handle = NULL;

int usos_pal_rtos_init(void)
{
    s_obs_queue = xQueueCreateStatic(
        USOS_OBS_QUEUE_LEN,
        sizeof(usos_obs_event_t),
        s_obs_queue_storage,
        &s_obs_queue_cb
    );
    return (s_obs_queue != NULL) ? 0 : -1;
}

int usos_pal_task_start(usos_task_fn_t fn, void *arg)
{
    if (fn == NULL || s_obs_queue == NULL) {
        return -1;
    }

    s_usos_task_handle = xTaskCreateStatic(
        fn,
        "usos_task",
        USOS_STACK_WORDS,
        arg,
        CONFIG_USOS_TASK_PRIORITY,
        s_usos_task_stack,
        &s_usos_task_tcb
    );
    return (s_usos_task_handle != NULL) ? 0 : -1;
}

bool usos_pal_obs_queue_send(const usos_obs_event_t *event)
{
    if (s_obs_queue == NULL || event == NULL) {
        return false;
    }
    return (xQueueSend(s_obs_queue, event, 0) == pdTRUE);
}

bool usos_pal_obs_queue_recv(usos_obs_event_t *out_event, uint32_t timeout_ms)
{
    if (s_obs_queue == NULL || out_event == NULL) {
        return false;
    }
    return (xQueueReceive(s_obs_queue, out_event, pdMS_TO_TICKS(timeout_ms)) == pdTRUE);
}

uint32_t usos_pal_tick_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000LL);
}

uint32_t usos_pal_tick_us(void)
{
    return (uint32_t)esp_timer_get_time();
}

uint32_t usos_pal_heap_free(void)
{
    return esp_get_free_heap_size();
}

uint32_t usos_pal_heap_min(void)
{
    return esp_get_minimum_free_heap_size();
}

uint32_t usos_pal_obs_queue_depth(void)
{
    if (s_obs_queue == NULL) {
        return 0U;
    }
    return (uint32_t)uxQueueMessagesWaiting(s_obs_queue);
}

uint32_t usos_pal_get_own_stack_hwm(void)
{
    return (uint32_t)uxTaskGetStackHighWaterMark(NULL) * (uint32_t)sizeof(StackType_t);
}

#if configUSE_TRACE_FACILITY
static TaskStatus_t s_task_status_buf[USOS_MAX_TRACKED_TASKS];

static uint8_t map_task_state(eTaskState st)
{
    switch (st) {
        case eRunning:   return 0U;
        case eReady:     return 1U;
        case eBlocked:   return 2U;
        case eSuspended: return 3U;
        case eDeleted:
        default:         return 4U;
    }
}
#endif

int usos_pal_get_runtime_stats(usos_runtime_stats_t *out_stats)
{
    if (out_stats == NULL) {
        return -1;
    }

    out_stats->free_heap_bytes = esp_get_free_heap_size();
    out_stats->min_free_heap_bytes = esp_get_minimum_free_heap_size();
    out_stats->task_count = (uint16_t)uxTaskGetNumberOfTasks();
    out_stats->tracked_count = 0U;
    out_stats->total_cpu_percent = 255U;

#if configUSE_TRACE_FACILITY
    uint32_t total_runtime = 0U;
    UBaseType_t n = uxTaskGetSystemState(s_task_status_buf, USOS_MAX_TRACKED_TASKS,
                                         &total_runtime);
    uint8_t idle_cpu = 0U;
    for (UBaseType_t i = 0U; i < n && i < USOS_MAX_TRACKED_TASKS; ++i) {
        usos_task_info_t *dst = &out_stats->tasks[out_stats->tracked_count++];
        strncpy(dst->name, s_task_status_buf[i].pcTaskName, sizeof(dst->name) - 1U);
        dst->name[sizeof(dst->name) - 1U] = '\0';
        dst->state = map_task_state(s_task_status_buf[i].eCurrentState);
        dst->stack_hwm_bytes =
            (uint32_t)s_task_status_buf[i].usStackHighWaterMark * (uint32_t)sizeof(StackType_t);

#if configGENERATE_RUN_TIME_STATS
        if (total_runtime > 0U) {
            uint32_t pct = (uint32_t)(((uint64_t)s_task_status_buf[i].ulRunTimeCounter * 100ULL) /
                                      (uint64_t)total_runtime);
            dst->cpu_percent = (pct > 100U) ? 100U : (uint8_t)pct;
            if (strncmp(dst->name, "IDLE", 4) == 0) {
                idle_cpu = (uint8_t)(idle_cpu + dst->cpu_percent);
            }
        } else {
            dst->cpu_percent = 255U;
        }
#else
        dst->cpu_percent = 255U;
#endif
    }

#if configGENERATE_RUN_TIME_STATS
    if (total_runtime > 0U) {
        uint8_t avg_idle = (idle_cpu > 100U) ? 100U : idle_cpu;
        out_stats->total_cpu_percent = (uint8_t)(100U - avg_idle);
    }
#endif
#endif

    return 0;
}

uint32_t usos_pal_static_ram_bytes(void)
{
    return (uint32_t)(sizeof(s_obs_queue_cb) + sizeof(s_obs_queue_storage) +
                      sizeof(s_usos_task_tcb) + sizeof(s_usos_task_stack));
}

void usos_pal_log(const char *tag, const char *fmt, ...)
{
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ESP_LOGI(tag != NULL ? tag : "USOS", "%s", buf);
}

int usos_pal_bus_recover(usos_bus_type_t bus, uint8_t port)
{
    (void)bus;
    (void)port;
    /* Implemented in Stage 5 Recovery Engine */
    return 0;
}

int usos_pal_gpio_reset(uint16_t pin, uint32_t pulse_ms)
{
    gpio_set_level((gpio_num_t)pin, 0);
    vTaskDelay(pdMS_TO_TICKS(pulse_ms));
    gpio_set_level((gpio_num_t)pin, 1);
    return 0;
}

int usos_pal_wdt_status(void)
{
    return 0;
}
