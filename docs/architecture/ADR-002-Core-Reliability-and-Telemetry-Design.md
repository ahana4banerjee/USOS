# ADR-002: Core Reliability, Memory, and Telemetry Design

**Status:** Approved  
**Date:** 2026-11-25  
**Related Requirements:** `NFR-001`–`NFR-004`, `HLT-001`–`HLT-004`, `REC-001`–`REC-004`, `TEL-001`–`TEL-004`

---

## 1. Context

USOS must provide continuous health scoring, fault detection, bounded recovery, and remote telemetry on an ESP32 running FreeRTOS without destabilizing or blocking the host application.

---

## 2. Approved Decisions

1. **Static Allocation & Bounded Buffers:** All USOS core data structures (sensor registry, fault history ring buffer, observation queue, telemetry queue, and `usos_task` stack/TCB) use compile-time static allocation and fixed-size bounded buffers. Dynamic heap allocation (`malloc`/`free`) is prohibited in the core reliability hot path.
2. **Deterministic Core Intelligence (No Mandatory ML):** Phase 4-1 health scoring uses configurable weighted factors (`0–100`) with hysteresis; fault detection uses deterministic thresholds and lightweight statistics. TinyML/ML is strictly deferred to Phase 4-2 Stage 4 as an optional, compile-time-gated experiment.
3. **Opt-In Task Restart (`task_restart_safe`):** Recovery escalates `RETRY → REINIT_DRIVER → RESET_HW → TASK_RESTART → ISOLATED` with configurable `max_retries` and `cooldown_ms`. FreeRTOS task restart is **disabled by default** and permitted only when the developer explicitly sets `task_restart_safe = true` in `USOS_ADAPTER`, preventing leaked heap or locked shared mutexes.
4. **Batched JSON over MQTT Telemetry:** Core engines push compact fixed-size events to a bounded `usos_tel_queue`. An asynchronous MQTT task serializes events into batched JSON payloads and publishes when connected. If disconnected or queue-full, oldest periodic snapshots are dropped while critical fault/recovery events are preserved, ensuring zero impact on autonomous local operation.

---

## 3. Consequences
- **Positive:** Predictable RAM footprint, host-testable deterministic behavior, safe task lifecycle boundaries, and seamless FastAPI/PostgreSQL ingestion.
- **Trade-off:** JSON payloads are larger than binary encodings (e.g., CBOR), mitigated by batching and selective event-driven reporting.
