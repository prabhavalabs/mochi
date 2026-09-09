# FreeRTOS application with two companions

This independent application embeds two character widgets in its own screen,
without the touchscreen playground's picker. Peach receives a Happy reaction
from a worker task every five seconds, returning to Idle after 1.5 animation
seconds. Sprout keeps Thinking independently.

The application uses a statically allocated FreeRTOS queue for copied
`mochi::Command` values. Multiple ordinary tasks may call
`postCompanionCommand()`; it returns `false` if the queue is unavailable or full.
Only the Arduino UI task dispatches commands, updates mascots, or draws pixels.
At most eight commands are drained per frame.

`postCompanionCommandFromISR()` shows the separate interrupt entry point. No
interrupt is installed by this example. Callers must initialize the wake flag,
follow FreeRTOS interrupt-priority rules, and perform the appropriate yield.
The SDK and command queue storage allocate no heap memory; the board's
framebuffer and `xTaskCreate()` may allocate during setup.

## Build and run

From the repository root, with PlatformIO installed:

```sh
pio run --project-dir examples/freertos_companion
pio device list
pio run --project-dir examples/freertos_companion --target upload --upload-port YOUR_PORT
```

This profile targets the same **Waveshare ESP32-S3-Touch-AMOLED-2.16** as the
playground. Its board/toolchain settings and the playground's settings must stay
aligned when dependencies change. Uploading replaces the installed application.
To restore the picker, build/upload `firmware/` instead.

If serial upload fails, follow the [JTAG guide](../../firmware/FLASHING.md), using
`examples/freertos_companion/.pio/build/waveshare_amoled_216/firmware.factory.bin`
as the image path. See the [integration guide](../../docs/INTEGRATION.md) for
portable commands, buffer ownership, and integration with other GUI libraries.
