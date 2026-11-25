# USOS API, Adapter, and Telemetry Specification

**Document Version:** 1.1.0-draft  
**Status:** Stage 1 Baseline (Updated per Stage 1 Technical Review)  
**References:** [SRS.md](file:///d:/Projects/College/USOS/SRS.md) (`IF-001`–`IF-005`, `INT-001`–`INT-005`) | [Architecture.md](file:///d:/Projects/College/USOS/Architecture.md)

---

## 1. Developer-Facing Integration API (`usos.h`)

USOS exposes a **single-header, declarative integration boundary** (`#include <usos.h>`). Application developers do **not** call `USOS_Init()`, `USOS_ReadSensor()`, or `USOS_Update()` inside their application loop. Initialization is handled via static `const` adapter registration combined with `-Wl,--wrap=app_main` (see [ADR-001](file:///d:/Projects/College/USOS/docs/architecture/ADR-001-Build-Framework-and-Integration-Mechanism.md)).

### 1.1 `USOS_ADAPTER(...)` Declarative Configuration (Target C/C++ Structure)

```c
// Conceptual target structure (finalized in Phase 4-1 Stage 2)
typedef struct {
    const char *id;
    usos_sensor_type_t type;       // DHT22, BMP280, HCSR04, LDR, MPU6050
    usos_bus_type_t bus;           // GPIO, I2C, SPI, ADC
    uint8_t bus_port;              // e.g., I2C_NUM_0
    uint16_t address_or_pin;       // I2C addr or GPIO/ADC pin
    float valid_min;
    float valid_max;
    uint32_t min_rate_ms;          // Minimum sampling interval (fastest sampling)
    uint32_t max_rate_ms;          // Maximum sampling interval (slowest sampling)
    uint32_t required_rate_ms;     // Fixed required interval when adaptive_allowed == false
    bool adaptive_allowed;
    bool task_restart_safe;        // Opt-in for task-level recovery (default: false)
    int (*reinit_fn)(void);        // Optional driver reinit callback
    int (*hw_reset_fn)(void);      // Optional hardware/power reset callback
    const char *wrapped_symbol;    // Tier 1: Target C symbol for --wrap (e.g., "DHT_Read")
} usos_sensor_config_t;
```

### 1.2 Sampling Interval Semantics
- `min_rate_ms`: Minimum interval in milliseconds (**fastest** allowed sampling rate).
- `max_rate_ms`: Maximum interval in milliseconds (**slowest** allowed sampling rate).
- `required_rate_ms`: Fixed required interval in milliseconds when `adaptive_allowed == false`.
- Invariant: `min_rate_ms <= current_interval_ms <= max_rate_ms`.

---

## 2. Internal Core Interfaces (Platform-Independent)

### 2.1 Observation Event (`usos_obs_event_t`)
Fixed-size struct pushed non-blockingly (`O(1)`) from wrappers/monitors into `usos_obs_queue`:
- `uint32_t timestamp_ms`
- `uint8_t component_id`
- `usos_obs_domain_t domain` (`SENSOR`, `HARDWARE`, `RUNTIME`, `RESOURCE`)
- `int32_t status_code` (`0` = OK, negative = error/timeout)
- `uint32_t latency_us`
- `float value` (for sensor readings)

### 2.2 Platform Abstraction Layer (`platform/`)
- `usos_pal_rtos.h`: `usos_pal_tick_ms()`, `usos_pal_heap_free()`, `usos_pal_heap_min()`, `usos_pal_task_snapshot()`, `usos_pal_queue_send_isr_safe()`.
- `usos_pal_hw.h`: `usos_pal_bus_recover(bus, port)`, `usos_pal_gpio_reset(pin, pulse_ms)`, `usos_pal_wdt_status()`.

---

## 3. MQTT Telemetry Contract (`IF-004`)

### 3.1 Topic Hierarchy
- `usos/<device_id>/register` — Device & sensor metadata (retained, on boot)
- `usos/<device_id>/health` — Periodic system + per-component health scores & factor breakdowns
- `usos/<device_id>/sensors` — Batched sensor readings & current adaptive sampling intervals (`interval_ms`)
- `usos/<device_id>/runtime` — FreeRTOS CPU %, free heap, stack high-water marks, USOS self-overhead
- `usos/<device_id>/events` — Fault detection, recovery transitions, and adaptation reason events

### 3.2 Example Health & Event Payloads (Batched JSON)

```json
{
  "device_id": "esp32-lab-01",
  "ts_ms": 124500,
  "system_health": 82,
  "state": "DEGRADED",
  "factors": {
    "sensor_comm": 95,
    "runtime_health": 88,
    "memory_health": 91
  },
  "components": [
    {"id": "dht_main", "score": 62, "state": "DEGRADED", "interval_ms": 1000, "faults": 2}
  ]
}
```

```json
{
  "device_id": "esp32-lab-01",
  "ts_ms": 124520,
  "event": "RECOVERY",
  "component": "dht_main",
  "fault_type": "COMM_TIMEOUT",
  "action": "REINIT_DRIVER",
  "attempt": 1,
  "result": "SUCCESS"
}
```
