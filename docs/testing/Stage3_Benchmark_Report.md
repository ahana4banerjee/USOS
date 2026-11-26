# Stage 3 Benchmark Report — Conventional vs. USOS (ESP32 / ESP-IDF v5.5.6)

**Date:** 2026-11-26  
**Stage:** Phase 4-1, Stage 3 (Observation Engine — Post-Hardening Baseline)  
**Projects Compared:**
- **Conventional (No USOS):** [examples/conventional_baseline](file:///d:/Projects/College/USOS/examples/conventional_baseline/main/main.c)
- **With USOS (Stage 3):** [examples/baseline_sensors](file:///d:/Projects/College/USOS/examples/baseline_sensors/main/main.c)

---

## 1. Build-Time Memory Comparison (`esp_idf_size`)

| Metric | Conventional (No USOS) | With USOS (Stage 3) | Overhead (Delta) | % of ESP32 Budget |
|---|---:|---:|---:|---|
| **Static DRAM (`.data` + `.bss`)** | 13,436 B (7.4%) | 19,980 B (11.1%) | **+6,544 B (~6.39 KB)** | 3.62% of 180,736 B DRAM |
| **IRAM** | 53,723 B (41.0%) | 54,051 B (41.2%) | **+328 B** | 0.25% of 131,072 B IRAM |
| **Flash (`.text` + `.rodata` + `.appdesc`)** | 99,406 B | 103,746 B | **+4,340 B (~4.24 KB)** | 0.41% of 1 MB app partition |
| **Total Image Size (`.map`)** | 163,421 B | 168,089 B | **+4,668 B (~4.56 KB)** | — |
| **`libUSOS.a` Static RAM (`.bss` + `.data`)** | — | 7,411 B (7,403 + 8) | **7,411 B** | Exact match with `ram=7411B` |

---

## 2. Live Runtime Comparison (Wokwi ESP32 Simulation)

### Observed Serial Output

**Without USOS (`conventional_baseline`):**
```text
[BASELINE_NO_USOS] tasks=5 cpu=0% heap=302784B(min=302784B)
App readings -> T=26.5C P=1013.2hPa D=45.0cm L=2048
```
*(Initial first-tick sample at boot printed `cpu=100%` before idle counters accumulated, settling immediately to `0%`.)*

**With USOS (`baseline_sensors`):**
```text
I (5402) USOS_OBS: tasks=6 cpu=0.4% heap=296240B(min=296240B) | ram=7411B q=0/0/16 drops=0 hwm=2540B
App readings -> T=26.5C P=1013.2hPa D=45.0cm L=2048
```

### Runtime Metrics Summary

| Metric | Conventional (No USOS) | With USOS (Stage 3) | Delta / Notes |
|---|---:|---:|---|
| **FreeRTOS Tasks** | 5 | 6 | **+1** (`usos_task`) |
| **Free Heap at Boot / Steady State** | 302,784 B | 296,240 B | **-6,544 B** (exact match to static DRAM delta; zero heap leaks) |
| **Minimum Free Heap** | 302,784 B | 296,240 B | **0 B** runtime heap drift |
| **USOS-Reported Static RAM** | — | 7,411 B | **0 B** discrepancy vs. `libUSOS.a` map (`7,403 B .bss + 8 B .data`) |
| **CPU Utilization** | 0% *(integer truncated)* | **0.4%** *(0.1% resolution)* | Sub-1% overhead across dual-core ESP32 |
| **Queue Peak / Capacity** | — | `0/0/16` (interval / all-time / cap) | **0** dropped events (`drops=0`) |
| **Supervisor Stack HWM (Free)** | — | **2,540 B** / 4,096 B | **1,556 B** peak used (62.0% safety margin) |

---

## 3. Key Takeaways

1. **Exact Accounting:** The `-6,544 B` heap delta matches the `+6,544 B` static DRAM delta to the single byte, and USOS's self-reported `ram=7411B` matches `libUSOS.a` `.bss + .data` (`7,411 B`) to the single byte.
2. **Low Overhead:** Stage 3 Observation Engine adds **~4.24 KB Flash**, **~6.39 KB net DRAM**, **0.4% CPU**, and **0 dropped events**, leaving **296,240 B** free heap and **62% stack headroom** on `usos_task`.
