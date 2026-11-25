# Stage 1 Technical Review — USOS

**Academic Context:** B.Tech 4th Year Major Project (ECE) — Phase 4-1 (1st Semester)  
**Stage:** Stage 1 — Architecture and Specification  
**Date:** 2026-11-25  
**Status:** **FROZEN — Stage 1 Baseline Complete**

---

## 1. Finalized Architectural Decisions

| Area | Decision | Status | Reference |
| :--- | :--- | :--- | :--- |
| **Primary Build Framework** | **ESP-IDF with CMake** | Approved | [ADR-001](file:///d:/Projects/College/USOS/docs/architecture/ADR-001-Build-Framework-and-Integration-Mechanism.md) |
| **Driver Interception** | **Three-tier:** (1) GNU `--wrap` for C drivers, (2) explicit adapter-bound wrappers for C++/inline drivers, (3) HAL/HW hooks | Approved | [ADR-001](file:///d:/Projects/College/USOS/docs/architecture/ADR-001-Build-Framework-and-Integration-Mechanism.md) |
| **Startup Initialization** | **Pure `static const` `USOS_ADAPTER` + `-Wl,--wrap=app_main`** | Conditionally Approved *(Stage 2 build proof)* | [ADR-001](file:///d:/Projects/College/USOS/docs/architecture/ADR-001-Build-Framework-and-Integration-Mechanism.md) |
| **CMake Registration** | `usos_register_adapter(WRAP_SYMBOLS ...)` propagating `-Wl,--wrap` to final executable | Concept Approved *(Stage 2 validation)* | [ADR-001](file:///d:/Projects/College/USOS/docs/architecture/ADR-001-Build-Framework-and-Integration-Mechanism.md) |
| **Wrapper Caching Policy** | **Pass-through by default** (`cache_enabled = false`); opt-in per compatible sensor with timestamp & stale flag | Approved | [SRS.md](file:///d:/Projects/College/USOS/SRS.md) (`ADP-006`) |
| **Task Restart Policy** | **Opt-in only** via `task_restart_safe` (disabled by default) | Approved | [ADR-002](file:///d:/Projects/College/USOS/docs/architecture/ADR-002-Core-Reliability-and-Telemetry-Design.md) |
| **Core Memory Strategy** | **Static allocation & bounded buffers/queues** (no `malloc` in hot path) | Approved | [ADR-002](file:///d:/Projects/College/USOS/docs/architecture/ADR-002-Core-Reliability-and-Telemetry-Design.md) |
| **Core Intelligence** | **Deterministic algorithms** (weighted health, threshold/statistical faults); no mandatory ML | Approved | [ADR-002](file:///d:/Projects/College/USOS/docs/architecture/ADR-002-Core-Reliability-and-Telemetry-Design.md) |
| **Telemetry Transport** | **Batched JSON over MQTT** with non-blocking snapshot-drop backpressure | Approved | [ADR-002](file:///d:/Projects/College/USOS/docs/architecture/ADR-002-Core-Reliability-and-Telemetry-Design.md) |
| **Sampling Semantics** | `min_rate_ms` = fastest (min interval); `max_rate_ms` = slowest (max interval); `required_rate_ms` = fixed | Approved | [SRS.md](file:///d:/Projects/College/USOS/SRS.md) |
| **License** | **MIT License** | Approved | [README.md](file:///d:/Projects/College/USOS/README.md) |

---

## 2. Approved Technology Stack

- **MCU & RTOS:** ESP32, FreeRTOS, C/C++ (C11 / C++17), ESP-IDF with CMake
- **Simulation & Testing:** Wokwi ESP32 Simulator, Unity / CMock (host unit tests), Python (benchmarking)
- **Initial Sensors & Buses:** DHT22, BMP280, HC-SR04, LDR (ADC), optional MPU6050; I2C, SPI, UART, GPIO, ADC
- **Telemetry & Dashboard:** MQTT (Mosquitto), FastAPI, PostgreSQL, React + Vite, Recharts / Chart.js

---

## 3. Core Module Boundaries (12 Modules)

1. **Integration / Adapter Layer:** `usos.h`, `USOS_ADAPTER(...)`, `--wrap` & shim bindings.
2. **Sensor Management Layer:** Static sensor/component registry.
3. **Observation Engine:** Normalizes Sensor, Hardware, Runtime, and Resource event streams.
4. **Hardware Monitoring:** Bus errors (I2C/SPI/UART), GPIO/ADC status, peripheral availability.
5. **Runtime Monitoring:** FreeRTOS task states, CPU %, stack high-water marks, heap, TWDT.
6. **Resource Monitoring:** USOS static RAM, queue peaks, USOS task CPU/stack overhead.
7. **Health Assessment Engine:** `HEALTHY / DEGRADED / FAULT / RECOVERY` states, `0–100` factor-weighted score, hysteresis.
8. **Fault Detection Engine:** Deterministic/statistical fault classification & bounded history ring buffer.
9. **Recovery / Self-Healing Engine:** Bounded tiered state machine (`RETRY → REINIT → RESET → TASK_RESTART (opt-in) → ISOLATED`).
10. **Adaptive Sampling Engine:** Interval policy bounded by `[min_rate_ms, max_rate_ms]` or `required_rate_ms`, with pass-through default.
11. **Telemetry Engine:** Bounded async queue, batched JSON serialization, ESP-MQTT client.
12. **Platform Abstraction Layer (PAL):** Narrow OS/HAL interfaces isolating core C/C++ logic from FreeRTOS/ESP-IDF.

---

## 4. Remaining Technical Uncertainties (To Be Resolved in Stage 2 / Evaluation)

1. **ESP-IDF `--wrap=app_main` Linker Proof:** Must be empirically proven in a minimal ESP-IDF project before building Stage 2 modules on top of it.
2. **`usos_register_adapter(WRAP_SYMBOLS ...)` Propagation:** Must be proven to propagate `-Wl,--wrap` to the final ESP-IDF application executable.
3. **Empirical Performance Targets (`PRF-001`–`PRF-006`):** Marked `TBD` in `SRS.md` until Stage 7 and Phase 4-2 Stage 6 benchmarking.

---

## 5. Stage 1 Completion Checklist

- [x] Primary requirements baseline ([SRS.md](file:///d:/Projects/College/USOS/SRS.md)) — **Frozen v1.0.0**
- [x] System & software architecture ([Architecture.md](file:///d:/Projects/College/USOS/Architecture.md)) — **Frozen v1.0.0**
- [x] Milestone-based roadmap ([Roadmap.md](file:///d:/Projects/College/USOS/Roadmap.md)) — **Frozen v1.0.0**
- [x] Repository landing page & contributing rules ([README.md](file:///d:/Projects/College/USOS/README.md), [CONTRIBUTING.md](file:///d:/Projects/College/USOS/CONTRIBUTING.md))
- [x] API/telemetry contract & test strategy ([API_Specification.md](file:///d:/Projects/College/USOS/docs/design/API_Specification.md), [Test_Strategy.md](file:///d:/Projects/College/USOS/docs/testing/Test_Strategy.md)) — **Frozen v1.0.0**
- [x] ADRs finalized ([ADR-001](file:///d:/Projects/College/USOS/docs/architecture/ADR-001-Build-Framework-and-Integration-Mechanism.md), [ADR-002](file:///d:/Projects/College/USOS/docs/architecture/ADR-002-Core-Reliability-and-Telemetry-Design.md)) — **Frozen**
- [x] Adaptive sampling semantics & pass-through caching reconciled across all documents

---

## 6. Mandatory Entry Gates at Start of Stage 2

Before implementing Stage 2 observation/adapter features, execute a minimal ESP-IDF proof-of-concept verifying:
1. **`--wrap=app_main` Proof:**
   - USOS queues and internal structures initialize before `usos_task` starts.
   - `usos_task` does not assume application peripherals are already initialized.
   - `__real_app_main()` is invoked exactly once.
   - Missing build/link configuration fails clearly rather than silently running without USOS.
2. **`usos_register_adapter(WRAP_SYMBOLS ...)` Proof:**
   - One wrapped C driver function + one unwrapped driver function.
   - Successful build showing expected symbol redirection on the final executable.
   - Runtime test confirming both wrapper and original function execute correctly.
3. **Repository Hygiene:** Add the MIT `LICENSE` file.
