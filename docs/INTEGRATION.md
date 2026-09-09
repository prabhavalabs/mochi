# Use Mochi inside your application

`mochi::Mascot` is a reusable character instance. It owns appearance, animation,
and temporary reactions. Your application owns the clock, buffers, display,
screen layout, and tasks. The touchscreen picker is an example application;
none of its menus or controls are required to use the SDK.

The core requires C++17 and writable RGB565 pixels. It runs without Arduino or
an operating system. A compiled library must match your target architecture and
toolchain; build it for each target. C applications and managed languages need a
binding or bridge. There is no universal binary that runs on every board.

## Trigger a character from application events

Keep the instance alive between frames. These functions run on its owner task:

```cpp
#include <Mochi.h>

mochi::Mascot companion;

void initializeCompanion() {
    companion.setCharacter(mochi::Character::Peach);
    companion.setState(mochi::State::Idle);
}
void onWorkStarted() {
    companion.setState(mochi::State::Thinking);
}
void onWorkCompleted() {
    companion.setState(mochi::State::Idle);
    companion.trigger(mochi::State::Happy, 1.5f);
}
void onMessageReceived() {
    companion.trigger(mochi::State::Waving, 2.0f);
}
```

`setState()` selects the normal activity and cancels a temporary reaction.
`trigger()` temporarily plays a state, then returns to that normal activity.
Reactions replace each other; they do not stack. Triggering the current state
restarts its animation and timer. Invalid arguments return `false` without
changing the instance. Durations must be finite and in `(0, 86400]` seconds.

Durations use **animation seconds**: speed affects them, pause freezes them, and
each `update()` consumes at most 100 ms of input time before applying speed.
Reactions expire on an update boundary; the pose then blends back to the base
state. For a wall-clock deadline, use your application's timer and call
`cancelTrigger()` when that deadline arrives.

| Method | Meaning |
| --- | --- |
| `state()` | Current activity or temporary reaction |
| `baseState()` | Normal activity to resume |
| `triggered()` / `triggerRemaining()` | Reaction active flag / remaining animation seconds |
| `cancelTrigger()` | Return to the normal activity immediately, with pose blending |
| `setVisible(bool)` | Enable/disable drawing; does not pause time or erase old pixels |
| `onStateChange(callback, context)` | Observe synchronous state changes, including reaction expiry |

The callback runs on the task making the call. Keep its context alive, keep the
callback short, and do not re-enter the instance from it. Repeating the same state
does not produce a state-change callback. Character selection and custom palettes
belong to each instance and are independent of playback.

## Put a character in an existing screen

A surface is a borrowed view of native-endian `uint16_t` RGB565 pixels. Capacity
and stride are pixel counts, not bytes. The SDK neither allocates nor retains the
buffer passed to `Mascot::draw()`.

```cpp
void drawCompanion(mochi::Surface applicationScreen, float elapsedSeconds) {
    // Exact 128 x 128 rectangle inside the host framebuffer.
    auto widget = mochi::sub_surface(applicationScreen, 16, 32, 128, 128);
    if (!mochi::valid_surface(widget)) return;

    companion.update(elapsedSeconds);
    mochi::Renderer(widget).clear(mochi::rgb565(0x283341));
    companion.draw(widget, 64, 68, 0.32f);
    // Your driver or GUI presents/invalidates this rectangle.
}
```

`sub_surface()` creates a zero-copy view with the parent's stride. Invalid
parents or rectangles return an empty surface. Rendering clips to the view,
including raised hands, leaves, and ears, and does not touch sibling widgets or
row padding. Use a larger view or smaller scale if clipping hides artwork.

For patterned backgrounds, redraw the background through your GUI before drawing
the character. Clear/restore the old region when moving or hiding it. Drawing
never clears the screen, advances animation, or presents pixels. A hidden instance
continues to advance when you call `update()`; call `setPaused(true)` to freeze it.

Create several `Mascot` objects for independent widgets. Give each its own state,
character, palette, and update calls. They may share one framebuffer through
separate subviews. See the complete [application example](../sdk/Mochi/examples/application.cpp).

## Send events from other tasks

All direct access to a `Mascot`, including reads and drawing, belongs to **one
owner task**. Functions in any application module can call it on that task. Other
tasks should send value commands for the owner to dispatch on its next frame.

The optional `CommandQueue<N>` supports exactly **one producer and one consumer**:

```cpp
mochi::CommandQueue<8> events;  // Keep alive for both tasks.

// Called only by the single producer task.
bool notifyWorkCompleted() {
    return events.push(mochi::Command::trigger(mochi::State::Happy, 1.5f));
}

// Called only by the UI/animation task, before update() and draw().
void dispatchCompanionEvents() {
    mochi::Command command;
    for (size_t n = 0; n < events.capacity && events.pop(command); ++n) {
        if (!companion.dispatch(command)) {
            // Invalid command: record or handle it according to your application.
        }
    }
}
```

The queue has `N` usable entries, performs no allocation, and returns `false`
when full without overwriting a command. The producer decides whether to drop,
coalesce, or retry. Bound the number drained per frame so a busy producer cannot
starve animation. Dispatch validation rejects invalid enum values and numbers.

| Factory | Effect when dispatched |
| --- | --- |
| `Command::setState(state)` | Change normal activity |
| `Command::trigger(state, seconds)` | Temporary reaction |
| `Command::cancelTrigger()` | End reaction |
| `Command::setCharacter(character)` | Change appearance |
| `Command::blink()` | Brief blink |
| `Command::setPaused(bool)` / `setSpeed(float)` | Playback controls |
| `Command::setSpeechLevel(float)` / `clearSpeechLevel()` | Mouth animation control |
| `Command::setVisible(bool)` | Drawing visibility |

For **multiple producers**, use your operating system's queue or serialize
producers externally. For an **interrupt handler**, use the operating system's
ISR-safe queue API and its priority/yield rules. The portable queue's
`always_lock_free` reports an atomic property of the target; it does not guarantee
that arbitrary interrupt usage is safe. You may include `<mochi/Mascot.h>` without
the optional queue on targets that do not provide suitable atomics.

The [FreeRTOS example](../examples/freertos_companion/README.md) uses a static RTOS
queue, accepts multiple task producers, and provides a separate ISR entry point.
It has two widgets and no selection menu. `Command` is a trivially copyable
in-process value; do not send its raw bytes as a network protocol or persist them
as a stable file format. Define and validate a protocol in the host application.

## Work with display drivers and GUI libraries

Mochi integrates at the RGB565 buffer boundary. An application using LVGL,
Arduino_GFX, TFT_eSPI, or another graphics library can provide a compatible
buffer and arrange its own draw/flush lifecycle. Dedicated adapters for all these
libraries are not included. Convert other pixel formats in your integration layer.

The host handles invalidation, byte swapping, bus locking, and DMA buffer
lifetime. Do not modify or release pixels while an asynchronous display transfer
still reads them. One application may use its normal screens alongside Mochi;
the SDK does not take over a display, start a task, or require a global singleton.

## Render with a small buffer

The [stripe example](../sdk/Mochi/examples/stripes.cpp) renders a 240 x 240 image
using eight rows at a time: **3,840 bytes** of pixel storage instead of 115,200.
Advance the animation once per frame, then draw the same pose in each stripe,
subtracting that stripe's vertical offset from the character's center.

Send each stripe before reusing its storage, following your driver's transfer
rules. This trades additional rasterization work for less pixel memory. Floating
point translation can change antialiased edges by one RGB565 channel step.
Pixel storage is not total memory use: the renderer also uses stack space for
geometry. Measure task stack high-water marks and performance on your target.
The core does not allocate, but display drivers and host task creation may do so.

## Install the SDK independently

Build and install the core using your target's CMake toolchain as needed:

```sh
cmake -S sdk/Mochi -B build/sdk -DCMAKE_BUILD_TYPE=Release
cmake --build build/sdk --config Release
cmake --install build/sdk --config Release --prefix /path/to/mochi-install
```

In a separate application's `CMakeLists.txt`:

```cmake
find_package(Mochi 0.3 CONFIG REQUIRED)
target_link_libraries(your_application PRIVATE Mochi::Mochi)
```

Configure that application with `-DCMAKE_PREFIX_PATH=/path/to/mochi-install`.
The installation contains headers, a static library, license, and relocatable
CMake package files. It does not depend on the original source checkout.
Pre-1.0 package compatibility is restricted to the same minor version.
Source integration with `add_subdirectory()` and local PlatformIO packages
remains supported; see the [main README](../README.md#use-the-sdk-in-your-project).

## Compatibility and validation

Existing `Animator`, `Renderer`, pose, palette, and gesture APIs remain available.
Use them directly when your application already separates animation and drawing.
`Mascot` combines those responsibilities for simpler integration. Existing 0.2
source consumers remain supported; rebuild applications when upgrading.

| Check | Coverage |
| --- | --- |
| Native tests with memory/undefined-behavior sanitizers | Reactions, clipping, subviews, command validation, all character/state pairs |
| Concurrent queue test | 100,000 ordered commands between two threads, bounded capacity and wraparound |
| Independent installed consumer | Relocates the installation and removes source/build folders before compiling another application |
| Embedded builds | Touchscreen playground and independent FreeRTOS application against the same SDK |
| Small-buffer rendering | Partial final stripes and RGB565 edge-rounding tolerance |

CI checks Linux/macOS and builds for ESP32-S3. Other boards and GUI integrations
need their own hardware validation. Keep drivers in separate adapters, add a
minimal consumer for new integrations, and document their memory and task
requirements. See [Porting](PORTING.md) and [Contributing](../CONTRIBUTING.md).
