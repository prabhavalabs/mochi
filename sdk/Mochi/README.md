# Mochi

Portable, allocation-free C++17 animation for four little companions. Version 0.3.0.

## Embed a reusable companion

`Mascot` owns a character's appearance, playback, and temporary reactions. Your
application owns the display, timing, tasks, and screen layout:

```cpp
#include <Mochi.h>

mochi::Mascot companion;

void initialize() {
    companion.setCharacter(mochi::Character::Peach);
    companion.setState(mochi::State::Idle);
}
void onNotification() {  // Called on the companion's owner task.
    companion.trigger(mochi::State::Waving, 2.0f);
}
void render(mochi::Surface widget, float seconds) {
    companion.update(seconds);
    mochi::Renderer(widget).clear();
    companion.draw(widget, widget.width / 2.f, widget.height / 2.f, .32f);
}
```

`trigger()` returns to the base state when its duration expires; `setState()`
changes the base state and cancels an active reaction. Durations are animation
seconds, affected by speed and pause. Each instance has independent state.
`Mascot` never clears the screen, starts a task, or requires a selection menu.

Read the [application integration guide](../../docs/INTEGRATION.md) for the full
reaction contract, command dispatch, SPSC/FreeRTOS queues, clipped subviews,
multiple instances, small buffers, and GUI/driver boundaries. Direct calls are
owned by one task; other tasks post commands. The lower-level `Animator` and
`Renderer` APIs below remain available for custom integrations.

## Add to a project

For PlatformIO, point `lib_deps` at the package directory. Enable C++17 in the
application's build configuration:

```ini
lib_deps = symlink:///absolute/path/to/sdk/Mochi
build_unflags = -std=gnu++11
build_flags = -std=gnu++17
```

For CMake, add the source package and link the exported build target:

```cmake
add_subdirectory(path/to/Mochi)
target_link_libraries(my_application PRIVATE Mochi::Mochi)
```

Or install the compiled core with `cmake --install` and consume it independently:

```cmake
find_package(Mochi 0.3 CONFIG REQUIRED)
target_link_libraries(my_application PRIVATE Mochi::Mochi)
```

Set `CMAKE_PREFIX_PATH` to the installation prefix. See the
[install instructions](../../docs/INTEGRATION.md#install-the-sdk-independently).
The installed static library must match the application's architecture/toolchain.

No Arduino or external graphics library is needed for the core. `examples/offscreen.cpp`
is a complete consumer that renders a PPM image with only the SDK and C++ library.

## Lower-level animation and rendering

```cpp
#include <Mochi.h>
#include <array>

constexpr int width = 320, height = 320;
std::array<uint16_t, width * height> pixels{};
mochi::Animator buddy;
mochi::Renderer renderer({pixels.data(), pixels.size(), width, height});

void showThinking() {
    buddy.setState(mochi::State::Thinking);
}

void frame(float elapsedSeconds) {
    buddy.update(elapsedSeconds);
    renderer.clear();
    renderer.draw(buddy.pose(), width / 2.f, height / 2.f, 0.8f);
    // Present pixels with your display driver or image/window backend.
}
```

Call `update()` with **seconds**, not milliseconds. State transitions blend the
body and face, with faster eye movement for blinks. An app can decide when to
render, how to cap frame rate, whether to show controls and when to present.

## Characters

| Enum | Appearance |
| --- | --- |
| `Character::Mochi` | Original cream buddy (default) |
| `Character::Sprout` | Mint seedling with leaves |
| `Character::Peach` | Peach cat with ears and whiskers |
| `Character::Nimbus` | Soft blue cloud |

```cpp
renderer.setCharacter(mochi::Character::Sprout);
buddy.setState(mochi::State::Thinking);
// State, time, pause, speech level and speed remain independent of appearance.
renderer.draw(buddy.pose(), 160, 160, .8f);
```

`setCharacter()` returns false for invalid enum values, leaving the current
character and palette unchanged. A different character loads its default palette;
selecting the current character is a no-op that preserves custom colors. To
customize colors, call `setPalette()` after selecting the character. Call
`default_palette(character)` to obtain its colors. `Palette::accent` and `detail`
color leaves and ear details. A newly constructed renderer starts as Mochi;
applications that recreate a renderer each frame must reapply their selection.

`character()` reads the current selection. `kCharacterCount`, `character_name()`
and `character_detail()` support application pickers. `Character::Count` is a
sentinel. Name/detail helpers return `Unknown`/empty text for invalid values.
Each renderer has its own selection, so multiple characters can share a surface.
Geometry is drawn procedurally; no sprite sheets or extra framebuffers are needed.

`examples/cast.cpp` renders all four with the same pose. Every character works
with every state; SDK 0.1 callers that never select a character continue to draw
Mochi. Recompile consumers when upgrading, as `Palette` gained two fields.

## Playback API

| Call | Behavior |
| --- | --- |
| `setState(State)` | Select a state; reject invalid enum values. Selecting the current state is a no-op. |
| `next()` / `previous()` | Cycle through all ten states, with wraparound. |
| `restart()` | Restart current-state time and clear a manual blink without changing state, pause, speed or speech level. No state-change callback. |
| `blink()` | Queue a brief blink; playback must run for it to advance. Sleeping eyes remain closed. |
| `setPaused(bool)` | Freeze/resume automatic motion. Selecting another state while paused previews that pose immediately. |
| `setSpeed(float)` | Accept finite multipliers from 0.1 through 4.0; reject other values without changing speed. |
| `setSpeechLevel(float)` | Accept 0..1 mouth opening while Speaking; leaves other states unchanged. |
| `clearSpeechLevel()` | Restore the built-in speaking animation. |
| `update(float)` | Advance time and blend the pose; returns false when paused or given invalid/nonpositive time. |
| `pose()` | Read the current pose for rendering or copying into a custom pose. |
| `state()`, `elapsed()`, `speed()`, `paused()` | Read playback state. `elapsed()` measures animation seconds since the state changed. |
| `onStateChange(callback, context)` | Register a synchronous callback; use `nullptr` to remove it. |

The states are `Idle`, `Blinking`, `Happy`, `Listening`, `Thinking`, `Speaking`,
`Sleeping`, `Surprised`, `Sad` and `Waving`. `State::Count` is a sentinel, not a
playable state. `state_name()` and `state_detail()` supply the demo's captions.

Each `update()` consumes at most 100 ms of wall time before applying speed. This
prevents large jumps after a long pause in the host task; it does not catch up
missed time. Use a regular tick for smooth animation. State time uses double
precision and oscillation phases are reduced before float rendering, so small
motion remains visible during long-running states.

```cpp
buddy.onStateChange([](mochi::State from, mochi::State to, void* context) {
    // Update application state or enqueue an event here.
}, nullptr);
```

Animator callbacks run during `setState()` after the state changes. `Mascot` also
uses this callback when a reaction changes state or expires. Keep callbacks short and
do not re-enter playback methods from the callback. Use a single animation/UI
task for each animator and renderer; queue work from network, microphone or
interrupt callbacks. Separate animator instances have independent playback.

## Buffers and colors

`Surface` borrows an existing buffer. **Capacity and stride are in pixels.**
Pixels are native-endian `uint16_t` RGB565 values, not a byte-swapped wire buffer.
The buffer must remain alive for the renderer's lifetime. A stride of zero means
`width`; larger strides allow a view into a padded buffer. Row padding is never
cleared or drawn over.

`valid_surface(surface)` checks a layout without constructing a renderer.
`sub_surface(parent, x, y, width, height)` returns an exact zero-copy rectangle,
retaining the parent's stride; an invalid parent or rectangle returns an empty
surface. Coordinates used when drawing into a view are relative to that view.
See [widget integration](../../docs/INTEGRATION.md#put-a-character-in-an-existing-screen).

```cpp
mochi::Surface surface{pixels.data(), pixels.size(), width, height};
mochi::Palette colors;
colors.body = mochi::rgb565(0xB8E8D0);
colors.blush = mochi::rgb565(0xF5AFA0);
mochi::Renderer renderer(surface, colors);
```

`valid()` checks the surface layout and declared capacity. `clear()` and `draw()`
return false for an invalid surface. `draw()` rejects nonfinite or out-of-contract
pose/transform inputs without touching the buffer, and clips valid geometry to
the surface. The caller remains responsible for supplying the actual allocated
capacity. Dimensions up to 16,384, positive scales up to 100, and centers within
±1,000,000 pixels are supported. The built-in pose's natural extent is about
360 × 280 pixels at scale 1; choose placement/scale for your surface.

The renderer draws only the character. It does not clear the previous frame,
draw labels or synchronize a display. Clear or restore the relevant background
before rendering, then present it. `setPalette()` changes subsequent draws. Use
`clear(color)` to choose your background.

## Input

Feed the recognizer calibrated screen coordinates and a wrapping `uint32_t`
millisecond timestamp. Poll it even when there is no new touch report so it can
cancel contacts whose release report was lost.

```cpp
mochi::GestureRecognizer touch;

void handleTouch(mochi::TouchReport report, int16_t x, int16_t y, uint32_t nowMs) {
    const auto event = touch.update(report, x, y, nowMs);
    if (event.type == mochi::GestureType::Tap) buddy.blink();
    if (event.type == mochi::GestureType::SwipeLeft) buddy.next();
    if (event.type == mochi::GestureType::SwipeRight) buddy.previous();
}
```

Reports are `None`, `Pressed`, and `Released`; released/absent reports do not
require coordinates. Gesture events are `None`, `Tap`, `SwipeLeft`, `SwipeRight`
or `Cancelled`. Default swipe distance is greater than 55 pixels, tap travel is
less than 25 pixels, and missing releases cancel after 1,800 ms. Pass a
`GestureConfig` to choose thresholds for a different screen. A drag out and back
is not a tap. Timeout cancellation never activates a button. Multi-touch,
pinches and touch-controller calibration are outside this recognizer.

## Check the package

```sh
cmake -S . -B /tmp/mochi-build -DMOCHI_BUILD_TESTS=ON -DMOCHI_BUILD_EXAMPLE=ON
cmake --build /tmp/mochi-build
ctest --test-dir /tmp/mochi-build --output-on-failure
/tmp/mochi-build/mochi_offscreen /tmp/mochi.ppm
```
