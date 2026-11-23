# 32gx

**32gx** is a Sega 32X emulator for the Nintendo Wii, built using `devkitPPC` and `libogc`. It features a modern, card-style file browser with support for loading ROMs directly from SD cards and USB drives.

## Features
- **Card-Style GX File Browser:** Smooth graphical navigation optimized for SD and USB storage devices.
- **Wii Hardware Acceleration:** Uses Wii's GX graphics pipeline for rendering and layout.
- **Multi-Storage Support:** Automatically detects and reads from `sd:/` and `usb:/`.

## Building from Source

To compile this project, you need the [devkitPro](https://devkitpro.org/) toolchain installed with `devkitPPC` and `libogc`.

1. Set your environment variable:
   ```bash
   export DEVKITPPC=/opt/devkitpro/devkitPPC

    Run make in the root directory to generate the boot.dol file:
    Bash

    make

Controls

    D-Pad / Pointer: Navigate the file browser grid.

    A Button: Open folder or select ROM.

    HOME Button: Exit to Homebrew Channel.
