---
slug: 'mascot-api'
title: 'Mascot API'
group: 'SDK'
order: 21
description: 'The high-level interface for reusable character instances, temporary reactions, playback, and drawing.'
---

## Ownership

Include `<Mochi.h>` and construct `mochi::Mascot`. Each instance owns its appearance, animator, reaction timer, and visibility. It owns no display, framebuffer, task, or selection UI. All direct access belongs to one owner task, including reads. Use commands across task boundaries.

## Appearance and state

| Call                                | Contract                                                                            |
| ----------------------------------- | ----------------------------------------------------------------------------------- |
| `setCharacter(Character)`           | Return false for an invalid value; a different selection loads its default palette. |
| `character()`                       | Read the current character.                                                         |
| `setPalette(Palette)` / `palette()` | Set or read this instance's colors.                                                 |
| `setState(State)`                   | Set the base activity and cancel any temporary reaction.                            |
| `state()` / `baseState()`           | Read the active animation and the state to return to.                               |
| `pose()`                            | Read the current blended pose.                                                      |
| `elapsed()`                         | Read animation seconds since the active state changed.                              |

## Temporary reactions

| Call                      | Contract                                                                        |
| ------------------------- | ------------------------------------------------------------------------------- |
| `trigger(State, seconds)` | Restart a temporary reaction; replace the current reaction without stacking it. |
| `cancelTrigger()`         | Return to the base state immediately.                                           |
| `triggered()`             | Read whether a reaction is active.                                              |
| `triggerRemaining()`      | Read remaining animation seconds.                                               |

Durations must be finite and in `(0, 86400]`. Invalid requests return false without changing state. Time follows pause and speed. Triggering the same state again restarts its reaction. Setting a base state cancels the timer even if that state is already showing.

## Playback

| Call                               | Contract                                                                            |
| ---------------------------------- | ----------------------------------------------------------------------------------- |
| `update(deltaSeconds)`             | Advance animation; reject invalid/nonpositive deltas and return false while paused. |
| `blink()`                          | Start a brief manual blink; it advances only while playing.                         |
| `setPaused(bool)` / `paused()`     | Freeze or resume playback and read its setting.                                     |
| `setSpeed(float)` / `speed()`      | Set or read a finite multiplier in `[0.1, 4.0]`.                                    |
| `setSpeechLevel(float)`            | Accept `[0, 1]` mouth opening for Speaking.                                         |
| `clearSpeechLevel()`               | Restore automatic mouth movement.                                                   |
| `onStateChange(callback, context)` | Set a synchronous state-change callback, or remove it with `nullptr`.               |

Each update consumes at most 0.1 wall seconds before applying speed. It prevents jumps after stalls rather than catching up missed time. Call regularly using a monotonic clock. Callbacks must remain short and must not re-enter playback methods.

`Mascot` does not expose the lower-level `Animator::next()`, `previous()`, or `restart()` methods. Your application can choose the next state and call `setState()`. For direct timeline control, use [Animator and Renderer](/docs/api-reference/#lower-level-animation-and-rendering).

## Visibility and rendering

`setVisible(bool)` changes drawing visibility; `visible()` reads it. Hiding a companion does not pause it. Continue updating hidden characters if reactions should expire, or pause them explicitly.

```cpp
buddy.draw(widget, widget.width / 2.f, widget.height / 2.f, 0.5f);
```

`draw(surface, x, y, scale)` uses coordinates relative to that surface. It never clears, presents, or advances time. It validates its arguments even when hidden. Read [Rendering & memory](/docs/rendering/) for capacity, stride, and bounds.

## Command dispatch

`dispatch(const Command&)` applies a value command on the owner task and returns success/failure. Palette changes remain direct owner-task operations. The [command guide](/docs/commands/) covers queues and producer limits.
