#include "usos_pal_rtos.h"
#include "usos_pal_hw.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static uint32_t s_mock_ms = 0;
static uint32_t s_mock_us = 0;
static usos_obs_event_t s_mock_queue[USOS_OBS_QUEUE_LEN];
static uint32_t s_q_head = 0;
static uint32_t s_q_tail = 0;
static uint32_t s_q_count = 0;

void usos_mock_reset(void)
{
    s_mock_ms = 0;
    s_mock_us = 0;
    s_q_head = 0;
    s_q_tail = 0;
    s_q_count = 0;
    memset(s_mock_queue, 0, sizeof(s_mock_queue));
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
    return 131072U;
}

uint32_t usos_pal_heap_min(void)
{
    return 110592U;
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
