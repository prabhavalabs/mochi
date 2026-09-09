# Changelog

## 0.3.0 — Application integration

- Reusable `Mascot` instances with independent appearance, playback and visibility.
- Temporary reactions that return to the application's base state; explicit cancellation and retriggering.
- Validated value commands and an optional bounded single-producer/single-consumer queue.
- Zero-copy RGB565 subviews for embedding characters in existing screens.
- Independent application, small-buffer stripe, and FreeRTOS queue examples.
- Relocatable CMake installation with a `Mochi::Mochi` imported target.
- Integration documentation covering task ownership, GUI buffers, timing and portability limits.
- Touchscreen application now uses the reusable instance API; USB `trigger` and `cancel` commands.
- Native reaction/subview/queue tests, installed-package checks, and both embedded builds in CI.
- Existing `Animator`, `Renderer`, pose and gesture source APIs remain supported.

## 0.2.0 — Initial public source distribution

- Portable C++17 animation, RGB565 rendering, palettes and gesture APIs.
- Mochi, Sprout, Peach and Nimbus, each supporting ten animation states.
- Waveshare ESP32-S3-Touch-AMOLED-2.16 adapter and touchscreen example.
- Character and mood pickers, blink, pause, speed, tour and USB controls.
- Standalone offscreen examples and a browser reference for Mochi.
- Native sanitizer checks, CMake tests and automated firmware builds.
- Reject malformed and overlong USB command lines without executing their suffixes.
- Offline browser assets, MIT licensing, contributor and security documentation.

Earlier 0.1.0 and standalone builds were local development versions. The public
source starts at 0.2.0; APIs may evolve before 1.0.
