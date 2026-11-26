#include "usos_pal_rtos.h"
#include "usos_pal_hw.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static uint32_t s_mock_ms = 0;
static uint32_t s_mock_us = 0;
static uint32_t s_mock_heap_free = 131072U;
static uint32_t s_mock_heap_min = 110592U;
static uint32_t s_mock_stack_hwm = 2048U;
static uint8_t s_mock_cpu_pct = 12U;
static usos_obs_event_t s_mock_queue[USOS_OBS_QUEUE_LEN];
static uint32_t s_q_head = 0;
static uint32_t s_q_tail = 0;
static uint32_t s_q_count = 0;

void usos_mock_reset(void)
{
    s_mock_ms = 0;
    s_mock_us = 0;
    s_mock_heap_free = 131072U;
    s_mock_heap_min = 110592U;
    s_mock_stack_hwm = 2048U;
    s_mock_cpu_pct = 12U;
    s_q_head = 0;
    s_q_tail = 0;
    s_q_count = 0;
    memset(s_mock_queue, 0, sizeof(s_mock_queue));
}

void usos_mock_set_runtime(uint32_t heap_free, uint32_t heap_min,
                           uint32_t stack_hwm, uint8_t cpu_pct)
{
    s_mock_heap_free = heap_free;
    s_mock_heap_min = heap_min;
    s_mock_stack_hwm = stack_hwm;
    s_mock_cpu_pct = cpu_pct;
}

void usos_mock_set_time_ms(uint32_t ms)
{
    s_mock_ms = ms;
    s_mock_us = ms * 1000U;
}

uint32_t usos_mock_obs_count(void)
{
    return s_q_count;
}

int usos_pal_rtos_init(void)
{
    usos_mock_reset();
    return 0;
}

int usos_pal_task_start(usos_task_fn_t fn, void *arg)
{
    (void)fn;
    (void)arg;
    return 0;
}

bool usos_pal_obs_queue_send(const usos_obs_event_t *event)
{
    if (event == NULL || s_q_count >= USOS_OBS_QUEUE_LEN) {
        return false;
    }
    s_mock_queue[s_q_tail] = *event;
    s_q_tail = (s_q_tail + 1U) % USOS_OBS_QUEUE_LEN;
    s_q_count++;
    return true;
}

bool usos_pal_obs_queue_recv(usos_obs_event_t *out_event, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (out_event == NULL || s_q_count == 0U) {
        return false;
    }
    *out_event = s_mock_queue[s_q_head];
    s_q_head = (s_q_head + 1U) % USOS_OBS_QUEUE_LEN;
    s_q_count--;
    return true;
}

uint32_t usos_pal_tick_ms(void)
{
    return s_mock_ms;
}

uint32_t usos_pal_tick_us(void)
{
    return s_mock_us;
}

uint32_t usos_pal_heap_free(void)
{
    return s_mock_heap_free;
}

uint32_t usos_pal_heap_min(void)
{
    return s_mock_heap_min;
}

uint32_t usos_pal_obs_queue_depth(void)
{
    return s_q_count;
}

uint32_t usos_pal_get_own_stack_hwm(void)
{
    return s_mock_stack_hwm;
}

int usos_pal_get_runtime_stats(usos_runtime_stats_t *out_stats)
{
    if (out_stats == NULL) {
        return -1;
    }
    out_stats->task_count = 3U;
    out_stats->tracked_count = 1U;
    out_stats->total_cpu_percent = s_mock_cpu_pct;
    out_stats->free_heap_bytes = s_mock_heap_free;
    out_stats->min_free_heap_bytes = s_mock_heap_min;
    strncpy(out_stats->tasks[0].name, "usos_task", sizeof(out_stats->tasks[0].name) - 1U);
    out_stats->tasks[0].name[sizeof(out_stats->tasks[0].name) - 1U] = '\0';
    out_stats->tasks[0].state = 0U;
    out_stats->tasks[0].stack_hwm_bytes = s_mock_stack_hwm;
    out_stats->tasks[0].cpu_percent = 2U;
    return 0;
}

uint32_t usos_pal_static_ram_bytes(void)
{
    return (uint32_t)sizeof(s_mock_queue);
}

void usos_pal_log(const char *tag, const char *fmt, ...)
{
    va_list args;
    printf("[%s] ", tag != NULL ? tag : "USOS");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

int usos_pal_bus_recover(usos_bus_type_t bus, uint8_t port)
{
    (void)bus;
    (void)port;
    return 0;
}

int usos_pal_gpio_reset(uint16_t pin, uint32_t pulse_ms)
{
    (void)pin;
    (void)pulse_ms;
    return 0;
}

int usos_pal_wdt_status(void)
{
    return 0;
}
