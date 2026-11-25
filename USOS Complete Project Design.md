# **Universal Sensor Operating System (USOS)**

### **Lightweight Embedded Reliability Middleware for Adaptive Sensor, Hardware and Runtime Management**

---

#### **1\. Core Idea**

USOS is a **lightweight embedded reliability middleware and monitoring framework** designed to operate underneath an existing embedded application.

The primary objective is not to replace the application's programming model, sensor APIs, or application logic.

Instead, USOS should provide a reliability layer that continuously observes and manages the relationship between:

Application  
     ↓  
Runtime  
     ↓  
Hardware  
     ↓  
Sensors

USOS should detect abnormal behaviour, assess system health, adapt resource usage, and attempt recovery wherever possible while maintaining a very small CPU and RAM footprint.

The central design principle is:

> **USOS should improve the reliability and observability of an existing embedded system without requiring developers to rewrite their application.**

---

##### **1.1 Developer Integration Philosophy**

The intended developer workflow is deliberately minimal.

A developer should only need to add:

\#include \<usos.h\>

USOS\_ADAPTER(  
    /\* system configuration \*/  
);

The exact syntax and configuration structure will be finalized during Phase 4-1 Stage 1\.

The developer should **not** need to add USOS API calls throughout the application.

The following APIs should therefore not be required during normal application development:

USOS\_Init()  
USOS\_ReadSensor()  
USOS\_GetHealth()  
USOS\_Update()  
USOS\_RegisterEvent()  
USOS\_SetSamplingRate()  
...

Existing application logic should remain unchanged wherever technically possible.

For example:

\#include \<usos.h\>

USOS\_ADAPTER(...);

int main()  
{  
    // Existing application code

    DHT\_Read(\&temperature);  
    BMP280\_ReadPressure(\&pressure);

    // Existing control logic

    while (1)  
    {  
        // Existing application  
    }  
}

USOS should operate transparently through its integration/adapter layer.

---

##### **1.2 Core Responsibilities**

USOS will focus on five fundamental operations:

                USOS  
                  │  
       ┌──────────┼──────────┐  
       ↓          ↓          ↓  
    OBSERVE     ANALYZE     ADAPT  
       │          │          │  
       └──────────┼──────────┘  
                  ↓  
                HEAL  
                  │  
                  ↓  
              REPORT

### **Observe**

Monitor:

* Sensors  
* Drivers  
* Hardware interfaces  
* FreeRTOS tasks  
* CPU utilization  
* RAM/heap usage  
* Stack usage  
* Timing behaviour  
* Communication failures  
* Sensor readings  
* Runtime errors

### **Analyze**

Determine:

* Sensor health  
* Hardware health  
* Runtime health  
* Abnormal readings  
* Communication failures  
* Resource pressure  
* Sampling requirements  
* Fault severity

### **Adapt**

Dynamically adjust where appropriate:

* Sensor sampling frequency  
* Monitoring frequency  
* Diagnostic frequency  
* Retry frequency  
* Resource usage  
* Monitoring priorities

### **Heal**

Attempt automatic recovery through mechanisms such as:

* Communication retry  
* Peripheral reinitialization  
* Sensor reset  
* Driver restart  
* Task restart  
* Fault isolation  
* Safe fallback behaviour

### **Report**

Expose system information through a monitoring interface and dashboard.

---

##### **1.3 Scope of Intelligence**

USOS is not intended to understand arbitrary application semantics.

For example, USOS will not attempt to determine what a user's business logic means.

Instead, it monitors observable system-level properties such as:

Task execution  
CPU utilization  
Memory utilization  
Stack usage  
Timing  
Sensor communication  
Sensor values  
Driver errors  
Peripheral status  
Watchdog behaviour  
Runtime failures

This provides application-level reliability monitoring without requiring USOS to understand the internal logic of the application.

---

### **2\. Problem Statement**

Embedded applications become increasingly difficult to maintain as the number of sensors, peripherals and software components increases.

Developers commonly need to implement and maintain:

* Sensor drivers  
* Hardware communication  
* Sampling schedules  
* Error handling  
* Retry mechanisms  
* Sensor resets  
* Watchdogs  
* Runtime monitoring  
* Resource monitoring  
* Fault detection  
* Fault recovery  
* Logging  
* Remote monitoring  
* Dashboard infrastructure

These mechanisms are usually implemented separately and are often tightly coupled to the application.

As a result:

More sensors  
      ↓  
More drivers  
      ↓  
More tasks  
      ↓  
More monitoring code  
      ↓  
More failure cases  
      ↓  
More application complexity

A further problem is that embedded systems frequently use **fixed sampling and monitoring rates**, even when the system state does not require them.

For example:

Temperature sensor → sample every 1 second  
Pressure sensor    → sample every 1 second  
Health monitoring  → every 1 second  
Diagnostics        → every 1 second

Even when the system is completely stable, computation, communication and memory resources continue to be consumed.

Conversely, when the system becomes unstable, a fixed sampling strategy may not provide sufficient diagnostic information.

USOS addresses this problem by introducing a lightweight reliability middleware capable of:

> **continuously observing an embedded system, estimating its health, adapting monitoring/sampling behaviour according to system conditions, and attempting automated recovery while maintaining low resource overhead.**

---

### **3\. Literature Survey**

| Existing Work / System | Main Contribution | Limitation / Gap Relevant to USOS |
| ----- | ----- | ----- |
| **FreeRTOS** | Lightweight RTOS providing tasks, scheduling, synchronization, queues, timers and memory management. | Provides execution infrastructure but does not provide an integrated sensor/hardware reliability and self-healing layer. |
| **Zephyr RTOS** | Comprehensive embedded OS with device models, drivers, hardware abstraction and sensor APIs. | Large ecosystem; USOS focuses specifically on a lightweight reliability middleware that can sit over an existing runtime. |
| **Zephyr Sensor Subsystem** | Standardized sensor management, sampling and sensor-oriented functionality. | Primarily a sensor framework; USOS focuses on system-wide reliability across sensors, hardware, runtime and resources. |
| **Arduino Sensor Libraries** | Simplify integration of individual sensors. | Sensor-specific and generally lack system-level health management and self-healing. |
| **STM32 HAL / ESP-IDF** | Hardware abstraction and peripheral management. | Hardware-oriented rather than a unified reliability and adaptive-management framework. |
| **Embedded Watchdog Systems** | Detect certain hangs and restart failed systems. | Generally reactive and coarse-grained; limited sensor-level diagnosis and adaptive resource management. |
| **IoT Monitoring Platforms** | Provide remote telemetry, visualization and alerts. | Usually external to the embedded runtime and do not directly participate in low-level recovery or adaptive sampling. |
| **Predictive Maintenance / Anomaly Detection** | Detect abnormal behaviour using statistical or ML techniques. | Often focused on specific applications and may impose excessive computation for constrained MCUs. |

### **Research / Engineering Gap**

USOS does **not** claim novelty in individual components such as:

* Sensor monitoring  
* Health scoring  
* Adaptive sampling  
* Watchdogs  
* Fault detection  
* RTOS monitoring  
* Dashboard visualization

The intended contribution is their integration into a **lightweight, transparent embedded reliability middleware** with minimal developer integration.

The project investigates:

> **Whether a small middleware layer can improve embedded-system reliability and observability while dynamically reducing unnecessary monitoring and sensing overhead.**

The primary engineering objectives are therefore:

* Low integration effort  
* Low RAM overhead  
* Low CPU overhead  
* Continuous system observability  
* Adaptive sensing/monitoring  
* Automated fault recovery  
* Unified health representation  
* Remote system visibility

---

### **4\. Proposed Solution**

#### **4.1 USOS Middleware Architecture**

USOS will be implemented as a middleware/library layer rather than as a completely independent operating system.

The initial execution environment will use **FreeRTOS**.

Therefore:

Application  
      │  
      ▼  
     USOS  
      │  
      ▼  
   FreeRTOS  
      │  
      ▼  
    HAL  
      │  
      ▼  
   Hardware

USOS does not attempt to replace FreeRTOS.

Instead, FreeRTOS provides:

* Task execution  
* Scheduling  
* Synchronization  
* Timers  
* Queues  
* Memory primitives  
* Interrupt integration

USOS provides the reliability-oriented intelligence above it.

---

#### **4.2 USOS Integration / Adapter Layer**

The adapter is the primary developer-facing integration mechanism.

Its purpose is to describe the system to USOS and establish the connection between USOS and existing drivers/hardware.

Conceptually:

\#include \<usos.h\>

USOS\_ADAPTER(  
    ...  
);

The adapter may describe:

Board / MCU  
Sensors  
Existing drivers  
Communication interfaces  
Pins  
Sampling capabilities  
Reset/reinitialization mechanisms  
Optional system resources

The developer should not have to manually implement USOS monitoring, health calculation, recovery logic or dashboard communication.

### **Design principle**

> **The adapter describes the system; USOS manages the system.**

The adapter must therefore remain declarative rather than becoming a second programming interface.

---

#### **4.3 Transparent Application Integration**

The fundamental requirement is that existing application logic should remain unchanged.

For example:

DHT\_Read(\&temperature);  
BMP280\_ReadPressure(\&pressure);

if (temperature \> threshold)  
{  
    activate\_fan();  
}

should remain application code.

USOS should obtain the information required for monitoring through its integration mechanism rather than requiring the developer to replace every sensor operation with a USOS API.

### **Integration mechanisms**

The implementation may use a combination of:

* Driver wrappers  
* Compile-time instrumentation  
* Function interception  
* Generated adapter code  
* Weak/linker-level hooks where supported  
* Existing driver callbacks/hooks  
* Runtime observation

The exact mechanism will be selected according to the target platform and driver architecture during Phase 4-1 Stage 1\.

### **Important constraint**

USOS must not promise universal transparent interception of arbitrary proprietary software.

Instead:

> **The USOS reference implementation will guarantee transparent integration for supported driver/build patterns.**

Unsupported drivers may require an adapter-specific integration mechanism, but this must remain within the single `USOS_ADAPTER` configuration boundary.

---

#### **4.4 System Observation Layer**

The Observation Layer is responsible for collecting lightweight runtime information.

It will monitor four major domains.

### **A. Sensor Observation**

* Sensor readings  
* Sampling frequency  
* Communication success  
* Communication latency  
* Missing samples  
* Invalid values  
* Range violations  
* Repeated failures

### **B. Hardware Observation**

* GPIO/peripheral status  
* I²C/SPI/UART errors  
* Peripheral availability  
* Communication failures  
* Reset/reinitialization status

### **C. Runtime Observation**

For FreeRTOS:

* Task state  
* Task execution time  
* CPU utilization  
* Stack usage  
* Heap usage  
* Task failures  
* Timing violations  
* Watchdog events

### **D. USOS Resource Observation**

* USOS RAM usage  
* USOS CPU overhead  
* Internal queues/buffers  
* Monitoring frequency  
* Number of active monitored components

---

#### **4.5 System Health Engine**

The Health Engine converts observed information into health states.

A basic state model:

            ┌──────────┐  
             │ HEALTHY  │  
             └────┬─────┘  
                  │  
             abnormality  
                  ▼  
             ┌──────────┐  
             │ DEGRADED │  
             └────┬─────┘  
                  │  
              fault  
                  ▼  
             ┌──────────┐  
             │  FAULT   │  
             └────┬─────┘  
                  │  
              recovery  
                  ▼  
             ┌──────────┐  
             │ RECOVERY │  
             └────┬─────┘  
                  │  
              success  
                  ▼  
             ┌──────────┐  
             │ HEALTHY  │  
             └──────────┘

Health estimation may use:

* Communication success rate  
* Sensor error count  
* Response latency  
* Missing samples  
* Value validity  
* Runtime errors  
* Resource pressure  
* Recovery history

A health score may be represented as:

0 – 100

but the exact scoring algorithm should remain configurable.

---

#### **4.6 Fault Detection Engine**

The Fault Detection Engine identifies abnormal system behaviour.

Initial fault classes:

Sensor communication failure  
Invalid sensor value  
Sensor timeout  
Repeated driver failure  
Peripheral communication failure  
Task failure  
Task starvation  
Stack overflow/near overflow  
Memory pressure  
Watchdog event  
Sampling instability

The initial implementation should use lightweight deterministic/statistical techniques.

Machine learning is not required for the core system.

---

#### **4.7 Recovery / Self-Healing Engine**

When a fault is detected, USOS should attempt an appropriate recovery action.

Example:

Sensor failure  
      ↓  
Fault Detection  
      ↓  
Classify fault  
      ↓  
Retry communication  
      ↓  
If failure persists  
      ↓  
Reinitialize driver  
      ↓  
If failure persists  
      ↓  
Reset sensor/peripheral  
      ↓  
Verify operation  
      ↓  
Healthy → Resume  
      │  
      └── Failure → Isolate / Report

Possible recovery actions:

* Retry  
* Reinitialize  
* Peripheral reset  
* Driver restart  
* Task restart  
* Sensor isolation  
* Fallback mode  
* Safe-state notification

Recovery actions must be bounded and must not create infinite recovery loops.

---

#### **4.8 Adaptive Sampling and Monitoring**

Adaptive sampling is one of the primary intelligence components of USOS.

Instead of maintaining fixed sampling rates indefinitely, USOS can modify sensing or monitoring frequency according to system conditions.

Example:

Stable  
  ↓  
Low sampling / monitoring frequency

Rapid change  
  ↓  
Increase sampling frequency

Abnormal behaviour  
  ↓  
High-frequency observation

Fault suspected  
  ↓  
Diagnostic sampling

Recovered  
  ↓  
Gradually return to low-power state

The adaptive policy may consider:

* Rate of change  
* Sensor importance  
* Health state  
* Data freshness  
* Application requirements  
* Resource availability  
* Recent fault history

The objective is:

> **Maintain required observability and reliability while minimizing unnecessary CPU, RAM, communication and sensing operations.**

---

#### **4.9 Resource-Aware Operation**

USOS itself must remain lightweight.

The framework should therefore monitor and optimize its own overhead.

Design principles:

* Static allocation where practical  
* Fixed-size buffers  
* Bounded queues  
* Minimal dynamic allocation  
* Event-driven processing  
* Configurable monitoring rates  
* Avoid unnecessary polling  
* Lightweight health calculations  
* Selective telemetry  
* Batch transmission where appropriate

USOS should report its own:

RAM usage  
CPU overhead  
Telemetry overhead  
Number of active monitoring operations

This allows the project's central trade-off to be experimentally evaluated.

---

#### **4.10 Sensor and Hardware Management**

USOS will maintain a lightweight internal registry containing information such as:

Sensor ID  
Sensor type  
Driver  
Interface  
Current state  
Sampling rate  
Health  
Last successful operation  
Fault count  
Recovery count

The registry is primarily for internal management and observability.

It should not require the application developer to interact with it directly.

---

#### **4.11 Application Runtime Monitoring**

USOS will monitor application runtime behaviour without requiring changes to application logic.

For the FreeRTOS implementation this may include:

Task status  
CPU utilization  
Stack usage  
Heap usage  
Task execution timing  
Watchdog status  
Runtime errors

The objective is not to understand application semantics but to identify system-level conditions that may indicate degraded reliability.

---

#### **4.12 Dashboard and Telemetry**

USOS will provide a telemetry interface connecting the embedded device to an external dashboard.

Conceptually:

ESP32  
  │  
  │ USOS telemetry  
  ▼  
MQTT / Communication Layer  
  │  
  ▼  
Backend  
  │  
  ▼  
Database  
  │  
  ▼  
React Dashboard

The dashboard should provide:

### **System Overview**

* Overall health  
* Device status  
* CPU usage  
* RAM usage  
* Active faults  
* Recovery status

### **Sensor Monitoring**

* Live values  
* Sensor status  
* Health score  
* Sampling rate  
* Communication quality  
* Fault history

### **Runtime Monitoring**

* Task status  
* CPU utilization  
* Stack usage  
* Heap usage  
* Runtime events

### **Reliability**

* Fault events  
* Recovery attempts  
* Recovery success/failure  
* Repeated faults  
* Degraded components

### **Adaptive Behaviour**

* Current sampling rate  
* Historical sampling rate  
* Reason for adaptation  
* Resource savings

### **Historical Analytics**

* Sensor trends  
* Health trends  
* Fault frequency  
* Recovery trends  
* Resource usage

---

#### **4.13 Optional Advanced Intelligence**

The following are explicitly **optional** and should not define the core project:

* Predictive failure detection  
* TinyML anomaly detection  
* Sensor fusion  
* Advanced predictive maintenance  
* Complex sensor discovery

They may be added only if the core reliability system is stable.

The project should remain successful even if these features are not implemented.

---

### **5\. Architecture**

#### **5.1 Overall Architecture**

┌───────────────────────────────────────────────────────┐  
│                  EXISTING APPLICATION                 │  
│                                                       │  
│       Existing application logic remains unchanged    │  
└───────────────────────────┬───────────────────────────┘  
                            │  
                            │ Normal execution  
                            ▼  
┌───────────────────────────────────────────────────────┐  
│                         USOS                          │  
│                                                       │  
│ ┌───────────────────────────────────────────────────┐ │  
│ │             Integration / Adapter Layer           │ │  
│ │                                                   │ │  
│ │ Board • Sensors • Drivers • Interfaces • Hooks    │ │  
│ └───────────────────────┬───────────────────────────┘ │  
│                         ▼                             │  
│ ┌───────────────────────────────────────────────────┐ │  
│ │              Observation Layer                    │ │  
│ │                                                   │ │  
│ │ Sensor Monitor │ Hardware Monitor │ Runtime      │ │  
│ │ Resource Monitor │ Timing Monitor                 │ │  
│ └───────────────────────┬───────────────────────────┘ │  
│                         ▼                             │  
│ ┌───────────────────────────────────────────────────┐ │  
│ │               Reliability Engine                  │ │  
│ │                                                   │ │  
│ │ Health │ Fault Detection │ Diagnostics            │ │  
│ └───────────────────────┬───────────────────────────┘ │  
│                         ▼                             │  
│ ┌───────────────────────────────────────────────────┐ │  
│ │                 Adaptive Engine                   │ │  
│ │                                                   │ │  
│ │ Adaptive Sampling │ Monitoring │ Resource Policy  │ │  
│ └───────────────────────┬───────────────────────────┘ │  
│                         ▼                             │  
│ ┌───────────────────────────────────────────────────┐ │  
│ │                 Recovery Engine                   │ │  
│ │                                                   │ │  
│ │ Retry │ Reset │ Reinitialize │ Restart │ Isolate │ │  
│ └───────────────────────┬───────────────────────────┘ │  
│                         ▼                             │  
│ ┌───────────────────────────────────────────────────┐ │  
│ │                  Telemetry Layer                  │ │  
│ └───────────────────────┬───────────────────────────┘ │  
└─────────────────────────┼─────────────────────────────┘  
                          │  
                          ▼  
                  ┌───────────────┐  
                  │   FreeRTOS    │  
                  │               │  
                  │ Tasks         │  
                  │ Scheduler     │  
                  │ Timers        │  
                  │ Queues        │  
                  │ Synchronize   │  
                  └───────┬───────┘  
                          │  
                          ▼  
                  ┌───────────────┐  
                  │     HAL       │  
                  └───────┬───────┘  
                          │  
                          ▼  
              ┌───────────────────────┐  
              │       Hardware        │  
              │                       │  
              │ MCU │ Sensors │ Bus   │  
              └───────────────────────┘

---

#### **5.2 External Monitoring Architecture**

┌───────────────┐  
│    ESP32      │  
│               │  
│ Application   │  
│     \+         │  
│    USOS       │  
└───────┬───────┘  
        │  
       MQTT  
        │  
        ▼  
┌──────────────────┐  
│  FastAPI Backend │  
└────────┬─────────┘  
         │  
         ▼  
┌──────────────────┐  
│    PostgreSQL    │  
└────────┬─────────┘  
         │  
         ▼  
┌──────────────────┐  
│ React \+ Vite     │  
│ Dashboard        │  
└──────────────────┘

---

#### **5.3 Internal Data Flow**

Sensor / Runtime Event  
          │  
          ▼  
     Observation  
          │  
          ▼  
    Data Validation  
          │  
          ▼  
   Health Assessment  
          │  
      ┌───┴────┐  
      │        │  
    Normal   Abnormal  
      │        │  
      ▼        ▼  
 Continue   Fault Engine  
                │  
          ┌─────┴─────┐  
          ▼           ▼  
       Recover       Adapt  
          │           │  
          └─────┬─────┘  
                ▼  
             Telemetry  
                │  
                ▼  
            Dashboard

---

#### **5.4 Core Module Boundaries**

USOS  
│  
├── integration/  
│   ├── adapter  
│   ├── driver hooks  
│   └── platform configuration  
│  
├── observation/  
│   ├── sensor monitor  
│   ├── hardware monitor  
│   ├── runtime monitor  
│   └── resource monitor  
│  
├── reliability/  
│   ├── health engine  
│   ├── fault detector  
│   └── diagnostics  
│  
├── adaptive/  
│   ├── sampling policy  
│   ├── monitoring policy  
│   └── resource policy  
│  
├── recovery/  
│   ├── retry  
│   ├── reset  
│   ├── reinitialize  
│   └── isolation  
│  
├── telemetry/  
│   ├── event collection  
│   ├── serialization  
│   └── communication  
│  
└── platform/  
    ├── FreeRTOS  
    └── HAL

---

### **6\. Development Plan**

#### **Phase 4-1 — USOS Core Reliability Middleware**

##### **Stage 1 — Architecture and Specification**

Finalize:

* USOS positioning  
* Integration philosophy  
* `USOS_ADAPTER` specification  
* Supported driver integration model  
* Observation model  
* Health states  
* Fault model  
* Recovery model  
* Adaptive sampling model  
* Runtime monitoring model  
* Telemetry model  
* Dashboard data contract  
* Memory/CPU constraints  
* FreeRTOS integration  
* HAL boundaries

### **Stage 1 Deliverables**

* System architecture  
* Low-level architecture  
* Adapter specification  
* Sensor metadata specification  
* Health model  
* Fault model  
* Recovery state machine  
* Event structure  
* Telemetry structure  
* Resource budget  
* API/internal interface specification

---

##### **Stage 2 — USOS Integration Layer**

Implement:

* `usos.h`  
* Adapter mechanism  
* Platform detection  
* Sensor/driver registration  
* Hardware configuration  
* Integration hooks  
* FreeRTOS integration hooks

Primary objective:

> An existing application should be able to integrate USOS through the defined minimal workflow without rewriting application logic.

---

##### **Stage 3 — Observation Engine**

Implement:

### **Sensor monitoring**

* Reading observation  
* Communication status  
* Error counting  
* Timeout detection  
* Sampling statistics

### **Runtime monitoring**

* CPU utilization  
* Task states  
* Stack usage  
* Heap usage  
* Timing information

### **Hardware monitoring**

* Peripheral errors  
* Communication failures  
* Reset status

---

##### **Stage 4 — Health and Fault Engine**

Implement:

* Health states  
* Health score  
* Fault classification  
* Threshold-based detection  
* Communication failure detection  
* Invalid-value detection  
* Runtime fault detection  
* Fault history

---

##### **Stage 5 — Recovery Engine**

Implement:

* Retry  
* Driver reinitialization  
* Sensor reset  
* Peripheral reinitialization  
* Task restart where safe  
* Fault isolation  
* Recovery verification  
* Recovery limits

Demonstrate:

Fault  
 ↓  
Detection  
 ↓  
Recovery  
 ↓  
Verification  
 ↓  
Resume / Report

---

##### **Stage 6 — Adaptive Engine**

Implement:

* Adaptive sensor sampling  
* Adaptive monitoring frequency  
* Health-dependent monitoring  
* Resource-aware sampling  
* Event-triggered diagnostics

Example:

Stable → Low monitoring

Changing → Increased monitoring

Anomaly → High monitoring

Fault → Diagnostic mode

Recovered → Gradual reduction

---

##### **Stage 7 — Basic Telemetry and Dashboard**

Implement:

* Device registration  
* Live sensor values  
* Health state  
* Health score  
* CPU  
* RAM  
* Task information  
* Fault events  
* Recovery events  
* Sampling frequency

---

#### **Phase 4-2 — Intelligent USOS**

##### **Stage 1 — Advanced Adaptive Management**

Improve:

* Sampling policies  
* Resource-aware adaptation  
* Sensor prioritization  
* Monitoring scheduling  
* Adaptive diagnostic frequency

---

##### **Stage 2 — Advanced Reliability**

Implement selectively:

* Sensor degradation detection  
* Failure trend analysis  
* Recovery history  
* Component reliability scoring  
* Cross-component fault correlation

---

##### **Stage 3 — Advanced Monitoring**

Add:

* Historical health analysis  
* Runtime trend analysis  
* Sensor correlation  
* Fault clustering  
* Long-term reliability metrics

---

##### **Stage 4 — Optional Intelligence**

Only if core USOS is stable:

* TinyML anomaly detection  
* Predictive failure detection  
* Sensor fusion  
* Predictive maintenance

These features are optional and are not required for the core project to be considered complete.

---

##### **Stage 5 — Advanced Dashboard**

Dashboard should display:

### **System**

* Overall health  
* CPU  
* RAM  
* Runtime status

### **Sensors**

* Current values  
* Health  
* Sampling rates  
* Communication quality

### **Reliability**

* Active faults  
* Fault history  
* Recovery history  
* Recovery success rate

### **Adaptive Behaviour**

* Current sampling rate  
* Sampling history  
* Adaptation reason  
* Estimated resource savings

### **Analytics**

* Health trends  
* Fault trends  
* Resource trends  
* Sensor reliability

---

### **Stage 6 — Experimental Evaluation**

Compare:

### **Conventional System**

Application  
    │  
    ├── Sensor drivers  
    ├── Fixed sampling  
    ├── Manual monitoring  
    └── Manual recovery

against:

### **USOS**

Application  
    │  
    ▼  
   USOS  
    │  
    ├── Observation  
    ├── Health assessment  
    ├── Adaptive sampling  
    ├── Fault detection  
    ├── Self-healing  
    └── Telemetry

Evaluate:

### **Integration**

* Lines of application code changed  
* Number of integration steps  
* Sensor addition effort  
* Sensor replacement effort

### **Resource overhead**

* RAM  
* Flash  
* CPU  
* Task count  
* Communication overhead

### **Reliability**

* Fault detection time  
* Fault detection accuracy  
* Recovery time  
* Recovery success rate  
* False positive rate  
* Fault recurrence

### **Adaptation**

* Sampling reduction  
* Monitoring reduction  
* CPU savings  
* Communication savings  
* Energy consumption where measurable

### **Scalability**

* Number of sensors  
* Number of tasks  
* Number of monitored components  
* RAM/CPU growth

---

### **7\. Tools and Hardware**

| Category | Tool / Hardware | Purpose |
| ----- | ----- | ----- |
| MCU | **ESP32 development board** | Primary target |
| RTOS | **FreeRTOS** | Execution foundation |
| Temperature/Humidity | DHT22 | Sensor reliability demonstration |
| Pressure/Temperature | BMP280 | I²C monitoring |
| Distance | HC-SR04 | GPIO/timing reliability |
| Light | LDR \+ resistor | ADC monitoring |
| Optional IMU | MPU6050 | High-rate adaptive sampling |
| Communication | I²C | Sensor communication |
| Communication | SPI | Extended sensor support |
| Communication | UART | Debugging/telemetry |
| Programming | C/C++ | Embedded implementation |
| Build | ESP-IDF / PlatformIO | Firmware development |
| Simulation | Wokwi | Initial validation |
| Version Control | Git \+ GitHub | Source control |
| Telemetry | MQTT | Device communication |
| Backend | FastAPI | Dashboard backend |
| Database | PostgreSQL | Telemetry/history |
| Frontend | React \+ Vite | Dashboard |
| Visualization | Recharts / Chart.js | Data visualization |
| Testing | Unity/CMock or equivalent | Unit testing |
| Analysis | Python | Benchmarking |
| Optional ML | scikit-learn / TensorFlow Lite Micro | Advanced intelligence |
| Documentation | Markdown | Technical documentation |

---

### **8\. Advantages, Disadvantages and Limitations**

#### **8.1 Advantages**

### **1\. Minimal application modification**

The intended workflow is:

\#include \<usos.h\>

USOS\_ADAPTER(...);

rather than requiring developers to replace their existing application APIs.

---

### **2\. Centralized reliability**

Reliability logic does not need to be independently implemented for every sensor.

Sensors  
  ↓  
USOS  
  ↓  
Common reliability mechanisms

---

### **3\. Low resource overhead**

USOS is specifically designed for constrained MCUs.

The project can quantitatively demonstrate:

Reliability improvement  
        vs  
Resource overhead

---

### **4\. Adaptive resource usage**

USOS does not have to monitor every component at maximum frequency continuously.

Monitoring intensity can respond to system conditions.

---

### **5\. Hardware and sensor observability**

The dashboard provides a unified view of:

Sensors  
Hardware  
Runtime  
Resources  
Faults  
Recovery

---

### **6\. Automated recovery**

Common recoverable failures can be handled without requiring application code to implement every recovery path.

---

### **7\. Existing applications can be retained**

The architecture prioritizes compatibility with existing application logic instead of forcing developers into a completely new programming model.

---

### **8\. Clear experimental evaluation**

USOS can be quantitatively compared with conventional embedded implementations.

---

#### **8.2 Disadvantages**

### **1\. Transparent integration is technically difficult**

USOS cannot universally intercept arbitrary C/C++ functions without some knowledge of the build environment and driver structure.

### **Solution**

Define supported integration mechanisms for the reference platform and use:

* Driver wrappers  
* Compile-time hooks  
* Generated adapters  
* Linker/compiler mechanisms where appropriate  
* Existing driver callbacks

The user-facing workflow remains the same even if the internal mechanism differs by platform.

---

### **2\. USOS cannot understand arbitrary application logic**

USOS cannot determine whether:

if (temperature \> 50\)

is logically correct for a particular application.

### **Solution**

Monitor observable runtime properties rather than application semantics.

USOS should explicitly define its application monitoring boundary.

---

### **3\. Not every hardware fault can be recovered**

A physically damaged sensor cannot necessarily be repaired by software.

### **Solution**

USOS distinguishes:

Recoverable fault  
        vs  
Non-recoverable fault

For non-recoverable faults it should:

* Detect  
* Isolate where possible  
* Report  
* Continue safely if possible

---

### **4\. Adaptive sampling can conflict with application requirements**

Some applications require strict fixed sampling rates.

### **Solution**

Introduce sampling constraints in the adapter configuration:

minimum rate  
maximum rate  
required rate  
adaptive allowed/not allowed

USOS may adapt only within safe boundaries.

---

### **5\. Monitoring itself consumes resources**

A reliability system can become a resource burden.

### **Solution**

USOS must treat monitoring overhead as a first-class design constraint.

Use:

* Event-driven monitoring  
* Adaptive monitoring  
* Fixed-size buffers  
* Lightweight statistics  
* Selective telemetry  
* Configurable monitoring levels

---

### **6\. Health scores can be misleading**

A single numerical score cannot completely represent system reliability.

### **Solution**

The dashboard should expose both:

Overall health score  
\+  
Underlying health factors

For example:

System Health: 82/100

Sensor communication: 95  
Runtime health: 88  
Memory health: 91  
Temperature sensor: 62

---

### **7\. Dashboard connectivity may not always be available**

An embedded system cannot depend on the cloud/dashboard for basic operation.

### **Solution**

USOS must operate autonomously.

Dashboard available  
     ↓  
Telemetry \+ remote monitoring

Dashboard unavailable  
     ↓  
USOS continues local monitoring/recovery

---

### **8\. Self-healing can cause recovery loops**

Repeated resets may make a faulty system less stable.

### **Solution**

Use bounded recovery:

Failure  
 ↓  
Retry  
 ↓  
Reset  
 ↓  
Reinitialize  
 ↓  
Retry limit reached  
 ↓  
FAULT / ISOLATE

Every recovery action should have:

* Retry limit  
* Cooldown  
* Escalation level  
* Recovery history

---

### **9\. Platform dependency**

Some runtime and hardware-monitoring capabilities depend on FreeRTOS and MCU architecture.

### **Solution**

Keep USOS internally divided into:

USOS Core  
    │  
Platform Adapter  
    │  
FreeRTOS / MCU / HAL

The core reliability logic should remain platform-independent wherever practical.

---

#### **8.3 Explicit Non-Goals**

To prevent scope explosion, USOS will **not** initially attempt to provide:

* A replacement for FreeRTOS  
* A completely new programming language  
* Automatic rewriting of arbitrary source code  
* Universal support for every sensor  
* Guaranteed recovery from physical hardware damage  
* Full semantic understanding of arbitrary application code  
* Mandatory ML/TinyML  
* Mandatory sensor fusion  
* Mandatory cloud connectivity  
* A complex universal sensor API exposed to application developers

The core project is complete without these features.

---

### **9\. Final Project Definition**

## **One-line definition**

> **USOS is a lightweight embedded reliability middleware that transparently monitors sensors, hardware, runtime and resources, dynamically adapts sensing and monitoring behaviour, automatically recovers from supported faults, and provides unified system health visibility with minimal application integration.**

---

## **Core Philosophy**

                EXISTING APPLICATION  
                         │  
                  No rewrite required  
                         │  
                         ▼  
                       USOS  
                         │  
       ┌─────────────────┼─────────────────┐  
       │                 │                 │  
       ▼                 ▼                 ▼  
    OBSERVE           ANALYZE            ADAPT  
       │                 │                 │  
       └─────────────────┼─────────────────┘  
                         ▼  
                       HEAL  
                         │  
                         ▼  
                      REPORT  
                         │  
                         ▼  
                    DASHBOARD

---

## **Core USOS Loop**

       ┌─────────────────────┐  
        │      OBSERVE        │  
        └──────────┬──────────┘  
                   ▼  
        ┌─────────────────────┐  
        │       ASSESS        │  
        │  Health / Fault     │  
        └──────────┬──────────┘  
                   ▼  
        ┌─────────────────────┐  
        │       DECIDE        │  
        │ Adapt / Recover     │  
        └──────────┬──────────┘  
                   ▼  
        ┌─────────────────────┐  
        │       ACT           │  
        │ Sample / Recover    │  
        └──────────┬──────────┘  
                   ▼  
        ┌─────────────────────┐  
        │       REPORT        │  
        └──────────┬──────────┘  
                   │  
                   └──────────────► OBSERVE

---

## **Final Positioning**

USOS should be presented as:

> **A lightweight reliability middleware for embedded systems — not another RTOS, not merely another sensor API, and not merely a dashboard.**

Its distinguishing combination is:

Minimal Integration  
        \+  
System-wide Observation  
        \+  
Reliability Assessment  
        \+  
Adaptive Sampling / Monitoring  
        \+  
Automated Recovery  
        \+  
Low Resource Overhead  
        \+  
Complete System Dashboard

The central engineering question is:

> **Can an embedded reliability layer provide continuous system health management and adaptive behaviour with sufficiently low overhead that it is practical for resource-constrained microcontrollers?**

That question provides the project with a clear implementation scope, measurable outcomes, and a defensible engineering/research direction.

