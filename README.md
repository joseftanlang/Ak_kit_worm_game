# Game built with AK Embedded Base Kit

<div align="center">
  <a href="https://github.com/user-attachments/assets/6f043a43-5259-4eed-88e2-d772b11eac33">
    <video
      src="https://github.com/user-attachments/assets/6f043a43-5259-4eed-88e2-d772b11eac33"
      controls
    ></video>
  </a>
</div>

This repository contains the firmware for the worm game that runs on the AK Embedded Base Kit with STM32L151. The project is a hands-on example of event-driven embedded programming: the screen, buttons, buzzer, timers, EEPROM, and task scheduler work together to present a complete game loop rather than a single demo screen.

The board is designed for embedded learning and prototyping. It combines a 1.54" OLED display, 3 push buttons and a buzzer so you can study interaction, timing, persistence, and modular firmware architecture on real hardware.

<div align="center">
  <a href="https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu">
    <img src="hardware/images/ak-embedded-base-kit-version-3.jpg" width="480"/>
  </a>
</div>

<!-- ## Quick Start

If you just want to understand the game quickly, this is the shortest path:

1. Build the firmware with `make -C application`.
2. Flash the application to the board at `0x08003000`.
3. Use Mode to enter or confirm.
4. Use Up and Down to move through menus and settings.
5. Enter the worm screen, move the worm, eat apples, and avoid colliding with itself. -->

<!--
```mermaid
flowchart TD
 A[Boot board]  B[Open game menu]
 B C[Enter worm screen]
 C D[Move with Up / Down]
 D  E[Eat apples and grow]
 E  F{Crash or win?}
 F |No| D
 F |Yes| G[Show result overlay]
 G  H[Open score chart or return]
```
-->
## Contents

Use this table to jump to major sections in this README.

<!-- - [Quick Start](#quick-start) -->
- [Contents](#contents)
- [How To Read This Project](#how-to-read-this-project)
- [What This Game Is About](#what-this-game-is-about)
- [Main Features](#main-features)
- [Hardware Context](#hardware-context)
- [Memory Map](#memory-map)
- [Game Flow](#game-flow)
- [Game design](#game-design)
<!-- - [Internal Game IDs and Tasks](#internal-game-ids-and-tasks) -->
<!-- - [Settings](#settings)
- [Controls](#controls)
- [Build](#build) -->

## How To Read This Project

- If you are playing the game, read the game overview, controls, and quick start sections first.
- If you are changing firmware behavior, focus on internal game IDs, task ownership, and settings persistence.
- If you are building or flashing, jump straight to the build and flashing sections.

## What This Game Is About

Worm is a compact arcade game built for the display and controls on the Ak Kit. The player moves a worm around the screen, eats apples, avoids crashing into itself, and tries to survive long enough to grow as much as possible. The game also includes a score system, visual feedback, and buzzer music.

The important part of this project is not only the gameplay itself, but how the gameplay is split into small tasks:

- One task owns the worm movement.
- One task owns apples.
- One task owns score handling.
- One task handles collision and eating logic.
- One task manages borders.
- One task coordinates the main game screen.
- One task manages lives. (Optional)

That structure makes the firmware easier to reason about and is a good reference if you are studying cooperative embedded state machines.

## Main Features

- OLED-based worm gameplay with a clean animated screen.
- 3-button control scheme for menu navigation and gameplay.
- Persistent game settings stored in EEPROM.
- Adjustable worm speed from slow to fast.
- Adjustable apple count.
- Selectable buzzer music loop.
- Buzzer enable or disable option.
- Score tracking and top score flow.
- Event-driven task architecture with timer-based updates.

## Hardware Context

The firmware targets the AK Embedded Base Kit hardware stack:

- STM32L151 MCU.
- 1.54" OLED display (V3) or 1.3" OLED display (V2)
- 3 push buttons.
- Buzzer for sound effects and looping music.

### Memory Map

- `0x08000000` - Boot firmware.
- `0x08002000` - Boot/Application shared data region.
- `0x08003000` - Application firmware.

The application is linked to start at `0x08003000` so it matches the boot layout and the shared flash map used by the Ak Kit.

## Game Flow

The game is organized as a screen-driven application:

1. The firmware boots and initializes hardware, tasks, timers, and optional modules.
2. The display shows the main app flow and menu screens.
3. The player enters the worm game screen.
4. The worm is reset, apples are initialized, score is cleared, and music starts if enabled.
5. The game runs on periodic ticks driven by the scheduler.
6. If the player crashes or wins, the game shows an overlay.
7. The player can move to the score chart screen or back out to the menu.

The worm screen itself is also responsible for starting and stopping the background music loop, so the audio state always matches the visible game state.

## Game Sequence

This is the entire game sequence of the worm game from start to end.

<div align="center">
  <img src="resources/image/FPT_worm.io_w.png" alt="Startup 1" height = "512" />
</div>

## Game Design

The game UI is structured into a few clear screens that guide the player from power-on to gameplay, settings, and score review. Each screen is designed to be compact and readable on the small OLED while providing the player with clear feedback and control.

<!-- ### Startup Screen

<div align="center">
  <img src="resources/image/worm_screen.png" alt="Startup 1" width="512"/>
</div> -->
<!-- <div style="width: 100%; text-align: center;">
  <img src="resources/image/worm_screen.png" alt="Startup 1" style="width: 100%; max-width: 100%; height: auto; display: inline-block;">
</div> -->

The startup screen shows the board and firmware identity, provides a brief boot animation and offers entry into the main app menu. It confirms hardware is initialized (display, buttons, buzzer) and gives a short visual cue if saved settings or scores were loaded successfully.

### Menu Screen (Initial Screen)

<div align="center">
  <img src="resources/image/worm_menu_screen.png" alt="Menu 1" width="512" height="256" />
</div>

The menu screen is the central hub. Menu entries are arranged vertically and can be navigated with Up/Down; Mode enters or confirms. Visual highlights and simple icons make it easy to pick modes (Start Game, Settings, Charts). Menu animations are subtle to keep the UI responsive.

### Worm Game (Playfield)

<div align="center">
  <img src="resources/image/worm_game_1.png" alt="Worm Game" width="512" height="256" />
</div>

This is the core gameplay screen. The player controls the worm with Up/Down to change direction. Apples appear as targets; eating them grows the worm and increases score. The HUD is intentionally minimal: score, lives (if enabled), and a small status indicator for buzzer/music.

### Game Over / Result

<div align="center">
  <img src="resources/image/worm_over_1.png" alt="Game Over" width="512" height="256" />
</div>

When the player crashes the worm, an overlay appears with the final score and options to retry or return to the menu. A short buzzer melody and a crisp visual overlay emphasize the result while preserving the final playfield for reference.

### Charts Screen
<div align="center">
  <img src="resources/image/worm_charts.png" alt="Settings 1" width="512" height="256" />
</div>

The Charts screen displays a visual representation of player progress and high scores. It retrieves stored scores from non-volatile EEPROM memory, ensuring that results persist across power cycles. The screen shows the top 3 recorded scores so players can compare their performance over time.

### Settings Screen

<div align="center">
  <img src="resources/image/worm_settings.png" alt="Settings 1" width="512" height="256" />
</div>

Settings let the player tune gameplay and audio quickly:

- Worm speed: five discrete steps from slow to fast.
- Apple count: change the number of active apples (1–8).
- Music selection: choose from a set of tunes.
- Buzzer: toggle sound on or off.

Controls are mapped consistently so a short hold action loads presets while single presses adjust the selected value.

---

Design notes

- Visual clarity: the renderer uses bold pixels and a clear glyph set so text and icons remain legible on the small OLED.
- Modularity: each gameplay system (worm, apples, score, collisions) is owned by a separate task, making it easy to change pacing or rules without touching rendering code.
- Accessibility: simple 3-button controls and presets let new players start quickly without digging through menus.

<!-- ## Internal Game IDs and Tasks

The phrase “game ID” in this codebase is best understood as the internal task ID used by the scheduler. These are not player-facing IDs; they are numeric identifiers that let the firmware route messages to the correct task.

The IDs are declared in [application/sources/app/task_list.h](application/sources/app/task_list.h). The main game-related entries are:

- `WORM_GAME_APPLE_ID` - apple spawning and apple lifecycle.
- `WORM_GAME_WORM_ID` - worm movement and direction control.
- `WORM_GAME_SCORE_ID` - score calculation and persistence.
- `WORM_GAME_EATING_ID` - eating detection and related interactions.
- `WORM_GAME_BORDER_ID` - border and collision boundary management.
- `WORM_GAME_LIVES_ID` - lives and survival logic.
- `WORM_GAME_GAMER_ID` - the main game coordinator and periodic game tick.

The application-level task IDs around them include the system, display, shell, interface, debug, and communication tasks. That split is important because the game is not a standalone loop; it is one module inside a larger embedded runtime.

### Why This Matters

If you are tracing behavior, debugging a screen update, or changing gameplay timing, these IDs tell you which module owns the behavior. For example:

- Movement timing is driven by the worm/game coordinator.
- Apple behavior is independent from the visual screen code.
- Score updates happen through the score module rather than in the renderer.
- Button events are posted as messages instead of being handled directly in the input callback.

## Settings

The settings screen is one of the most useful parts of the firmware because it shows how user preferences are persisted safely and loaded back on startup.

The available settings are:

- Worm speed.
- Apple count.
- Music selection.
- Buzzer enable or disable.

### Worm Speed

Speed is stored as a value from `1` to `5` and mapped to actual tick intervals:

- `1` - 180 ms
- `2` - 150 ms
- `3` - 110 ms
- `4` - 90 ms
- `5` - 50 ms

Higher speed means the worm updates more often and the game becomes harder.

### Apple Count

Apple count is stored from `1` to `8`. This controls how many apples can be active in the game at once, which changes the pacing and the difficulty.

### Music Selection

The available looped songs are:

- `We Will We Will Rock You (ROCK)`
- `Mario Kart (MARIO)`
- `Pirates of the Caribbean (PIRATES)`
- `The Wheel On The Bus (BUS)`
- `Jingle Bells (XMAS)`

The selected music starts when entering the worm screen and loops while the game remains active. and while the screen is active, provided the buzzer is enabled.

### Buzzer Setting

The buzzer can be turned on or off. When it is off, the game silences sound playback instead of trying to restart it.

## Settings Persistence

Settings are persisted in EEPROM so they survive power cycles. The settings structure includes a magic value plus the current configuration values, and the code validates ranges when loading them back.

That means the firmware does not blindly trust stored bytes. If EEPROM contains invalid data, the code falls back to safe defaults instead of using out-of-range values.

This same pattern is also used for worm score storage, which helps keep saved data structured and easier to verify.

## Controls

The button behavior is simple and consistent across the app:

- Mode button is used for enter, confirm, and back/exit depending on the screen.
- Up and Down buttons move through menu items or game settings.
- Button hold actions are also used for shortcuts on some screens.

In the settings screen:

- Up and Down move the selection.
- Mode toggles the selected setting.
- Holding Mode returns to the game menu.
- Holding Up loads a strong gameplay preset.
- Holding Down loads a minimal gameplay preset.

In the worm screen:

- Up and Down change worm direction.
- Mode exits the screen or opens the score flow after game over.

__Using a keyboard:__

The code has been customized to support keyboard input. where you would need to firstly run the code from the application folder:

```
make com dev=/dev/ttyUSB0
```

After this then you would be able to see the following example:

```
taskID: 4       msgType:0x80    refCnt:1        sig:10       waitTime:0      exeTime:0
taskID: 4       msgType:0x80    refCnt:1        sig:10       waitTime:0      exeTime:0
taskID: 4       msgType:0x80    refCnt:1        sig:10       waitTime:0      exeTime:0
```

You can use the keyboard keys in the following table:

| Keys | Command Output |
| --- | --- |
| 'W' or 'w' | UP BUTTON |
| 'A' or 'a' | MODE BUTTON |
| 'S' or 's' | DOWN BUTTON |
| 'D' or 'd' | MODE BUTTON |
| 'H' or 'h' | LONG MODE |
| 'G' or 'g' | LONG MODE |
| 'J' or 'j' | LONG MODE |

## Build

Build the application from the repository root:

```sh
make all
```

The generated artifacts are placed under `application/build_worm-game/`, including the `.axf` and converted `.bin`, `.out`, and `.elf` files.

### Windows Notes

- `application/make.cmd` exists to make local cleanup easier on Windows.
- A GNU Make binary is still required for full builds.
- Tool paths in the Makefile are configured for the ARM GCC and STM32Cube/OpenOCD setup used by this project.

## Flashing

After building the bootloader and application, the firmware can be loaded through the normal board flashing flow used by the kit. The repository also supports direct application flashing through the USB path when the boot firmware and the board setup are already in place.

Example:

```sh
USB-C cable:
ak_flash /dev/ttyUSB0 ak-base-kit-stm32l151-application.bin 0x08003000

or

ST-LINK:
make flash
```

- `application/` - firmware application sources and build system.
- `boot/` - bootloader-related files.
- `hardware/` - board images, schematics, and manufacturing resources.
- `resources/` - bitmap and asset resources used by the game. -->

## References

| Topic | Link |
| ------ | ------ |
| Blog & Tutorial | <https://epcb.vn/blogs/ak-embedded-software> |
| Where to buy KIT? | <https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu> |
| Schematic | [hardware/schematic/schematic-ak-embedded-base-kit-version-3.pdf](hardware/schematic/schematic-ak-embedded-base-kit-version-3.pdf) |