# ADR-001: Build Framework, Startup Initialization, and Driver Integration Mechanism

**Status:** Approved (Framework & Integration) / Recommended (Startup Mechanism)  
**Date:** 2026-11-25  
**Related Requirements:** `SW-003`, `CON-001`, `INT-001`–`INT-005`

---

## 1. Context

USOS targets **ESP32 + FreeRTOS** in **C/C++** and requires:
1. Deep access to FreeRTOS runtime statistics (`vTaskGetRunTimeStats`, stack high-water marks, Task Watchdog Timer).
2. Minimal, declarative integration (`#include <usos.h>`, `USOS_ADAPTER(...)`) without requiring the application developer to call `USOS_Init()` or rewrite existing sensor calls.

Three foundational decisions are documented here:
1. Primary build framework.
2. Safe ESP-IDF startup initialization mechanism.
3. Tiered driver interception mechanism.

---

## 2. Decision 1: Primary Build Framework — **APPROVED**

**Decision:** Standardize on **ESP-IDF with CMake** as the primary build framework.

- **Rationale:**
  - Native `sdkconfig` control over FreeRTOS trace/runtime-stats (`configGENERATE_RUN_TIME_STATS`, `configUSE_TRACE_FACILITY`, `INCLUDE_uxTaskGetStackHighWaterMark`).
  - First-class CMake support for `-Wl,--wrap=<symbol>` linker wrapping.
  - Native `esp_task_wdt`, `esp_timer`, and `esp-mqtt` components, plus `idf.py size-components` for exact Flash/RAM overhead measurement.
  - Direct Wokwi ESP32 simulation compatibility.

---

## 3. Decision 2: ESP-IDF Startup Initialization Mechanism — **RECOMMENDED**

### 3.1 Evaluation of Initialization Options
To preserve the requirement that developers do **not** call `USOS_Init()` in application code, three mechanisms were evaluated against the ESP-IDF startup sequence:

1. **GCC `__attribute__((constructor))` [REJECTED]:**
   - *Timing in ESP-IDF:* Runs during `do_global_ctors()` in early startup *before* `vTaskStartScheduler()` is active, before secondary CPU core (CPU1) startup, and before `esp_timer`, NVS, and network/peripheral subsystems are ready.
   - *Risks:* Suffers from the C/C++ **Static Initialization Order Fiasco** across translation units—`USOS_ADAPTER` structs in another file may not yet be initialized when a constructor runs. Creating FreeRTOS tasks or calling driver/HAL APIs from a constructor is unsafe.

2. **`ESP_SYSTEM_INIT_FN(...)` [NOT SELECTED AS PRIMARY]:**
   - *Timing in ESP-IDF:* Official internal ESP-IDF startup hook (`CORE` / `SECONDARY` stages) with deterministic priority ordering.
   - *Limitations:* Still executes *before* `app_main()` runs user peripheral initialization (`i2c_driver_install()`, GPIO setup, `nvs_flash_init()`), and relies on private ESP-IDF startup headers (`esp_private/startup_internal.h`) that can change across ESP-IDF minor releases.

3. **Pure Static `const` `USOS_ADAPTER` + Linker-Wrapped `app_main` (`-Wl,--wrap=app_main`) [RECOMMENDED]:**
   - *Timing in ESP-IDF:* In ESP-IDF, the FreeRTOS `main_task` invokes `app_main()` **after**:
     - FreeRTOS scheduler is running on CPU0 and CPU1.
     - All C/C++ global constructors (`do_global_ctors()`) have completed.
     - Heap, IPC, `esp_timer`, and system services are initialized.
   - *Mechanism:*
     - `USOS_ADAPTER(...)` expands to a pure compile-time `static const usos_adapter_config_t` struct (zero runtime constructor, zero static initialization order fiasco).
     - USOS wraps `app_main` via `-Wl,--wrap=app_main` (using the same CMake `--wrap` infrastructure used for driver hooks).
     - `__wrap_app_main()` initializes USOS static queues and `usos_task` (`usos_core_init()`), then immediately invokes `__real_app_main()`.
     - **Driver Initialization Safety:** `usos_task` remains event-driven (blocked on `usos_obs_queue`) and does not access sensor hardware until the application's `__real_app_main()` has initialized peripherals and begun calling sensor drivers.

### 3.2 Recommendation
Adopt **Pure Static `const` `USOS_ADAPTER` + `-Wl,--wrap=app_main`** as the default ESP-IDF initialization mechanism.

---

## 4. Decision 3: Tiered Driver Interception Strategy — **APPROVED**

USOS explicitly does **not** claim universal transparent interception of arbitrary software. Instead, it uses a **three-tier integration strategy** behind `USOS_ADAPTER(...)`:

1. **Tier 1 — GNU Linker `--wrap` (`-Wl,--wrap=<symbol>`):** Primary mechanism for supported C driver functions (e.g., `DHT_Read`, `BMP280_ReadPressure`). Zero changes to application call sites.
2. **Tier 2 — Explicit Adapter-Bound Wrappers / Shims:** Required for C++ class methods, `static inline` header-only drivers, or third-party binaries where `--wrap` cannot intercept.
3. **Tier 3 — Hardware / HAL-Level Hooks:** I2C/SPI/UART/ADC/TWDT error and status hooks where bus-level visibility is needed.

### Supported Integration Patterns & Limitations
- **Supported:** Standard C driver functions linked as object/archive symbols; explicit C/C++ wrapper bindings declared in `USOS_ADAPTER`; ESP-IDF peripheral drivers.
- **Limitations:** `static inline` functions, LTO-inlined calls across same translation unit without `--wrap` visibility, and closed-source precompiled C++ virtual methods require Tier 2 explicit wrappers.

---

## 5. Consequences
- **Positive:** Eliminates `USOS_Init()` from user code while guaranteeing FreeRTOS scheduler readiness, deterministic static config order, and safe peripheral initialization sequencing.
- **Trade-off:** Requires CMake integration (`usos_register_adapter()` or `CMakeLists.txt` symbol list) to pass `--wrap` flags to the linker.
