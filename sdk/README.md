# Mochi SDK 0.2.0

Mochi is now a reusable C++17 SDK. The touchscreen playground in `../firmware/`
is an application built with these two packages:

| Package | Responsibility | Dependencies |
| --- | --- | --- |
| [Mochi](Mochi/README.md) | Character states, playback, RGB565 rendering, palettes, tap/swipe recognition | C++17 standard library |
| [MochiWaveshare216](MochiWaveshare216/README.md) | Display, touch, PSRAM framebuffer and power initialization for the 2.16-inch Waveshare board | Arduino-ESP32, Arduino_GFX, SensorLib, XPowersLib, Mochi |

The core contains no GPIO assignments, Arduino calls, display drivers, network
clients, audio drivers or application menus. To support another board, supply an
RGB565 buffer, present it with that board's driver, and feed calibrated touch
reports into the gesture recognizer.

```cpp
#include <Mochi.h>

mochi::Animator buddy;

// Call these when your application changes activity.
buddy.setState(mochi::State::Thinking);
buddy.blink();
buddy.setState(mochi::State::Speaking);
buddy.setSpeechLevel(0.65f);  // Optional mouth opening from your audio envelope.

// Call every animation tick using actual elapsed seconds.
buddy.update(1.0f / 30.0f);
```

The [core guide](Mochi/README.md) shows framebuffer ownership and rendering. The
[board guide](MochiWaveshare216/README.md) shows the hardware integration.

## Build and check

From the repository root:

```sh
python3 scripts/test-sdk.py
pio run --project-dir firmware
```

The first command compiles independent SDK consumers on the Mac with memory and
undefined-behavior sanitizers. It checks state transitions, callbacks, playback,
speech controls, gestures, buffer bounds, custom palettes, distinct silhouettes
and rendering bounds for all four characters in all ten states. The second builds the actual ESP32 example against
the local SDK packages.

For a standalone CMake build, including the runnable offscreen example:

```sh
cmake -S sdk/Mochi -B /tmp/mochi-sdk-build \
  -DMOCHI_BUILD_TESTS=ON -DMOCHI_BUILD_EXAMPLE=ON
cmake --build /tmp/mochi-sdk-build
ctest --test-dir /tmp/mochi-sdk-build --output-on-failure
/tmp/mochi-sdk-build/mochi_offscreen /tmp/mochi.ppm
/tmp/mochi-sdk-build/mochi_cast /tmp/cast.ppm
```

## Scope of this version

Version 0.2 adds `Character::Mochi`, `Sprout`, `Peach` and `Nimbus`. Call
`renderer.setCharacter()` to switch appearance without changing animation state.
All four support the same ten states and speech-level, blink and playback APIs.
The touchscreen example includes a visual character picker.

This is an early embedded SDK version. Its API may evolve before 1.0. The
browser playground remains a separate JavaScript reference, not a published
React package. Speech level controls animate the mouth; speech synthesis,
microphone capture, network transports and IMU reactions belong to the host
application and are not implemented here.

Packages are prepared locally with PlatformIO manifests. Nothing has been
published to a package registry.
