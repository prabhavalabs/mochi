# Port Mochi to another display

The portable package does not depend on the Waveshare adapter. Keep the core and
supply rendering, timing and input integration for your platform.

## 1. Allocate a framebuffer

Provide writable native-endian `uint16_t` RGB565 pixels. For a tightly packed
buffer, capacity is `width * height` pixels, or twice that many bytes. A stride
larger than width is supported; capacity must cover `(height - 1) * stride + width`.
Keep the allocation alive for every renderer that borrows it. On embedded targets,
large buffers usually belong in PSRAM or another suitable long-lived allocation.

```cpp
mochi::Renderer renderer({pixels, capacityInPixels, width, height, strideInPixels});
if (!renderer.valid()) {
    // Handle an invalid allocation or layout before rendering.
}
```

The renderer performs no allocation. An adapter may allocate a framebuffer at
startup; the included Waveshare adapter does so in PSRAM.

## 2. Drive animation and present pixels

Use actual elapsed seconds for `Animator::update()`, then clear/restore the old
background, draw the current pose, and send the changed pixels to the display.
Choose a center and scale that fit all poses, including raised hands, leaves and
ears. Selecting a character on the renderer does not change the animator's state.

Keep pixel byte swapping at the display-driver boundary. The SDK always treats
its surface as native-endian RGB565, regardless of the panel's wire format.

Start with complete-frame updates. If optimizing to a smaller rectangle, verify
every character and state remains inside it. The existing demo's rows 48–347 are
specific to its 480 × 480 layout and cannot be reused blindly. Respect the panel's
alignment rules; the CO5300 adapter expands updates to even rows.

## 3. Feed calibrated touch reports

Apply display rotation and touch mirroring in the adapter, so touch coordinates
match the rendered screen. Pass `Pressed`, `Released`, or `None` reports into
`GestureRecognizer::update()` with a wrapping `uint32_t` millisecond timestamp.
Keep polling during `None` so missing releases can expire safely. The application
decides what taps and swipes mean; the core does not own menus.

For non-touch targets, change states from physical buttons or application events.
Use one task for input, animation and rendering, or queue commands to that task.

## 4. Keep the adapter separate

Put pin maps, reset sequences, bus ownership and driver dependencies in their own
package. Document the exact board revision and supported toolchain. Preserve
upstream licenses. Do not introduce driver headers or GPIO constants into the core.

## 5. Validate the port

Run the native SDK tests, then check display orientation, RGB565 colors, clipping,
all 40 character/state combinations, pause and pace, touch coordinates and reconnects
on the real device. Record framebuffer placement, memory use, frame rate and any
unsupported hardware. Use the [contribution checklist](../CONTRIBUTING.md).
