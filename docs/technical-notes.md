# 32gx Technical Notes & Architecture

## Overview
**32gx** is a standalone Sega 32X emulator designed specifically for the Nintendo Wii, utilizing `devkitPPC`, `libogc`, and the Wii's native GX graphics pipeline. 

---

## 1. Emulated System Architecture (Sega 32X)
The emulator must coordinate multiple distinct processing units and audio systems simultaneously:
* **Motorola 68000:** Runs at ~7.67 MHz, handling primary Sega Genesis game logic, VDP background layers, and initial system boot.
* **Dual Hitachi SH-2 RISC CPUs:** Two 32-bit processors running at ~23 MHz, responsible for 32X polygon rendering, 3D math, and supplemental game logic.
* **Zilog Z80:** Runs at ~3.58 MHz for sound generation (YM2612 FM synth & SN76489 PSG).
* **32X PWM Audio:** 2-channel Pulse Width Modulation audio system unique to the 32X add-on.

---

## 2. Wii Host Constraints & Optimization Strategy
Developing for the Wii imposes strict hardware limitations that dictate how the emulator is structured:
* **CPU:** 729 MHz PowerPC Broadway (single-core). Emulating multiple CPU architectures (M68k + dual SH-2s) requires careful synchronization and efficient instruction decoding (or dynamic recompilation/JIT where applicable).
* **RAM:** 88 MB total available memory. ROM files and system states must be managed efficiently to avoid memory overflow.
* **Graphics (GX):** Instead of CPU-heavy pixel blending, the final framebuffers (Genesis layer + 32X VDP layer) are uploaded and composited using the Wii's hardware GX graphics pipeline.

---

## 3. Memory Mapping & Inter-Processor Communication
* **Shared Registers:** The Genesis 68000 and the 32X SH-2 chips communicate via shared communication registers (`COMM0` through `COMM7`) and mailbox flags.
* **Framebuffer Access:** The 32X features a packed-pixel or run-length encoded framebuffer that is written to by the SH-2 chips and read by the video subsystem for display output.

---

## 4. Roadmap & Development Phases
- [x] **Phase 1:** Project repository setup, `devkitPPC` Makefile configuration, and GX card-style SD/USB file browser.
- [ ] **Phase 2:** Base Genesis core integration (68k + VDP + Z80).
- [ ] **Phase 3:** 32X bus mapping, communication registers, and dual SH-2 execution loops.
- [ ] **Phase 4:** GX video hardware compositing and controller input mapping (Wii Remote, Classic Controller, GameCube Pad).
