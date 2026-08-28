# Teensy 4.1 Quad H-Bridge Actuator Control Firmware

A high-performance embedded control firmware for an 8-layer custom PCBA, designed to drive and monitor four independent linear actuators with closed-loop Hall-effect feedback, power hold-up protection, and CAN bus integration.

---

## Project Outline

1. **Hardware Integration:** Interface the Teensy 4.1 with onboard IFX007T half-bridge drivers, MAX33040E CAN transceiver, Hall-effect sensors, and the power-loss detection circuit.
2. **Actuator Motion & Feedback:** Implement multi-channel H-bridge control routines with interrupt-driven Hall sensor pulse tracking for precise positioning.
3. **Failsafe & Power Retention:** Monitor supply rail degradation via hardware interrupt to trigger non-volatile EEPROM position logging during power cutoffs.

---

## Hardware Architecture

### Components
* **Microcontroller:** Teensy 4.1
* **Motor Drivers:** Infineon IFX007T Half-Bridges (Quad H-Bridge configuration)
* **CAN Transceiver:** MAX33040E Control Area Network Transceiver
* **Power Regulation:** LMR50410-Q1 Buck Converter
* **Feedback:** Dual Hall-Effect Sensors per actuator channel
* **Power Hold-up Circuit:** Onboard capacitor bank for emergency EEPROM retention

### Pinout Mapping

| Subsystem | Signal Name | Teensy 4.1 Pin | Description |
| :--- | :--- | :--- | :--- |
| **Power Hold-up** | `VS_PIN` | Pin 24 | Falling-edge power failure detection interrupt |
| **CAN Bus** | `CAN_TX` / `CAN_RX` | Pin 31 / Pin 30 | CAN interface communication lines |
| | `SHDN` / `STBY` | Pin 28 / Pin 29 | Transceiver shutdown and standby control |
| **H-Bridge A** | `IN1_A` / `IN2_A` / `INH_A` | Pin 3 / Pin 15 / Pin 1 | Direction inputs and bridge inhibit/enable |
| **H-Bridge B** | `IN1_B` / `IN2_B` / `INH_B` | Pin 36 / Pin 18 / Pin 16 | Direction inputs and bridge inhibit/enable |
| **H-Bridge C** | `IN1_C` / `IN2_C` / `INH_C` | Pin 14 / Pin 2 / Pin 0 | Direction inputs and bridge inhibit/enable |
| **H-Bridge D** | `IN1_D` / `IN2_D` / `INH_D` | Pin 13 / Pin 37 / Pin 17 | Direction inputs and bridge inhibit/enable |
| **Hall Sensors** | `HALL1_[A-D]` | Pins 26, 35, 27, 38 | Channel 1 primary interrupt pulse inputs |
| | `HALL2_[A-D]` | Pins 25, 33, 32, 34 | Channel 2 secondary interrupt pulse inputs |
| **Current Sense** | `IS1_[A-D]` | Pins A9, A15, A6, A5 | Low-side / bridge current feedback line 1 |
| | `IS2_[A-D]` | Pins A7, A17, A8, A16 | Low-side / bridge current feedback line 2 |

---

## Software & Control Implementation

* **H-Bridge Drive Logic:** State-managed discrete digital outputs drive complementary high/low signals to control directional polarity (Extend, Retract, Stop) across four independent actuator channels (`A`, `B`, `C`, `D`).
* **Interrupt-Driven Pulse Counting:** Configured `RISING` edge external interrupts on Hall feedback pins track relative displacement in real-time, incrementing or decrementing position counters based on active directional state.
* **Emergency State Preservation:** Monitors `VS_PIN` via hardware interrupt to detect falling edge voltage drops, flagging immediate high-priority EEPROM buffer saves before onboard holdup capacitance discharges.

---

## Testing and Validation

* **Actuator Extension & Retraction:** Validated bidirectional driving logic across all four channels. Full travel stroke transitions operate reliably without shoot-through or state contention.
* **Hall Sensor Interrupt Verification:** Verified rising-edge interrupt triggers on all Hall channels. Position register increments during extension and decrements during retraction with zero missed counts during standard duty cycles.
* **Homing Sequence:** Confirmed mechanical hard-stop homing routine at startup, resetting internal relative coordinate registers to `0` upon baseline calibration.
* **Under Development:**
  * CAN bus frame transmission and remote command decoding (`ACAN_T4`).
  * Real-time current monitoring conversion on analog pins (`IS1_[A-D]`, `IS2_[A-D]`) for overcurrent and stall detection.
  * Measured benchmarking of EEPROM multi-byte write latency within the <5 ms power hold-up window.
