---
slug: 'rendering'
title: 'Rendering & memory'
group: 'SDK'
order: 32
description: 'Fit a companion into your existing interface, from a full framebuffer to a few rows of pixel storage.'
---

## Borrow a surface

```cpp
mochi::Surface screen{pixels, capacityInPixels, width, height, strideInPixels};
```

The buffer must remain alive while used. Capacity and stride are in **pixels**, not bytes. A zero stride means width; a larger stride supports padding. `valid_surface()` checks the declared layout, but the application must provide the real allocation size.

Pixels are native-endian `uint16_t` RGB565 values. Convert byte order only when your display interface requires it. Rendering never writes row padding.

## Put a widget in an existing screen

```cpp
auto widget = mochi::sub_surface(screen, 20, 30, 120, 120);
mochi::Renderer(widget).clear(mochi::rgb565(0x18231C));
buddy.draw(widget, 60.f, 60.f, 0.28f);
```

The zero-copy view retains the parent's stride and clips to its exact rectangle. An invalid rectangle returns an empty surface. Keep the parent alive. Coordinates within the widget start at its own top-left corner.

## Plan memory

| Buffer               | Pixel storage |
| -------------------- | ------------- |
| 120 × 120 RGB565     | 28,800 bytes  |
| 320 × 320 RGB565     | 204,800 bytes |
| 480 × 480 RGB565     | 460,800 bytes |
| 480 × 8 RGB565 strip | 7,680 bytes   |

The core allocates no heap memory; pixel buffers and display-driver allocations still count toward your device budget. Avoid large automatic arrays on embedded task stacks.

## Render with small buffers

Capture one pose per frame, then render each strip with a vertically shifted origin. Advance the animation once for the entire frame, not once per strip. Send each finished strip to your display before reusing it. The repository's [stripes example](https://github.com/prabhavalabs/mochi/blob/main/sdk/Mochi/examples/stripes.cpp) is a complete implementation.

## Clear and present correctly

Restore the previous background before drawing. If doing partial redraws, include the old and new character extents, moving hands, leaves, and every pose. The built-in artwork is approximately 360 × 280 pixels at scale 1. Use a clipped widget when strict screen boundaries matter.

The renderer accepts dimensions up to 16,384, positive scales up to 100, and centers within ±1,000,000 pixels. It rejects invalid/nonfinite transforms and out-of-contract poses without touching the buffer. Refer to [the API contract](/docs/api-reference/#buffers-and-colors) for details.

## GUI and display integration

Render on the task and within the buffer lifetime required by your GUI toolkit. Respect DMA ownership and flush completion; never overwrite pixels still being transferred. The core doesn't synchronize buses or GUI tasks. See [Porting](/docs/porting/) for the driver boundary and [Application integration](/docs/integration/) for multiple independent widgets.
