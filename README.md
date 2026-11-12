# AVR Digital Clock

A feature-rich digital clock and time-management system built with the **ATmega32** microcontroller using **Embedded C**. This project was developed as part of the **Embedded Systems Based on AVR Diploma at IMT School**.

The system combines an RTC-based clock and calendar with an interactive LCD/keypad interface, alarm, stopwatch, countdown timer, and a melody-based buzzer notification system.

A major focus of the implementation was developing the software stack from the ground up: **the project’s LIB, MCAL, HAL, service, and application code were written specifically for this project without relying on ready-made peripheral drivers or external code libraries.**

---

## Table of Contents

- [Project Overview](#project-overview)
- [Features](#features)
- [Hardware and Software](#hardware-and-software)
- [System Architecture](#system-architecture)
- [Project Directory](#project-directory)
- [Implementation Details](#implementation-details)
  - [RTC and Calendar](#rtc-and-calendar)
  - [LCD and Keypad Interface](#lcd-and-keypad-interface)
  - [Alarm Service](#alarm-service)
  - [Stopwatch Service](#stopwatch-service)
  - [Countdown Service](#countdown-service)
  - [Buzzer Service and Timer Coordination](#buzzer-service-and-timer-coordination)
  - [Application Flow](#application-flow)
- [Hardware Connections](#hardware-connections)
- [Configuration](#configuration)
- [Building and Running](#building-and-running)
- [Engineering Challenges](#engineering-challenges)
- [Development Approach](#development-approach)
- [Acknowledgment](#acknowledgment)

---

## Project Overview

The AVR Digital Clock is an embedded application that provides more than basic time display. It uses the **DS1307 Real-Time Clock (RTC)** to maintain time and date, while the ATmega32 handles the user interface, time-related services, and sound notifications.

The user interacts with the system through a **4×4 matrix keypad** and a character **LCD**. The application is organized into independent software layers and services to separate low-level hardware operations from application behavior.

### Main Objectives

- Build reusable low-level drivers for the ATmega32 peripherals used by the application.
- Interface external hardware modules through dedicated HAL drivers.
- Organize application functionality into independent services.
- Manage time-dependent operations using timer interrupts and periodic software tasks.
- Generate melodies and handle competing sound requests without blocking the main application.

---

## Features

### Clock and Calendar
- Real-time clock and date using the **DS1307 RTC**.
- 24-hour time format.
- Display of time and date on the LCD.
- Set-time and set-date functions through the keypad.
- Input validation before committing time/date values.
- Automatic day-of-week calculation for entered dates.

### Alarm
- Configure an alarm date and time.
- Enable and disable the alarm.
- Detect the configured alarm time/date.
- Play a selectable ringtone when the alarm is triggered.
- Stop the active alarm sound through the keypad.

### Stopwatch
- Start, stop, and reset controls.
- Periodic time updates handled by the stopwatch service.

### Countdown Timer
- Configure a countdown duration.
- Start, pause, resume, and reset/stop the countdown through the service controls.
- Detect countdown completion.
- Trigger a sound notification when the countdown finishes.

### User Interface
- Character LCD for displaying the clock, menus, and service screens.
- 4×4 matrix keypad for navigation and numeric input.
- Separate screens for clock settings, alarm, stopwatch, countdown, and ringtone selection.
- Numeric input handling with digit buffering, confirmation, backspace, and validation.

### Sound Notifications
- Buzzer output driven through the ATmega32 **OC0** hardware output.
- Multiple melodies, including Nokia, Jingle Bells, and Westminster-style tunes.
- Separate ringtone selection for alarm and countdown notifications.
- Alarm sound requests take priority over countdown sound requests.
- Note timing and melody progression are handled without blocking delays.

---

## Hardware and Software

### Hardware

| Component | Purpose |
|---|---|
| ATmega32 | Main microcontroller |
| DS1307 | Real-time clock and calendar |
| Character LCD | Clock display and user interface |
| 4×4 Matrix Keypad | Menu navigation and data entry |
| Buzzer | Audible alarm and countdown notifications |

### Software and Development

| Item | Configuration |
|---|---|
| Programming language | Embedded C |
| Target MCU | ATmega32 |
| CPU frequency | 8 MHz |
| Compiler | AVR-GCC |
| Register access | Direct memory-mapped register access |
| RTC communication | TWI / I²C |
| Timer-based sound output | Timer0, CTC mode, OC0 |
| Periodic time base | Timer1, CTC mode, Compare Match A |
| Architecture | LIB / MCAL / HAL / SERVICES / APP |

The source code defines `F_CPU` as `8000000UL` in the timer configuration.

---

## System Architecture

The project follows a layered architecture. Each layer has a defined responsibility and communicates with the layer below it through interfaces.

```text
                    +----------------------+
                    |         APP          |
                    | Menu, UI, Integration|
                    +----------------------+
                               |
                    +----------------------+
                    |       SERVICES       |
                    | Alarm / Stopwatch    |
                    | Countdown / Buzzer   |
                    +----------------------+
                               |
                    +----------------------+
                    |         HAL          |
                    | LCD / Keypad / RTC   |
                    | Buzzer               |
                    +----------------------+
                               |
                    +----------------------+
                    |         MCAL         |
                    | DIO / EXTI / GIE     |
                    | Timers / TWI         |
                    +----------------------+
                               |
                    +----------------------+
                    |         LIB          |
                    | Types / Bit Math     |
                    +----------------------+
                               |
                    +----------------------+
                    |      ATmega32        |
                    +----------------------+
```

### 1. LIB – Standard Types and Utilities

Contains common definitions used throughout the project:

- `std_types.h`: project-wide standard data type definitions such as `u8`, `u16`, and other required types.
- `bit_math.h`: bit-manipulation macros/functions used to set, clear, toggle, and read individual bits.

These utilities provide a consistent foundation for register-level programming.

### 2. MCAL – Microcontroller Abstraction Layer

Contains drivers that directly configure and control ATmega32 hardware peripherals.

| Driver | Responsibility |
|---|---|
| **DIO** | Configure pin/port direction, write pin/port values, read pin values, and toggle pin values. |
| **EXTI** | Configure and control external interrupts INT0, INT1, and INT2, with callback support. |
| **GIE** | Enable and disable the global interrupt bit. |
| **TMR** | Configure and control Timer0 and Timer1, including modes, prescalers, compare values, PWM-related functions, and callbacks. |
| **TWI** | Master/slave TWI initialization and communication operations such as START, repeated START, address transmission, data transfer, and STOP. |

### 3. HAL – Hardware Abstraction Layer

Contains drivers that provide application-friendly interfaces to external hardware modules.

| Driver | Responsibility |
|---|---|
| **LCD** | LCD initialization, command/data transmission, string writing, and cursor positioning. |
| **KPD** | 4×4 keypad initialization and pressed-key detection through matrix scanning. |
| **DS1307** | RTC initialization and time/date read and write operations, including conversion and validation. |
| **BUZZER** | Start and stop buzzer output and request a generated frequency through the timer output. |

### 4. SERVICES – Application-Oriented Services

Contains functionality built using the HAL and MCAL layers.

| Service | Responsibility |
|---|---|
| **Alarm Service** | Alarm configuration, enable/disable state, trigger/expiry handling, ringing state, and sound request control. |
| **Stopwatch Service** | Stopwatch start, stop, reset, time tracking, and state reporting. |
| **Countdown Service** | Countdown configuration, start/pause/reset/stop behavior, remaining-time tracking, completion detection, and notification request. |
| **Buzzer Service** | Melody data, note sequencing, note duration tracking, ringtone selection, sound-source arbitration, and source priority. |

### 5. APP – Application Layer

Coordinates the complete system:

- Initializes drivers and services in the required order.
- Maintains the current UI page and menu navigation.
- Reads and interprets keypad input.
- Handles numeric input and commits validated values.
- Refreshes RTC data.
- Runs the alarm, stopwatch, countdown, and buzzer service tasks.
- Updates the LCD according to the active screen.

---

## Project Directory

```text
AVR_CLOCK/
│
├── APP/
│   ├── APP_config.h
│   ├── APP_interface.h
│   ├── APP_private.h
│   └── APP_program.c
│
├── LIB/
│   ├── std_types.h
│   └── bit_math.h
│
├── MCAL/
│   ├── DIO/
│   │   ├── DIO_config.h
│   │   ├── DIO_interface.h
│   │   ├── DIO_private.h
│   │   └── DIO_program.c
│   ├── EXTI/
│   ├── GIE/
│   ├── TMR/
│   └── TWI/
│
├── HAL/
│   ├── LCD/
│   ├── KPD/
│   ├── DS1307/
│   └── BUZZER/
│
├── SERVICES/
│   ├── Alarm_Service/
│   ├── Stopwatch_Service/
│   ├── Countdown_Service/
│   └── Buzzer_Service/
│
├── docs/
│   └── AVR_Graduation_Project_.pdf
│
└── main.c
```

Each driver generally separates its public interface, configuration, private register definitions (where required), and implementation.

---

## Implementation Details

### RTC and Calendar

The DS1307 is accessed through the ATmega32 TWI peripheral using the project's custom TWI driver.

The RTC driver provides operations to:

- Initialize the DS1307.
- Read time and date.
- Write time and date.
- Handle the configured 24-hour time format.
- Convert between the RTC's BCD register representation and application values.
- Validate RTC data and report operation status.

The application calculates the day of the week for dates entered by the user (for the supported 2000–2099 year range) and writes the resulting date information to the RTC.

### LCD and Keypad Interface

The LCD driver provides functions for initialization, sending commands and data, writing strings, and setting the cursor position.

The keypad driver scans the 4×4 matrix to detect key presses. The application uses the returned key values for:

- Navigating between pages and menus.
- Entering time, date, alarm, and countdown values.
- Confirming or cancelling input.
- Editing numeric input where supported.
- Controlling the alarm, stopwatch, and countdown screens.

The application uses an input buffer and checks that the expected number of digits has been entered before parsing and validating the value.

### Alarm Service

The alarm service manages the alarm state independently of the user interface.

Its responsibilities include:

- Storing the configured alarm date and time.
- Enabling and disabling the alarm.
- Checking whether the configured alarm condition has been reached.
- Tracking trigger, expiry, and ringing states.
- Requesting the selected ringtone from the Buzzer Service.
- Releasing or stopping the sound request when the alarm is stopped.

The application provides the screens and keypad actions used to configure and control the alarm.

### Stopwatch Service

The stopwatch service provides the core stopwatch operations:

- Initialize the stopwatch.
- Start and stop time measurement.
- Reset the measured time.
- Retrieve the current elapsed time.
- Report whether the stopwatch is running.

The service is integrated into the application's periodic task flow.

### Countdown Service

The countdown service manages a user-configured duration.

It supports:

- Setting the countdown value.
- Starting and pausing the countdown.
- Resetting/stopping the countdown.
- Retrieving the remaining time.
- Detecting when the countdown has finished.
- Requesting a buzzer notification on completion.

The countdown logic is kept separate from the LCD and keypad handling.

### Buzzer Service and Timer Coordination

The buzzer system was one of the more challenging parts of the project because sound generation requires two different timing operations:

1. Generating the frequency of each musical note.
2. Measuring how long each note should play before moving to the next note.

The implementation uses **Timer0 and Timer1 for separate responsibilities**.

#### Timer0 – Note Frequency Generation

- Timer0 is configured in **CTC mode**.
- Its output compare pin, **OC0 (PB3)**, is used to generate the buzzer waveform.
- The compare value is selected according to the requested note frequency.
- The current Timer0 prescaler configuration is 64.

#### Timer1 – Melody Time Base

- Timer1 is configured in **CTC mode using OCR1A**.
- It uses a prescaler of 8.
- With an 8 MHz CPU clock, the Timer1 counter clock is 1 MHz.
- `OCR1A = 9999` produces a compare match every **10 ms**.
- The Timer1 callback informs the Buzzer Service that another 10 ms tick has elapsed.

#### Non-Blocking Melody Playback

The Timer1 interrupt does not perform the complete melody sequence. It updates a pending tick counter, while the Buzzer Service task processes those ticks in the main application flow.

This allows the service to:

- Track the remaining duration of the current note.
- Advance to the next note when its duration expires.
- Represent pauses between notes.
- Stop playback when the melody is complete.
- Keep the application loop running instead of waiting in blocking delay functions.

#### Sound Request Priority

The Buzzer Service manages requests from different system functions. The **Alarm has priority over the Countdown** when both request sound playback. Each source can select its own ringtone, and the service chooses which request should control the buzzer.

This separates sound generation from the alarm and countdown logic and centralizes sound arbitration in one service.

### Application Flow

The program entry point calls `APP_vidInit()` once, then continuously calls `APP_vidTask()` inside the main loop.

The application initialization sequence is important:

1. Initialize the TWI master before communicating with the RTC.
2. Initialize the DS1307.
3. Initialize the LCD and keypad.
4. Initialize the Buzzer Service.
5. Initialize the Alarm, Stopwatch, and Countdown services.
6. Read the initial RTC time/date values.
7. Configure Timer1 compare-match timing and register its callback.
8. Initialize Timer1 and enable its Compare Match A interrupt.
9. Enable global interrupts.

During normal operation, the application task coordinates the service tasks, RTC refresh, keypad handling, and LCD updates.

---

## Hardware Connections

The following pin assignments are taken from the project's configuration headers and ATmega32 peripheral mapping.

| Module | Signal | ATmega32 Pin |
|---|---|---|
| LCD | Data bus D0–D7 | PA0–PA7 |
| LCD | RS | PB0 |
| LCD | RW | PB1 |
| LCD | E | PB2 |
| Keypad | Column 0 | PD3 |
| Keypad | Column 1 | PD2 |
| Keypad | Column 2 | PD1 |
| Keypad | Column 3 | PD0 |
| Keypad | Row 0 | PD4 |
| Keypad | Row 1 | PD5 |
| Keypad | Row 2 | PD6 |
| Keypad | Row 3 | PD7 |
| Buzzer | OC0 | PB3 |
| DS1307 | SCL | PC0 |
| DS1307 | SDA | PC1 |

**Hardware notes:**
- Connect the DS1307 to the ATmega32 TWI pins (PC0/SCL and PC1/SDA).
- Use the required pull-up resistors on the I²C/TWI bus.
- Ensure the DS1307 has the correct supply and backup-battery connections.
- The buzzer driver is configured for the OC0 output on PB3.
- Check LCD operating voltage, contrast, and current requirements before powering the circuit.

---

## Configuration

Important configuration values are defined in the relevant driver configuration headers.

| Setting | Current configuration | File |
|---|---|---|
| CPU frequency | 8 MHz | `MCAL/TMR/TMR_config.h` |
| Timer0 mode | CTC | `MCAL/TMR/TMR_config.h` |
| Timer0 prescaler | 64 | `MCAL/TMR/TMR_config.h` |
| Timer1 mode | CTC with OCR1A | `MCAL/TMR/TMR_config.h` |
| Timer1 prescaler | 8 | `MCAL/TMR/TMR_config.h` |
| Timer1 compare value | 9999 | `APP/APP_config.h` |
| Timer1 tick period | 10 ms | `APP/APP_config.h` |
| TWI mode | Master | `MCAL/TWI/TWI_config.h` |
| TWI prescaler | 1 | `MCAL/TWI/TWI_config.h` |
| TWI bit-rate register value | 32 | `MCAL/TWI/TWI_config.h` |
| RTC time format | 24-hour | `HAL/DS1307/DS1307_config.h` |
| LCD data port | PORTA | `HAL/LCD/LCD_config.h` |
| Keypad port | PORTD | `HAL/KPD/KPD_config.h` |
| Buzzer output | PB3 / OC0 | `HAL/BUZZER/BUZZER_config.h` |

---

## Building and Running

The source targets the **ATmega32** and an **8 MHz** CPU clock.

### Requirements

- AVR-GCC toolchain.
- ATmega32 device support.
- AVR programmer and hardware, or a compatible AVR simulator such as Proteus.
- LCD, 4×4 keypad, DS1307 module, and buzzer connected according to the configuration.

### Build Steps

1. Clone or download this repository.
2. Open the source tree in an AVR-GCC-compatible IDE or create a project that targets `ATmega32`.
3. Add all `.c` files from `APP`, `SERVICES`, `HAL`, and `MCAL`, together with `main.c`, to the build.
4. Configure the compiler's include paths so the project's header files can be found.
5. Ensure the CPU frequency is set to `8000000UL`.
6. Compile the project and resolve any toolchain-specific configuration required by your environment.
7. Flash the generated HEX file to the ATmega32 or load it into your simulator.

**Build note:** The repository contains the source files but does not currently include a ready-made Makefile or IDE project file. The project was developed with an AVR-GCC toolchain; compiler-version or IDE settings may need to be adjusted in a different environment.

---

## Engineering Challenges

### 1. Building the Software Stack from Scratch

No ready-made peripheral drivers or external code libraries were used for the project implementation. The low-level utilities, register-level drivers, hardware interfaces, services, and application logic were developed as part of the project.

This included the fundamental bit operations (Set, Clear, Toggle, and Read), DIO pin/port control, timer configuration, interrupt handling, TWI communication, and the higher-level LCD, keypad, and RTC interfaces.

### 2. Coordinating Melody Frequency and Duration

Playing a melody is not just a matter of generating a tone. Each note needs a specific frequency and duration, and the next note must start at the correct time.

The solution uses Timer0 to generate the note frequency and Timer1 to provide a 10 ms time base for note-duration tracking. Melody progression is handled by the service task rather than by blocking the main loop with delays.

### 3. Managing Competing Sound Requests

The alarm and countdown can both request audible notifications. The Buzzer Service centralizes these requests and applies a defined priority, allowing the alarm notification to take precedence over the countdown notification.

### 4. Keeping the Application Modular

The clock, alarm, stopwatch, countdown, user interface, and sound playback have different responsibilities. Separating them into drivers and services makes the code easier to understand, test, maintain, and extend.

---

## Development Approach

The project was developed with emphasis on:

- **Register-level understanding:** configuring and controlling peripherals through their hardware registers.
- **Driver development:** creating reusable interfaces instead of depending on pre-built drivers.
- **Layered architecture:** separating hardware access, reusable services, and application behavior.
- **Interrupt-based timing:** using timer interrupts to provide periodic timing events.
- **Non-blocking design:** avoiding long delays in the melody playback flow.
- **Hardware/software integration:** combining RTC communication, display, keypad input, timers, and sound output in one application.

---

## Acknowledgment

Developed as part of the **Embedded Systems Based on AVR Diploma at IMT School**.

**Author:** Beshoy Gamil Hanna  
**Microcontroller:** ATmega32  
**Language:** Embedded C  
**Clock Frequency:** 8 MHz
