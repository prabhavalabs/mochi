# Mochi SDK 0.3.0

Mochi is now a reusable C++17 SDK. The touchscreen playground in `../firmware/`
is an application built with these two packages:

| Package | Responsibility | Dependencies |
| --- | --- | --- |
| [Mochi](Mochi/README.md) | Reusable character instances, timed reactions, commands, RGB565 rendering, palettes, gestures | C++17 standard library |
| [MochiWaveshare216](MochiWaveshare216/README.md) | Display, touch, PSRAM framebuffer and power initialization for the 2.16-inch Waveshare board | Arduino-ESP32, Arduino_GFX, SensorLib, XPowersLib, Mochi |

The core contains no GPIO assignments, Arduino calls, display drivers, network
clients, audio drivers or application menus. To support another board, supply an
RGB565 buffer, present it with that board's driver, and feed calibrated touch
reports into the gesture recognizer.

```cpp
#include <Mochi.h>

mochi::Mascot buddy;

// Call these when your application changes activity.
buddy.setState(mochi::State::Thinking);
buddy.trigger(mochi::State::Happy, 1.5f); // React, then resume Thinking.
buddy.blink();
buddy.setState(mochi::State::Speaking);
buddy.setSpeechLevel(0.65f);  // Optional mouth opening from your audio envelope.

// Call every animation tick using actual elapsed seconds.
buddy.update(1.0f / 30.0f);
```

The [core guide](Mochi/README.md) shows framebuffer ownership and rendering. The
[board guide](MochiWaveshare216/README.md) shows the hardware integration.
The [integration guide](../docs/INTEGRATION.md) covers embedding characters in
other applications, commands from tasks, independent widgets, and small buffers.

## Build and check

From the repository root:

```sh
python3 scripts/test-sdk.py
python3 scripts/test-install.py
pio run --project-dir firmware
pio run --project-dir examples/freertos_companion
```

The sanitizer runner compiles independent SDK consumers on macOS/Linux. It checks
reactions, command queues, clipping, stripes, playback, speech, gestures and all
character/state combinations. The install test checks a relocated CMake package
without its original checkout. PlatformIO builds both embedded applications.

For a standalone CMake build, including the runnable offscreen example:

```sh
cmake -S sdk/Mochi -B /tmp/mochi-sdk-build \
  -DMOCHI_BUILD_TESTS=ON -DMOCHI_BUILD_EXAMPLE=ON
cmake --build /tmp/mochi-sdk-build
ctest --test-dir /tmp/mochi-sdk-build --output-on-failure
/tmp/mochi-sdk-build/mochi_offscreen /tmp/mochi.ppm
/tmp/mochi-sdk-build/mochi_cast /tmp/cast.ppm
/tmp/mochi-sdk-build/mochi_application /tmp/application.ppm
/tmp/mochi-sdk-build/mochi_stripes /tmp/stripes.ppm
```

## Scope of this version

Version 0.3 adds `Mascot`, temporary reactions, value commands, an optional
single-producer/single-consumer queue, bounded surface views, and relocatable
CMake installation. The independent FreeRTOS example shows commands from worker
tasks without the touchscreen picker. Existing `Animator` and `Renderer` APIs
remain available. Mochi, Sprout, Peach and Nimbus share ten states and speech-level,
blink and playback controls.

This is an early embedded SDK version. Its API may evolve before 1.0. The
browser playground remains a separate JavaScript reference, not a published
React package. Speech level controls animate the mouth; speech synthesis,
microphone capture, network transports and IMU reactions belong to the host
application and are not implemented here.

Packages are prepared locally with PlatformIO manifests. Nothing has been
published to a package registry.
