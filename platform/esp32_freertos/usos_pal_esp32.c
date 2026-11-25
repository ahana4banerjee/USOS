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
