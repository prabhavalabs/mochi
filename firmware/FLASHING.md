# Flashing and recovery

These instructions target **Waveshare ESP32-S3-Touch-AMOLED-2.16** with 16 MB flash
and 8 MB OPI PSRAM. Keep USB at the bottom. Verify the board model before writing
firmware, and keep any existing firmware/settings backup private.

## Build and identify the device

From the repository root, with PlatformIO on your PATH:

```sh
pio run --project-dir firmware
pio device list
```

Find the Espressif interface and substitute its port for `YOUR_PORT`. The tested
native USB interface uses VID/PID `303A:1001`; the port can change after reconnecting.
On Linux, follow [PlatformIO's USB permissions guidance](https://docs.platformio.org/en/latest/core/installation/udev-rules.html)
if access is denied. Use a USB data cable and close other serial monitors before
flashing.

## Normal upload

```sh
pio run --project-dir firmware --target upload --upload-port YOUR_PORT
pio device monitor --port YOUR_PORT --baud 115200
```

Send `status` followed by a newline. A successful startup reports the example's
version, `ready: true`, display/touch/PMIC status and PSRAM size. A missing touch or
power chip can still allow display-only use; inspect each flag separately.

Flashing replaces the installed application. Normal upload does not deliberately
perform a full-device erase, and it does not modify eFuses or the chip's immutable ROM.

## Build outputs and offsets

Files are produced under `firmware/.pio/build/waveshare_amoled_216/`:

| File | Purpose | Flash offset |
| --- | --- | --- |
| `firmware.factory.bin` | Combined bootloader, partition table, boot metadata and app | `0x0` |
| `firmware.bin` | Application only, for a matching partition layout | `0x10000` |
| `bootloader.bin` | ESP32-S3 bootloader | `0x0` |
| `partitions.bin` | Partition table | `0x8000` |

Use the combined factory image for an initial installation. Do not write the
standalone application at address zero. Compiled firmware and device backups are
not stored in this source repository.

## USB JTAG alternative

Use [Espressif's OpenOCD distribution](https://github.com/espressif/openocd-esp32)
with ESP32-S3 support, rather than a generic build without Espressif extensions.
Ensure its `bin` directory is on PATH and its script directory is available to
OpenOCD (use `-s /path/to/openocd/scripts` if necessary).

The following command has been tested with OpenOCD `v0.12.0-esp32-20260304`:

```sh
openocd -f board/esp32s3-builtin.cfg \
  -c 'adapter speed 20000' \
  -c 'gdb port disabled' -c 'tcl port disabled' -c 'telnet port disabled' \
  -c 'program_esp firmware/.pio/build/waveshare_amoled_216/firmware.factory.bin 0x0 verify reset exit'
```

Connect only the intended board, or add `-c 'adapter serial YOUR_DEVICE_SERIAL'`
using the serial from your device inventory. The 20 MHz clock avoided transfer
CRC errors seen at 40 MHz during development. Confirm **Verify OK** in the output,
then check startup through the USB console. Do not run a JTAG flash and a serial
upload at the same time.

## Enter download mode manually

If automatic entry fails, follow the [Waveshare FAQ](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-2.16/FAQ):

1. Fully power the board off. A battery can keep it running after USB is removed.
2. Hold **BOOT/−**, power it on/reconnect USB, and then release BOOT/−.
3. Retry upload. Recheck the port if it changed.
4. Power-cycle after programming if the application does not start automatically.

Do not assume an unplugged board is off when a battery is attached. The PWR button
retains its board power function; use the vendor's procedure for the fitted battery.

## Backups and complete erasure

A full flash read can include Wi-Fi credentials and data left by an earlier app.
Store backups privately, outside source control. The example does not require a
full erase on each update. If deliberately clearing all stored data, use the
vendor's [flashing and erasing guide](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-2.16/Firmware-Flashing)
and verify the target first. Complete erasure destroys settings and files as well
as the application; it does not erase eFuses.

## After flashing

Select every character and mood, test tap/swipe and the two application buttons,
then check pause, tour and pace. `status` can report zero FPS for a static picker;
close it and press Play to measure animation. A red display indicates framebuffer
allocation failed: confirm the 8 MB OPI PSRAM configuration and inspect startup logs.
