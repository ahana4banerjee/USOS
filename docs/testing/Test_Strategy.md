# USOS Test and Experimental Evaluation Strategy

**Document Version:** 1.0.0  
**Status:** Frozen — Stage 1 Baseline  
**References:** [SRS.md](file:///d:/Projects/College/USOS/SRS.md) (`TST-001`–`TST-004`, `PRF-001`–`PRF-006`) | [Roadmap.md](file:///d:/Projects/College/USOS/Roadmap.md)

---

## 1. Testing Pyramid

| Level | Scope | Tools | Target Environment |
| :--- | :--- | :--- | :--- |
| **1. Host Unit Tests** | Health scoring, hysteresis, fault rules, adaptive clamping, recovery state machine, ring buffers. | Unity / CMock, CMake, GCC | Host OS (CI, no hardware) |
| **2. Simulation Integration** | Full USOS + FreeRTOS + virtual sensors (DHT22, BMP280, HC-SR04, LDR) + fault injection. | Wokwi ESP32 Simulator | Virtual ESP32 |
| **3. Hardware-in-the-Loop** | Physical ESP32 + real sensors, real I2C NACKs, disconnected wires, Wi-Fi drop tests. | ESP32, Logic Analyzer / Serial | Physical Bench |
| **4. End-to-End Dashboard** | MQTT → FastAPI → PostgreSQL → React UI verification. | Mosquitto, pytest, Playwright/manual | Host + ESP32 |

---

## 2. Fault Injection Scenarios (Mandatory Validation)

1. **DHT22 Disconnected / Checksum Error:** Verify `COMM_TIMEOUT` / invalid value detection, bounded retry → reinit → `ISOLATED` without infinite loop.
2. **BMP280 I2C Bus Hang / NACK:** Verify I2C error counter, bus recovery (`usos_pal_bus_recover`), and return to `HEALTHY`.
3. **HC-SR04 Echo Timeout / Out-of-Range:** Verify `TIMEOUT` and `OUT_OF_RANGE` fault classification.
4. **LDR Stuck / Rail Short (0 or 4095):** Verify stuck/range fault detection.
5. **Runtime Memory / Stack Stress:** Allocate controlled heap pressure / deep stack task to verify `MEMORY_PRESSURE` and `STACK_NEAR_OVERFLOW` alerts.
6. **MQTT / Wi-Fi Disconnect:** Drop broker connection and verify local observation, adaptation, and recovery continue uninterrupted (`FR-004`).

---

## 3. Phase 4-2 Stage 6: Experimental Evaluation Methodology

To answer USOS's central engineering question quantitatively, an identical 4-sensor ESP32 application will be benchmarked in two configurations:
- **Baseline (Conventional):** Fixed 1 Hz sampling, manual error checks, no USOS middleware.
- **USOS-Integrated:** Same application logic with `#include <usos.h>` and `USOS_ADAPTER(...)`.

Python analysis scripts will compare:
1. **Integration Effort:** Application lines of code (LOC) modified, integration steps.
2. **Resource Overhead:** Static RAM, heap usage, Flash size (`idf.py size-components`), CPU % (`vTaskGetRunTimeStats`), task count (`PRF-001`–`PRF-003`).
3. **Reliability:** Fault detection latency (`PRF-004`), recovery time (`PRF-005`), recovery success rate, false-positive rate.
4. **Adaptation:** Reduction in sensor transactions, bus traffic, and CPU active time during stable vs. faulted periods (`PRF-006`).
5. **Scalability:** RAM and CPU growth from 1 to 5+ sensors.
