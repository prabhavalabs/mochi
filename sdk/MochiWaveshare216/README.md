# MochiWaveshare216

Arduino adapter for **Waveshare ESP32-S3-Touch-AMOLED-2.16**: CO5300 480 × 480
display, CST9220 touch, AXP2101 power chip, 16 MB flash and 8 MB OPI PSRAM.
Version 0.2.0. Use with the Mochi core package at the same version.

## Add both local packages

```ini
[env:mochi]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.38-1/platform-espressif32.zip
board = esp32-s3-devkitc1-n16r8
framework = arduino
board_upload.flash_size = 16MB
board_build.flash_mode = qio
board_build.arduino.memory_type = qio_opi
board_build.psram_type = opi
board_build.partitions = default_16MB.csv
build_unflags = -std=gnu++11
build_flags =
  -std=gnu++17
  -D ARDUINO_USB_CDC_ON_BOOT=1
  -D ARDUINO_USB_MODE=1
lib_deps =
  symlink:///absolute/path/to/sdk/Mochi
  symlink:///absolute/path/to/sdk/MochiWaveshare216
```

PlatformIO resolves the adapter's pinned graphics, touch and power dependencies.
Both local packages must be listed because these packages are not in the public
registry. The repository's `firmware/` directory is the complete touchscreen
playground built against this adapter and the core.

## Minimal application

```cpp
#include <Arduino.h>
#include <Mochi.h>
#include <MochiWaveshare216.h>

namespace board = mochi::waveshare216;
mochi::Animator buddy;
bool ready = false;
uint32_t lastFrame = 0;

void setup() {
    ready = board::begin();
    buddy.setState(mochi::State::Waving);
    lastFrame = millis();
}

void loop() {
    delay(1);
    const uint32_t now = millis();
    if (!ready || now - lastFrame < 50) return;
    buddy.update((now - lastFrame) / 1000.f);
    lastFrame = now;
    auto& canvas = board::canvas();
    mochi::Renderer renderer({canvas.getFramebuffer(), 480 * 480, 480, 480});
    renderer.clear();
    renderer.draw(buddy.pose(), 240, 240, 1.1f);
    board::present();
}
```

## Adapter API

| Function | Contract |
| --- | --- |
| `begin()` | Initialize once. Return true when display and PSRAM framebuffer are usable. Repeated calls return the first result. |
| `status()` | Read independent `display`, `touch`, and `power` detection flags. A missing touch/PMIC does not prevent display-only use. |
| `canvas()` | Access the adapter-owned Arduino_Canvas after successful initialization. Do not free its pixels or call its `begin()`/`flush()` methods. |
| `present(y, height)` | Send full-width rows, defaulting to the entire display. Clip to screen and expand to even row alignment required by CO5300. |
| `setBrightness(percent)` | Set 0..100 percent after initialization; reject invalid/uninitialized requests. |
| `readTouch(x, y)` | Get a new `mochi::TouchReport`; `None` preserves the recognizer's contact state. |

Keep the USB connector at the bottom. Display orientation and touch transforms
are paired. Constants `kWidth`, `kHeight`, `kUserButton` and `kBootButton` expose
the panel size and application buttons. This adapter assumes exclusive ownership
of the display SPI bus, the shared board I²C bus and the touch interrupt.

The framebuffer consumes 460,800 bytes of PSRAM. Character/menu placement and
partial-redraw bounds belong to the application. For example, the playground
updates rows 48–347 during animation, and all rows when its controls change.
Never use those bounds for differently positioned/scaled artwork without checking
its full extent.

The onboard accelerometer, gyroscope, microphones and audio codec are not exposed
by this adapter. The USB JTAG flashing and recovery steps are documented in the
repository's [flashing guide](../../firmware/FLASHING.md).
