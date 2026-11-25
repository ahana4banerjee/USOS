# Software Requirements Specification (SRS)
## Universal Sensor Operating System (USOS)

**Document Version:** 1.0.0-draft  
**Status:** Proposed — Pending Review & Approval (Phase 4-1, Stage 1)  
**Primary Source of Truth:** [USOS Complete Project Design.md](file:///d:/Projects/College/USOS/USOS%20Complete%20Project%20Design.md)

---

## 1. Introduction

### 1.1 Purpose
This Software Requirements Specification (SRS) defines the functional, non-functional, hardware, software, interface, reliability, and validation requirements for the **Universal Sensor Operating System (USOS)**. It serves as the authoritative engineering baseline for architecture design, implementation, testing, and evaluation across Phase 4-1 (Core Reliability Middleware) and Phase 4-2 (Intelligent USOS).

### 1.2 Document Conventions
- **Requirement Identifiers:** Every requirement has a unique, traceable ID (e.g., `FR-001`, `NFR-001`, `HW-001`, `SW-001`, `CON-001`, `IF-001`, `INT-001`, `HLT-001`, `FLT-001`, `REC-001`, `ADP-001`, `RTM-001`, `TEL-001`, `DSH-001`, `PRF-001`, `TST-001`).
- **Priority / Scope Tags:**
  - `[CORE / MANDATORY]` — Required for Phase 4-1 core middleware completion.
  - `[OPTIONAL / PHASE 4-2]` — Deferred to Phase 4-2 or explicitly optional; the core project is complete without it.
- **Unresolved Targets:** Numerical performance targets that cannot be established prior to prototyping and benchmarking are marked `TBD` along with the methodology by which they will be determined.

### 1.3 Intended Audience
Embedded systems developers, RTOS engineers, academic evaluators/mentors, QA/test engineers, and open-source contributors.

---

## 2. Project Overview

USOS is a **lightweight embedded reliability middleware and monitoring framework** designed to operate underneath existing embedded applications. It is **not** a replacement for an RTOS; the initial reference implementation targets **FreeRTOS** on the **ESP32** microcontroller.

USOS operates a continuous five-stage reliability loop (**Observe → Assess → Decide → Act → Report**) across sensors, drivers, hardware interfaces, FreeRTOS tasks, and system resources, while exposing a minimal declarative integration boundary (`#include <usos.h>` and `USOS_ADAPTER(...)`) to the application developer.

---

## 3. Problem Statement

As embedded systems scale in sensor count, peripheral complexity, and concurrent RTOS tasks, developers must manually implement and maintain fragmented reliability logic—driver retries, sensor resets, watchdogs, stack/heap checks, logging, and remote dashboards—tightly coupled to application business logic. Furthermore, conventional systems rely on fixed sampling and monitoring frequencies, wasting CPU, memory, and bus bandwidth during stable operation while failing to increase diagnostic visibility during instability.

USOS addresses this by centralizing observation, health assessment, fault detection, bounded self-healing, adaptive sampling/monitoring, and telemetry into a low-overhead middleware layer without requiring application rewrites.

---

## 4. Project Objectives

1. **Minimal Integration Effort:** Enable integration via `#include <usos.h>` and a declarative `USOS_ADAPTER(...)` configuration without rewriting existing application logic.
2. **Low Resource Overhead:** Maintain minimal RAM, Flash, and CPU footprint suitable for resource-constrained MCUs.
3. **Continuous System Observability:** Monitor sensors, hardware buses, FreeRTOS runtime, and USOS's own resource usage.
4. **Unified Health Representation:** Convert raw observations into deterministic health states (`HEALTHY`, `DEGRADED`, `FAULT`, `RECOVERY`) and transparent factor-based health scores.
5. **Automated Bounded Recovery:** Detect common faults and execute bounded, non-looping recovery actions.
6. **Adaptive Sampling & Monitoring:** Dynamically adjust sampling and monitoring frequencies within developer-defined safety constraints.
7. **Autonomous Local Operation:** Operate reliably even when external telemetry or dashboard connectivity is unavailable.
8. **Quantitative Evaluation:** Enable empirical comparison against conventional fixed-rate embedded applications.

---

## 5. Scope

### 5.1 In Scope (Core — Phase 4-1)
- Declarative `USOS_ADAPTER` integration layer for supported driver/build patterns.
- Sensor registry and monitoring for DHT22, BMP280, HC-SR04, LDR (ADC), and optional MPU6050.
- Hardware bus monitoring (I2C, SPI, UART, GPIO/ADC status).
- FreeRTOS runtime and resource monitoring (tasks, CPU, stack, heap, timing, watchdogs, USOS self-overhead).
- Deterministic health scoring and threshold/statistical fault detection.
- Bounded recovery state machine (retry, driver reinit, sensor/peripheral reset, task restart where safe, isolation).
- Constraint-bounded adaptive sampling and monitoring.
- MQTT telemetry client, FastAPI backend, PostgreSQL storage, and React + Vite dashboard.

### 5.2 In Scope (Extended / Optional — Phase 4-2)
- Advanced adaptive policies, sensor prioritization, and degradation/trend analysis.
- Cross-component fault correlation and historical analytics.
- Optional TinyML anomaly detection, predictive failure detection, and sensor fusion (explicitly non-mandatory).

---

## 6. Functional Requirements

| ID | Requirement | Tag | Verification |
| :--- | :--- | :--- | :--- |
| `FR-001` | USOS shall operate as a middleware library beneath an existing FreeRTOS application without replacing the FreeRTOS kernel or scheduler. | `[CORE / MANDATORY]` | Architecture & build inspection |
| `FR-002` | USOS shall execute the continuous Observe → Assess → Decide → Act → Report reliability loop autonomously on the target MCU. | `[CORE / MANDATORY]` | Integration test on Wokwi / ESP32 |
| `FR-003` | USOS shall maintain an internal sensor and component registry tracking ID, type, driver, interface, state, sampling rate, health, last success timestamp, fault count, and recovery count. | `[CORE / MANDATORY]` | Unit test |
| `FR-004` | USOS shall continue full local observation, fault detection, adaptation, and recovery when MQTT/dashboard connectivity is unavailable. | `[CORE / MANDATORY]` | Network-disconnect fault injection test |
| `FR-005` | USOS shall monitor and expose its own resource consumption (RAM, CPU overhead, queue utilization, active monitors). | `[CORE / MANDATORY]` | Runtime benchmark test |
| `FR-006` | USOS shall support cross-component fault correlation (e.g., attributing simultaneous I2C sensor failures to an I2C bus fault). | `[OPTIONAL / PHASE 4-2]` | Multi-sensor fault injection test |
| `FR-007` | USOS may optionally support TinyML-based anomaly detection or sensor fusion only after all Phase 4-1 core requirements pass. | `[OPTIONAL / PHASE 4-2]` | Optional evaluation |

---

## 7. Non-Functional Requirements

| ID | Requirement | Tag | Verification |
| :--- | :--- | :--- | :--- |
| `NFR-001` | **Modularity:** Core reliability logic (Health, Fault, Adaptive, Recovery) shall be decoupled from MCU/RTOS-specific code via a Platform Abstraction Layer (PAL). | `[CORE / MANDATORY]` | Host unit tests compile without ESP32/FreeRTOS headers |
| `NFR-002` | **Deterministic Memory:** Core data structures shall prefer static allocation and fixed-size bounded buffers/queues; dynamic heap allocation in the hot path shall be avoided. | `[CORE / MANDATORY]` | Static analysis & heap-allocation trace |
| `NFR-003` | **Bounded Execution:** All recovery and monitoring loops shall have explicit upper bounds on retries, cooldowns, and execution time. | `[CORE / MANDATORY]` | Unit & stress tests |
| `NFR-004` | **Non-Intrusiveness:** USOS failures or telemetry backpressure shall not block or crash the host application's critical control tasks. | `[CORE / MANDATORY]` | Fault injection & stress test |
| `NFR-005` | **Configurability:** Monitoring intervals, health weights, fault thresholds, and recovery limits shall be configurable at compile time via `USOS_ADAPTER`. | `[CORE / MANDATORY]` | Configuration build matrix test |

---

## 8. Hardware Requirements

| ID | Requirement | Tag | Notes |
| :--- | :--- | :--- | :--- |
| `HW-001` | **Primary MCU:** ESP32 development board (dual-core Xtensa LX6, Wi-Fi, I2C, SPI, UART, ADC, GPIO). | `[CORE / MANDATORY]` | Reference hardware target |
| `HW-002` | **Temperature/Humidity Sensor:** DHT22 (single-wire digital timing protocol). | `[CORE / MANDATORY]` | Used for timing/checksum fault scenarios |
| `HW-003` | **Pressure/Temperature Sensor:** BMP280 (I2C / SPI). | `[CORE / MANDATORY]` | Used for I2C bus reliability & latency |
| `HW-004` | **Ultrasonic Distance Sensor:** HC-SR04 (GPIO trigger/echo timing). | `[CORE / MANDATORY]` | Used for GPIO/timeout fault scenarios |
| `HW-005` | **Light Sensor:** LDR with voltage-divider resistor (ESP32 ADC). | `[CORE / MANDATORY]` | Used for analog range/stuck-value detection |
| `HW-006` | **IMU Sensor (Optional):** MPU6050 (I2C). | `[OPTIONAL]` | Used for high-rate adaptive sampling validation |
| `HW-007` | **Simulation Environment:** Wokwi simulator supporting ESP32 and virtualized sensors. | `[CORE / MANDATORY]` | Used for initial CI/pre-hardware validation |

---

## 9. Software Requirements

| ID | Requirement | Tag | Notes |
| :--- | :--- | :--- | :--- |
| `SW-001` | **Embedded Language:** C (C11) / C++ (C++17) compatible with ESP-IDF / FreeRTOS toolchains. | `[CORE / MANDATORY]` | Core library in C/C++ with clean C linkage |
| `SW-002` | **RTOS:** FreeRTOS (ESP-IDF FreeRTOS SMP or standard FreeRTOS). | `[CORE / MANDATORY]` | Execution & synchronization foundation |
| `SW-003` | **Build Framework:** ESP-IDF (recommended) or PlatformIO; CMake for build orchestration and host tests. | `[CORE / MANDATORY]` | See ADR-001 for framework selection |
| `SW-004` | **Embedded Testing:** Unity / CMock (or equivalent C/C++ unit test framework) for host-side core testing. | `[CORE / MANDATORY]` | Runs in CI without hardware |
| `SW-005` | **Telemetry Protocol:** MQTT (over Wi-Fi/TCP). | `[CORE / MANDATORY]` | Device-to-backend transport |
| `SW-006` | **Backend & Storage:** Python FastAPI + PostgreSQL. | `[CORE / MANDATORY]` | Telemetry ingestion, REST/WS API, history |
| `SW-007` | **Frontend Dashboard:** React + Vite + Recharts (or Chart.js). | `[CORE / MANDATORY]` | Web visualization |
| `SW-008` | **Analysis Tooling:** Python scripts for benchmarking and Conventional-vs-USOS evaluation. | `[CORE / MANDATORY]` | Phase 4-2 Stage 6 |

---

## 10. System Constraints

| ID | Constraint |
| :--- | :--- |
| `CON-001` | **No Universal Binary Interception:** USOS cannot transparently intercept arbitrary closed-source or inline C/C++ driver functions without build-system or wrapper integration. |
| `CON-002` | **No Application Semantic Understanding:** USOS monitors observable system/sensor/runtime properties and shall not attempt to interpret application business logic. |
| `CON-003` | **Physical Fault Boundary:** Software recovery cannot repair physically damaged hardware; USOS must detect, isolate, and safely report non-recoverable faults. |
| `CON-004` | **Sampling Safety Bounds:** Adaptive sampling shall never violate developer-specified `min_rate`, `max_rate`, `required_rate`, or `adaptive_allowed` constraints. |
| `CON-005` | **No Mandatory ML in Core:** Machine learning / TinyML shall not be required for Phase 4-1 core operation. |

---

## 11. Operating Environment

- **Target Runtime:** ESP32 running FreeRTOS with Wi-Fi connectivity (optional at runtime; required only for remote telemetry).
- **Host / Server Environment:** Local or networked host running an MQTT broker (e.g., Eclipse Mosquitto), FastAPI service, PostgreSQL database, and React/Vite web UI.
- **Simulation Environment:** Wokwi ESP32 simulator for deterministic functional and fault-injection testing prior to physical hardware deployment.

---

## 12. System Interfaces

| ID | Interface | Description |
| :--- | :--- | :--- |
| `IF-001` | **Developer Integration Interface** | Single header `#include <usos.h>` and declarative `USOS_ADAPTER(...)` macro/struct block. |
| `IF-002` | **Driver Hook / Wrapper Interface** | Standardized interception hooks (linker `--wrap`, wrapper functions, or driver callbacks) capturing call start, end, return status, and value. |
| `IF-003` | **Platform Abstraction Layer (PAL)** | Abstracted interface to FreeRTOS primitives (tasks, queues, timers, tick count, stack/heap stats) and HAL (I2C/SPI/UART/GPIO/ADC/reset). |
| `IF-004` | **MQTT Telemetry Interface** | Structured JSON (or compact binary + JSON bridge) payloads published to defined MQTT topics (`usos/<device_id>/...`). |
| `IF-005` | **Dashboard Backend API** | FastAPI REST and WebSocket endpoints serving live and historical telemetry from PostgreSQL to the React frontend. |

---

## 13. Sensor Integration Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `INT-001` | A developer shall be able to integrate USOS into an existing application by adding `#include <usos.h>` and `USOS_ADAPTER(...)` without rewriting existing sensor read calls in `main()` or application tasks. | `[CORE / MANDATORY]` |
| `INT-002` | `USOS_ADAPTER` shall declaratively accept: MCU/board identifier, sensor list, driver binding/integration mode, bus/pin configuration, sampling constraints (`min_rate`, `max_rate`, `required_rate`, `adaptive_allowed`), valid value ranges, and optional reset/reinit callbacks. | `[CORE / MANDATORY]` |
| `INT-003` | USOS shall NOT require application code to call `USOS_Init()`, `USOS_ReadSensor()`, `USOS_GetHealth()`, `USOS_Update()`, `USOS_RegisterEvent()`, or `USOS_SetSamplingRate()` during normal operation. | `[CORE / MANDATORY]` |
| `INT-004` | USOS shall guarantee transparent integration for explicitly documented and supported driver/build patterns on the reference ESP32/FreeRTOS platform. | `[CORE / MANDATORY]` |
| `INT-005` | For drivers that cannot be linker-wrapped, `USOS_ADAPTER` shall support an explicit wrapper or callback binding within the adapter configuration boundary. | `[CORE / MANDATORY]` |

---

## 14. Health Monitoring Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `HLT-001` | USOS shall maintain a discrete health state per monitored component and for the overall system: `HEALTHY`, `DEGRADED`, `FAULT`, and `RECOVERY`. | `[CORE / MANDATORY]` |
| `HLT-002` | USOS shall compute a numerical health score in the range `0–100` using a configurable deterministic weighted model. | `[CORE / MANDATORY]` |
| `HLT-003` | USOS shall retain and expose the underlying health factors (communication success rate, error count, latency, missing samples, value validity, runtime/memory health) alongside the aggregate score so scores are never opaque. | `[CORE / MANDATORY]` |
| `HLT-004` | Health state transitions shall include hysteresis or consecutive-sample thresholds to prevent rapid oscillation (flapping) between states. | `[CORE / MANDATORY]` |

---

## 15. Fault Detection Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `FLT-001` | USOS shall detect sensor-level faults: communication failure, timeout, invalid/NaN value, out-of-range reading, stuck/frozen value, and repeated driver failures. | `[CORE / MANDATORY]` |
| `FLT-002` | USOS shall detect hardware-bus faults: I2C/SPI/UART transaction errors, bus lockups/NACKs, and peripheral unavailability. | `[CORE / MANDATORY]` |
| `FLT-003` | USOS shall detect runtime faults: FreeRTOS task starvation/hang, stack high-water-mark near-overflow, heap exhaustion/memory pressure, timing violations, and watchdog events. | `[CORE / MANDATORY]` |
| `FLT-004` | Each detected fault shall be classified by component, fault type, severity, and timestamp, and recorded in a bounded fault history ring buffer. | `[CORE / MANDATORY]` |
| `FLT-005` | Phase 4-1 fault detection shall use deterministic threshold and lightweight statistical rules (e.g., moving average, rate-of-change, consecutive failure counters). | `[CORE / MANDATORY]` |

---

## 16. Recovery Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `REC-001` | Upon fault detection, USOS shall execute a tiered, bounded recovery sequence appropriate to the fault class: (1) communication retry, (2) driver reinitialization, (3) sensor/peripheral hardware/power reset (if configured), (4) task restart (only where marked safe), (5) component isolation & safe fallback notification. | `[CORE / MANDATORY]` |
| `REC-002` | Every recovery action shall enforce configurable `max_retries`, `cooldown_ms`, and `escalation_level` limits to prevent infinite recovery loops. | `[CORE / MANDATORY]` |
| `REC-003` | Following a recovery action, USOS shall perform a verification check before transitioning the component back to `HEALTHY`. | `[CORE / MANDATORY]` |
| `REC-004` | If maximum recovery attempts are exhausted, USOS shall transition the component to an isolated `FAULT` state, stop active recovery attempts until cooldown/manual reset, and emit a fault-isolation event. | `[CORE / MANDATORY]` |

---

## 17. Adaptive Sampling Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `ADP-001` | USOS shall dynamically adjust internal monitoring frequency and (where adapter-controlled or supported by driver hooks) sensor sampling frequency based on health state, rate of value change, fault history, and resource pressure. | `[CORE / MANDATORY]` |
| `ADP-002` | Adaptation shall strictly respect per-sensor adapter constraints: if `adaptive_allowed == false` or `required_rate` is fixed, USOS shall not alter that sensor's sampling rate. | `[CORE / MANDATORY]` |
| `ADP-003` | Under `HEALTHY` and stable signal conditions, USOS shall step down sampling/monitoring toward `min_rate` to conserve CPU, bus, and energy resources. | `[CORE / MANDATORY]` |
| `ADP-004` | Under rapid signal change, `DEGRADED` health, or suspected `FAULT`, USOS shall step up sampling/monitoring toward `max_rate` for diagnostic visibility, and gradually step back down after recovery stabilizes. | `[CORE / MANDATORY]` |
| `ADP-005` | Every rate change shall log the old rate, new rate, and adaptation reason for telemetry and evaluation. | `[CORE / MANDATORY]` |

---

## 18. Runtime Monitoring Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `RTM-001` | USOS shall monitor FreeRTOS task states, per-task stack high-water marks, and overall CPU utilization (via FreeRTOS run-time stats). | `[CORE / MANDATORY]` |
| `RTM-002` | USOS shall monitor system heap usage (free heap, minimum ever free heap) and detect memory pressure thresholds. | `[CORE / MANDATORY]` |
| `RTM-003` | USOS shall integrate with FreeRTOS / ESP-IDF Task Watchdog Timer (TWDT) events or software heartbeat checks to detect stalled tasks. | `[CORE / MANDATORY]` |
| `RTM-004` | USOS shall measure task execution timing / jitter for monitored sensor tasks where hooks are attached. | `[CORE / MANDATORY]` |

---

## 19. Telemetry Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `TEL-001` | USOS shall collect, serialize, and transmit telemetry events and periodic health snapshots over MQTT when network connectivity is available. | `[CORE / MANDATORY]` |
| `TEL-002` | Telemetry transmission shall be decoupled from critical observation/recovery paths via a bounded asynchronous queue so network stalls never block local reliability logic. | `[CORE / MANDATORY]` |
| `TEL-003` | Telemetry shall support selective/event-driven reporting and batching to minimize Wi-Fi/MQTT bandwidth and CPU overhead. | `[CORE / MANDATORY]` |
| `TEL-004` | Telemetry payloads shall include device registration metadata, sensor values, health states & factor scores, CPU/RAM/stack metrics, fault/recovery events, adaptive rate changes, and USOS self-overhead metrics. | `[CORE / MANDATORY]` |

---

## 20. Dashboard Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `DSH-001` | The FastAPI backend shall subscribe to MQTT telemetry topics, validate payloads, and persist time-series and event records in PostgreSQL. | `[CORE / MANDATORY]` |
| `DSH-002` | The React + Vite dashboard shall display a **System Overview** (overall health score + factors, device status, CPU, RAM, active faults, recovery status). | `[CORE / MANDATORY]` |
| `DSH-003` | The dashboard shall display **Sensor Monitoring** (live values, status, health score, current sampling rate, communication quality, fault history). | `[CORE / MANDATORY]` |
| `DSH-004` | The dashboard shall display **Runtime Monitoring** (FreeRTOS task list/states, CPU utilization, stack high-water marks, heap usage). | `[CORE / MANDATORY]` |
| `DSH-005` | The dashboard shall display **Reliability & Adaptive Behaviour** (fault/recovery timeline, adaptation reasons, current vs. historical sampling rates). | `[CORE / MANDATORY]` |
| `DSH-006` | The dashboard shall support **Historical Analytics** (health trends, fault frequency, recovery success rate, estimated resource savings). | `[OPTIONAL / PHASE 4-2]` |

---

## 21. Performance Requirements

> **Note:** Specific numerical targets are marked `TBD` as required. They will be established empirically during **Phase 4-1 Stage 7** and **Phase 4-2 Stage 6** by benchmarking a baseline ESP32 FreeRTOS application (without USOS) against the same application integrated with USOS.

| ID | Metric | Target | Establishment Methodology |
| :--- | :--- | :--- | :--- |
| `PRF-001` | USOS Static RAM Footprint | `TBD` KB | Map-file & `uxTaskGetStackHighWaterMark` measurement with 4 sensors |
| `PRF-002` | USOS Flash Footprint | `TBD` KB | ESP-IDF `idf.py size-components` comparison (baseline vs. USOS) |
| `PRF-003` | USOS CPU Overhead | `TBD` % | FreeRTOS `vTaskGetRunTimeStats` comparison under nominal load |
| `PRF-004` | Fault Detection Latency | `TBD` ms | Fault-injection timestamp to `FLT` event emission delta |
| `PRF-005` | Recovery Execution Time | `TBD` ms | Fault detection to verification completion delta per fault class |
| `PRF-006` | Adaptive Resource Savings | `TBD` % | Reduction in sensor/bus transactions and CPU active time vs. fixed-rate baseline |

---

## 22. Testing and Validation Requirements

| ID | Requirement | Tag |
| :--- | :--- | :--- |
| `TST-001` | Core platform-independent modules (Health, Fault, Adaptive, Recovery) shall achieve unit test coverage on host using Unity/CMock with mocked PAL. | `[CORE / MANDATORY]` |
| `TST-002` | End-to-end integration and fault-injection scenarios shall be validated in Wokwi simulation before physical hardware testing. | `[CORE / MANDATORY]` |
| `TST-003` | Physical ESP32 hardware tests shall validate real sensor faults (disconnected wire, I2C NACK, timeout, noisy/stuck ADC) for DHT22, BMP280, HC-SR04, and LDR. | `[CORE / MANDATORY]` |
| `TST-004` | Experimental evaluation (Phase 4-2 Stage 6) shall quantitatively compare a Conventional Fixed-Rate System vs. USOS across integration effort, resource overhead, reliability, adaptation, and scalability. | `[CORE / MANDATORY]` |

---

## 23. Limitations

1. **Driver Interception Scope:** Transparent interception is guaranteed only for supported driver/build patterns (e.g., ESP-IDF linker `--wrap` or registered driver wrappers). Arbitrary closed-source binaries or inlined header-only functions cannot be intercepted without an explicit wrapper in `USOS_ADAPTER`.
2. **Application Semantics:** USOS cannot validate whether application control thresholds (e.g., `if (temp > 50)`) are logically correct.
3. **Hardware Damage:** Software recovery cannot fix physically broken sensors or shorted buses; such cases result in fault isolation and reporting.
4. **Health Score Nuance:** A single `0–100` score can mask localized degradation; therefore, factor breakdowns must always accompany the aggregate score.
5. **Platform Coupling:** While core logic is portable C/C++, runtime and peripheral observation depend on FreeRTOS and ESP32 HAL implementations.

---

## 24. Explicit Non-Goals

USOS will **not** attempt to provide:
- A replacement for FreeRTOS or a new RTOS kernel.
- A new programming language or automatic AST/source-code rewriting tool.
- Universal plug-and-play compatibility with every proprietary sensor without adapter configuration.
- Guaranteed recovery from physical hardware destruction.
- Mandatory Machine Learning / TinyML or sensor fusion in the core middleware.
- Mandatory cloud connectivity for local device operation.
- A complex universal sensor API that forces developers to rewrite application code.

---

## 25. Acceptance Criteria

Phase 4-1 (Core USOS) is accepted when:
1. An existing FreeRTOS/ESP32 multi-sensor application integrates USOS using only `#include <usos.h>` and `USOS_ADAPTER(...)` with zero application logic rewrites (`INT-001`, `INT-003`).
2. All four baseline sensors (DHT22, BMP280, HC-SR04, LDR) are observed, scored, and fault-monitored (`HLT-001`–`HLT-004`, `FLT-001`–`FLT-005`).
3. Injected communication, timeout, and range faults trigger bounded recovery and transition back to `HEALTHY` or isolated `FAULT` without recovery loops (`REC-001`–`REC-004`).
4. Adaptive sampling demonstrably reduces sampling/monitoring frequency in stable states and increases it during anomalies within configured bounds (`ADP-001`–`ADP-005`).
5. USOS operates autonomously when MQTT is disconnected (`FR-004`) and streams live telemetry to the React dashboard when connected (`TEL-001`–`DSH-005`).
6. Host unit tests pass and USOS self-overhead metrics are measured and documented (`PRF-001`–`PRF-006`, `TST-001`–`TST-003`).

---

## Appendix A: Identified Missing Specifications & Technical Risks

During analysis of [USOS Complete Project Design.md](file:///d:/Projects/College/USOS/USOS%20Complete%20Project%20Design.md), the following gaps and risks were identified and must be resolved in Stage 1 before implementation:

1. **Build Framework Choice (ESP-IDF vs. PlatformIO):** Both are listed. ESP-IDF provides native CMake, FreeRTOS SMP hooks, linker `--wrap` control, and TWDT APIs; PlatformIO wraps Arduino/ESP-IDF with easier hobbyist library management but less predictable linker-wrapping. *Resolution tracked in ADR-001.*
2. **Exact `USOS_ADAPTER` Syntax & Interception Mechanism:** How `USOS_ADAPTER(...)` hooks into `DHT_Read(&temperature)` without modifying the call site requires a concrete C/C++ mechanism (e.g., GNU `ld --wrap=DHT_Read`, preprocessor macro redirection, or C++ wrapper types). *Resolution tracked in ADR-001 and API_Specification.md.*
3. **Adaptive Sampling Control Boundary:** If the application itself calls `DHT_Read()` inside a `vTaskDelay(1000)` loop, USOS cannot slow down the application's loop unless (a) USOS manages the sampling schedule/cache and serves cached values on rapid calls, or (b) the adapter hooks into the task's delay/timer or driver poll. This subtle distinction must be explicitly designed.
4. **Task Restart Safety in FreeRTOS:** Calling `vTaskDelete` + `xTaskCreate` on an arbitrary task can leak mutexes, heap allocations, or peripheral locks. Task restart must be opt-in per task with cleanup/safe-restart semantics.
5. **Telemetry Serialization Format & Bandwidth:** JSON over MQTT is human-readable and easy for FastAPI, but verbose on ESP32; payload size and publish cadence need explicit bounds.
