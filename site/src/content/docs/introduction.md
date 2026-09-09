---
slug: 'introduction'
title: 'Introduction'
group: 'Start here'
order: 1
description: 'Mochi is a lightweight, open-source C++17 animation SDK for bringing expressive characters to embedded screens. Your application decides when they listen, think, celebrate, or simply keep you company.'
---

## Choose your starting point

- **[Portable C++](/docs/installation/)** — Build the core on your computer, render an image, and connect your own screen.
- **[ESP32 touchscreen](/docs/waveshare/)** — Run the complete character playground on the Waveshare 2.16-inch AMOLED board.
- **[Browser playground](/docs/playground/)** — Meet the cast and explore every emotion, without hardware or a build.

## Your first reaction

Create a companion and let it respond to something in your application:

```cpp
#include <Mochi.h>

mochi::Mascot buddy;

// Run these on the task that owns the companion.
buddy.setState(mochi::State::Idle);
buddy.trigger(mochi::State::Happy, 1.5f);
```

Keep calling `update(elapsedSeconds)` and `draw(...)` from your rendering loop. After 1.5 **animation seconds**, the reaction returns to Idle. Pause and speed affect that duration. The [first-character guide](/docs/first-character/) shows the complete loop.

## A small core, an open canvas

The core allocates no heap memory. It draws into a buffer you provide and starts no background tasks. Put a character inside a status widget, add several companions to a display, or render in small strips when memory is tight. No selection menu is required in your application.

`Mascot` combines appearance and playback. `Animator` and `Renderer` remain available separately. An optional board adapter handles the Waveshare display, touch, and power initialization. The website is an independent JavaScript preview of the designs; it does not run C++ or connect to your device.

## What’s included

Four characters, ten states, editable RGB565 palettes, blinks, temporary reactions, speech-level mouth movement, input gestures, and task-friendly commands. [Meet the characters](/docs/characters/) or browse the [SDK reference](/docs/api-reference/).

Version **0.3.0** is an early SDK. Build from the repository; packages are not yet available in a public registry. Source compatibility can change before 1.0. Speaking and Listening are visual states; audio capture and speech synthesis are outside the SDK.
