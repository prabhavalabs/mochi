---
slug: 'first-character'
title: 'Your first character'
group: 'Start here'
order: 3
description: 'Create an independent companion, draw it into a borrowed buffer, and trigger a temporary reaction from your application.'
---

## Create the buffer and companion

Keep large pixel buffers in static storage or memory allocated by your display driver. A 320 × 320 RGB565 framebuffer needs 204,800 bytes; avoid putting it on a small embedded task stack.

```cpp
#include <Mochi.h>
#include <array>

constexpr int width = 320, height = 320;
std::array<uint16_t, width * height> pixels{};
mochi::Surface screen{pixels.data(), pixels.size(), width, height};
mochi::Mascot buddy;
```

## Advance and draw

Call this from the task that owns the character. Supply elapsed **seconds** from your application's monotonic clock.

```cpp
void frame(float elapsedSeconds) {
    buddy.update(elapsedSeconds);
    mochi::Renderer(screen).clear();
    buddy.draw(screen, 160.f, 160.f, 0.8f);
    // Present pixels using your display driver.
}
```

`draw()` only draws the character. Clear or restore the old background before each frame, and present all affected pixels afterward. The SDK does not own your framebuffer, display, timer, or event loop.

## Respond to an event

```cpp
void onWorkStarted() {
    buddy.setState(mochi::State::Thinking);
}

void onWorkFinished() {
    buddy.setState(mochi::State::Idle);
    buddy.trigger(mochi::State::Happy, 1.5f);
}
```

A reaction replaces any active reaction and returns to the base state when its duration expires. `setState()` changes that base state and cancels a reaction. `cancelTrigger()` ends it early. Durations are affected by speed and pause.

For callbacks on other tasks, send a [command](/docs/commands/) to the owner instead of calling the instance directly.

## Make it yours

```cpp
buddy.setCharacter(mochi::Character::Sprout);
auto palette = buddy.palette();
palette.blush = mochi::rgb565(0xF3B8A1);
buddy.setPalette(palette);
```

Changing character keeps the current mood and time, but loads the new character's default palette. Apply your colors afterward. The [Mascot API](/docs/mascot-api/) covers the complete high-level interface.
