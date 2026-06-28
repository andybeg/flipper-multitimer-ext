# Flipper Zero MultiTimer Ext

MultiTimer Ext is a Flipper Zero timer application with saved timer presets,
a clock screen, optional external 4.2" WeActStudio e-paper output, and an
OpenSCAD enclosure model for mounting the display with Flipper Zero.

The Flipper application id is `multitimerext`; the built file is
`dist/multitimerext.fap`.

## Features

- `Clock` screen with current time and date.
- Saved timer presets shown directly in the main menu.
- Default saved timers: `00:01:00`, `00:03:00`, `00:05:00`, `00:10:00`.
- `Add Timer` screen for creating custom saved timers.
- Up to `20` saved/active timers.
- Pause, resume, stop, and delete saved timers.
- Persistent settings and saved timers.
- Optional WeActStudio 4.2" black/white e-paper output over SPI.
- Configurable e-paper refresh interval, refresh mode, and color inversion.
- Optional always-on Flipper backlight while the app is running.
- Parametric OpenSCAD enclosure for an angled e-paper mount.

## Build And Install

Install `ufbt`, then build from the repository root:

```bash
ufbt
```

The generated app is:

```text
dist/multitimerext.fap
```

To build, upload, and launch on a connected Flipper Zero:

```bash
ufbt launch
```

Close `qFlipper` and browser-based Flipper tools before using `ufbt launch`,
because they can hold the serial port.

## Main Menu

- `Clock` - shows current time and date.
- Saved timers - selecting one starts that countdown.
- `Add Timer` - creates a new saved timer and starts it.
- `Active Timers` - shows and manages currently running timers (count shown in menu).
- `Settings` - app and display settings.

## Add Timer Controls

- `Left` / `Right` - select hours, minutes, or seconds.
- `Up` / `Down` - change the selected value.
- `OK` - save the timer into the first free saved slot and start it.
- `Back` - return to the main menu.

## Running Timer Controls

- `OK` - pause or resume.
- `Back` - return to main menu (timer keeps running).
- `Left` - stop the active timer.
- `Right` - open Active Timers list.
- `Down` - while paused on a saved preset, delete that preset.

## Active Timers Controls

- `Up` / `Down` - select a timer.
- `OK` - open the selected timer.
- `Left` - stop or dismiss the selected timer.
- `Back` - return to main menu.

Delete confirmation (saved presets):

- `OK` - delete the saved timer.
- `Back` - cancel.

## Settings

Main settings:

- `Backlight On` - immediately toggles Flipper backlight behavior.
- `alarm time` - alarm duration, adjustable in 10 second steps.
- `E-Ink Settings` - opens external display settings.

E-Ink settings:

- `Show on E-Ink` - enables or disables all e-paper activity.
- `E-Ink Mode` - `Full` or `Partial` refresh.
- `Invert colors` - toggles inverted framebuffer output.
- `E-Ink refresh` - update interval, adjustable in 10 second steps.

If `Show on E-Ink` is off, the app does not initialize or update the external
display. If the display is not connected, the app should continue working on
Flipper normally.

## E-Paper Wiring

For the WeActStudio 4.2" black/white e-paper module:

```text
E-paper VCC  -> Flipper 3V3
E-paper GND  -> Flipper GND
E-paper SCL  -> Flipper PB3   // SPI SCK
E-paper SDA  -> Flipper PA7   // SPI MOSI
E-paper CS   -> Flipper PA4
E-paper D/C  -> Flipper PC0
E-paper RES  -> Flipper PC1
E-paper BUSY -> Flipper PC3
```

`MISO` is not used. Use `3V3`, not `5V`.

## Enclosure

The enclosure model is in:

```text
enclosure/flipper_eink_mount.scad
```

It is an OpenSCAD Customizer-friendly model with editable sections for:

- base box dimensions;
- e-paper board dimensions;
- angled screen holder and slide-in grooves;
- GPIO header position;
- adapter board opening;
- optional screen screw holes.

Generated STL:

```text
enclosure/flipper_eink_mount.stl
```

Before printing, measure the exact e-paper PCB dimensions and the Flipper GPIO
height while Flipper lies on its back cover.

## Data Storage

Saved timers and settings are stored under the app data directory:

```text
SD Card/apps_data/multitimerext/timers.dat
```

Delete this file to reset saved timers and settings.

## Development

Requirements:

- `ufbt`
- Flipper Zero firmware compatible with the target API used by `ufbt`
- OpenSCAD, only if editing/exporting the enclosure

Useful commands:

```bash
ufbt
ufbt launch
openscad -o enclosure/flipper_eink_mount.stl enclosure/flipper_eink_mount.scad
```

## License

This project is licensed under the MIT License. See `LICENSE`.
