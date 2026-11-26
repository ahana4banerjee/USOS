# USOS Project Journal & Changelog

> **Purpose:** This is a stage-by-stage learning journal for the **USOS (Universal Sensor Operating System)** B.Tech 4th Year Major Project (ECE). It records **what** was built, **why** key engineering choices were made, **how** things work under the hood, and **what** lessons were learned—to support project understanding, report writing, and viva/presentation preparation.
>
> *Updated at the completion of every stage with dates.*

---

## [Phase 4-1 — Stage 3: Observation Engine] — 2026-11-26

### What We Built
- **Observation Engine (`include/usos_observation.h`, `src/observation/usos_observation.c`):** Central orchestrator running inside `usos_supervisor_task` that consumes `usos_obs_event_t` events from `usos_obs_queue` and runs a periodic observation tick (`CONFIG_USOS_OBS_PERIOD_MS`, default `1000 ms`).
- **Four Specialized Monitors (`src/observation/`):**
  - **Sensor Monitor (`usos_sensor_monitor.c`):** Tracks total/success/error/timeout/consecutive-error counts, rolling min/max/avg latency, `NaN` and out-of-range counts, stuck-value detection, rate-of-change (`|dx/dt|`), and missed expected samples.
  - **Hardware Monitor (`usos_hw_monitor.c`):** Tracks per-bus (`GPIO`, `I2C`, `SPI`, `UART`, `ADC`) transactions, errors, NACK/timeouts, and bus availability.
  - **Runtime Monitor (`usos_runtime_monitor.c`):** Periodically snapshots FreeRTOS task count, task states, CPU %, per-task stack high-water marks, free/min heap, and watchdog status.
  - **USOS Resource Monitor (`usos_resource_monitor.c`):** Measures USOS's own overhead (`FR-005`)—static RAM bytes, `obs_queue` depth/peak/drops, `usos_task` stack high-water mark, and `usos_task` CPU %.
- **PAL & Config Extensions:** Extended `usos_pal_rtos.h`, `platform/esp32_freertos/usos_pal_esp32.c`, and `platform/mock/usos_pal_mock.c`; added `examples/baseline_sensors/sdkconfig.defaults` enabling FreeRTOS trace facility and runtime stats.
- **Stage 3 Host Unit Tests (`tests/unit/test_stage3.c`):** Added tests for sensor anomalies, bus error/recovery tracking, and runtime/resource monitoring.

### Why We Made These Choices
- **Why separate the 4 monitors in pure C11:** Keeping Sensor, Hardware, Runtime, and Resource monitors independent of FreeRTOS headers lets us test every counter, anomaly rule, and RAM calculation on a PC without hardware, while giving Stage 4 (Health & Fault Engine) clean read-only snapshots.
- **Why measure USOS's own overhead (`usos_resource_monitor`):** The central research question of USOS is whether reliability middleware can stay lightweight on constrained MCUs. Tracking static RAM, queue peaks/drops, and `usos_task` stack/CPU from Stage 3 onward makes that trade-off directly measurable.
- **Why `CONFIG_USOS_MAX_TRACKED_TASKS=24` with static `TaskStatus_t` buffer:** FreeRTOS `uxTaskGetSystemState()` returns `0` if the buffer is smaller than the total number of tasks (ESP32 SMP apps routinely run 10–18 system/Wi-Fi tasks). A static 24-entry buffer avoids `malloc` while ensuring snapshots succeed.

### How It Works & Verification Results
- **Event Flow:** Application wrapper → non-blocking `usos_pal_obs_queue_send()` (recording a drop in `usos_resource_monitor` if full) → `usos_supervisor_task` pops event → `usos_observation_process_event()` updates Sensor + Hardware monitors → periodic `usos_observation_tick()` samples Runtime + Resource monitors and logs `[USOS_OBS] ...`.
- **Host Unit Tests:** Compiled with `-Wall -Wextra -Werror` and passed all Stage 2 and Stage 3 tests (`test_sensor_monitor_metrics_and_anomalies`, `test_hw_monitor_bus_errors_and_recovery`, `test_runtime_and_resource_monitors`).
- **ESP-IDF Build:** `examples/baseline_sensors` built cleanly (`0x2c3a0` bytes, 83% app partition free) with `sdkconfig.defaults` active—ready to observe live `[USOS_OBS]` metrics in Wokwi.

---

## [Phase 4-1 — Stage 2: Integration Layer] — 2026-11-26

### What We Built
- **Build System & Repository Setup:** Added the MIT `LICENSE`, ESP-IDF component `CMakeLists.txt`, `Kconfig` (compile-time limits for max sensors, queue size, and task stack/priority), and `cmake/usos_cmake_helpers.cmake` (`usos_register_adapter()`).
- **Single-Header Declarative API (`include/`):** Created `usos.h`, `usos_types.h`, `usos_registry.h`, `usos_pal_rtos.h`, and `usos_pal_hw.h` so an application integrates USOS using only `#include <usos.h>` and `USOS_ADAPTER(...)`.
- **Static Sensor Registry (`src/registry/usos_registry.c`):** Fixed-size table tracking each sensor's configuration, health state, current sampling interval, fault count, and cache state.
- **Startup & Driver Interception (`src/integration/`):**
  - `usos_startup.c`: Intercepts `app_main()` via `__wrap_app_main()` to initialize USOS before user code runs.
  - `usos_wrappers.c`: Generic `usos_wrap_dispatch()` measuring driver latency, updating the registry, pushing observation events, and enforcing caching rules.
- **Platform Abstraction Layer (`platform/`):**
  - `platform/esp32_freertos/usos_pal_esp32.c`: FreeRTOS static task/queue implementation (`xTaskCreateStatic`, `xQueueCreateStatic`) and ESP32 timer/heap/GPIO hooks.
  - `platform/mock/usos_pal_mock.c`: Software mock of the RTOS/HW layer for running unit tests on a PC.
- **Examples & Tests:**
  - `examples/stage2_wrap_proof/`: Minimal ESP-IDF project proving `--wrap=app_main` and `--wrap=DHT_Read` (alongside an unwrapped function `Sensor_UnwrappedPing`).
  - `examples/baseline_sensors/`: 4-sensor demo (DHT22, BMP280, HC-SR04, LDR) with zero `USOS_*` calls inside `app_main()`, configured for Wokwi simulation.
  - `tests/unit/test_stage2.c`: Host unit tests for registry bounds, pass-through default, and opt-in caching rules.

### Why We Made These Choices
- **Why `--wrap=app_main` instead of GCC constructors (`__attribute__((constructor))`):** GCC constructors run too early—before the FreeRTOS scheduler starts and in an unpredictable order across files. Wrapping `app_main` runs *after* FreeRTOS is active and all static data is ready, while still sparing the developer from calling `USOS_Init()`.
- **Why static memory allocation (`xTaskCreateStatic`, `xQueueCreateStatic`):** Microcontrollers with limited RAM suffer from heap fragmentation if middleware allocates/frees memory repeatedly. Static allocation makes USOS's RAM footprint 100% predictable at compile time.
- **Why pass-through by default (`cache_enabled = false`):** Returning cached sensor values silently changes how fresh the application's data is, which can be dangerous for timing-sensitive or safety-critical sensors. Pass-through preserves original behavior; caching is strictly opt-in.
- **Why a Platform Abstraction Layer (PAL):** Keeping FreeRTOS/ESP-IDF calls out of `src/registry/` allows core logic to compile and run unit tests on a Windows PC in seconds without flashing hardware.

### How It Works & Key Lessons Learned
1. **Linker `--wrap` requires separate `.c` files:** When we first put `DHT_Read()` and `app_main()` in the same `main.c` file, Wokwi did not print the wrapper message! GCC optimized/inlined the call inside `main.c` before the linker ever saw it. Moving drivers into `drivers.c` created an external symbol reference between `.obj` files, which `ld --wrap=DHT_Read` successfully redirected to `__wrap_DHT_Read`.
2. **ESP-IDF component naming is case-sensitive:** Because our root folder is named `USOS`, `main/CMakeLists.txt` needed `REQUIRES USOS` (not `usos`).
3. **Fail-closed integration:** If a developer includes `<usos.h>` without calling `usos_register_adapter()` in CMake, `#ifndef USOS_CMAKE_ADAPTER_REGISTERED` triggers a clear compile error instead of silently running without USOS.
4. **MinGW GCC space-in-path quirk on Windows:** WinLibs `gcc.exe` installed under `C:\Users\Ahana Banerjee\...` failed in CMake because `ld.exe` split `default-manifest.o`'s path at the space. Using a custom `-specs` file without `default-manifest.o` let us compile and run all host unit tests cleanly.

### Verification Results
- **Host Unit Tests:** All 3 test groups passed (`test_registry_init_and_bounds`, `test_passthrough_default`, `test_opt_in_caching_and_init_fail_guard`).
- **ESP-IDF Builds & Disassembly:** Both `stage2_wrap_proof` and `baseline_sensors` built cleanly; `xtensa-esp32-elf-objdump` confirmed `app_main` calls `__wrap_DHT_Read` and `Sensor_UnwrappedPing` directly.
- **Wokwi Simulation:** Serial monitor output verified USOS initialization, single `app_main` entry, and `[PROOF] __wrap_DHT_Read intercepted call!` on `stage2_wrap_proof`, plus 4-sensor readings on `baseline_sensors`.

---

## [Phase 4-1 — Stage 1: Architecture & Specification] — 2026-11-25

### What We Built
- Established the complete engineering baseline before writing any C/C++ code:
  - **Primary Docs:** `SRS.md` (requirements with unique IDs `FR-*`, `NFR-*`, `ADP-*`, etc.), `Architecture.md` (12-module layered design + Mermaid diagrams), `Roadmap.md` (Phase 4-1 & 4-2 milestones), and `README.md` (project overview tailored for B.Tech 4th Year ECE Major Project).
  - **Supporting Docs:** `CONTRIBUTING.md`, `.gitignore`, `docs/design/API_Specification.md`, `docs/testing/Test_Strategy.md`, `docs/architecture/ADR-001` & `ADR-002`, and `STAGE_1_REVIEW.md`.

### Why We Did Documentation First
- Embedded middleware touches hardware, RTOS tasks, and application code simultaneously. Defining exact module boundaries, non-goals, and data contracts upfront prevents tight coupling and gives clear metrics for the final B.Tech project evaluation (comparing a Conventional Fixed-Rate System vs. USOS).

### Key Decisions & Corrections
- **ESP-IDF with CMake over PlatformIO:** Chosen because ESP-IDF exposes FreeRTOS runtime stats (`sdkconfig`), native `-Wl,--wrap` linker control, Task Watchdog APIs, and `idf.py size-components` for measuring overhead.
- **Realistic 3-tier driver interception:** Instead of claiming "universal magic interception" (impossible for `static inline` or C++ methods), USOS uses (1) linker `--wrap` for C drivers, (2) explicit wrappers/shims for C++/inline drivers, and (3) HAL/bus hooks.
- **Adaptive sampling interval semantics:** Standardized `min_rate_ms` as the **minimum interval (fastest sampling)** and `max_rate_ms` as the **maximum interval (slowest sampling)**. Stable operation *increases* the interval toward `max_rate_ms` to save resources; rapid changes or degraded health *decrease* it toward `min_rate_ms`.
- **Safe task recovery & no mandatory ML:** FreeRTOS task restart is opt-in only (`task_restart_safe = true`) to avoid leaking mutexes/heap, and Phase 4-1 core uses deterministic algorithms (TinyML is optional in Phase 4-2).
