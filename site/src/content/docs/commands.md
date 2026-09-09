---
slug: 'commands'
title: 'Commands & tasks'
group: 'SDK'
order: 33
description: 'Trigger a companion from other parts of your application while keeping animation state owned by one task.'
---

## Use values across boundaries

A `Command` is a trivially copyable value with no retained object pointers. Producers create commands; the character's owner task dispatches them.

```cpp
const auto event = mochi::Command::trigger(mochi::State::Happy, 1.5f);
// Transport event through your application's queue.
// On the UI task:
buddy.dispatch(event);
```

Factories cover `setState`, `trigger`, `cancelTrigger`, `setCharacter`, `blink`, `setPaused`, `setSpeed`, `setSpeechLevel`, `clearSpeechLevel`, and `setVisible`.

## One producer, one consumer

The optional `mochi::CommandQueue<N>` is a bounded single-producer/single-consumer queue. Exactly one task pushes and exactly one task pops. Check the return value: a full queue rejects a push. Choose whether your application drops, retries, or coalesces events. Do not busy-wait on the UI thread.

Read the complete [queue example](/docs/integration/) for the API and ownership rules. A queue is optional: callbacks already on the owner task can call `trigger()` directly.

## Multiple producers and interrupts

Use your RTOS's queue when multiple workers send events. Use the documented ISR-safe send operation from an interrupt and the appropriate wake/yield handling for your RTOS. The portable SPSC queue is not a general ISR or multi-producer transport.

The [FreeRTOS companion example](/docs/freertos/) demonstrates a worker posting reactions while a UI task updates and draws two independent widgets.

## Ordering and lifetime

Commands apply in the order the consumer dispatches them. A new trigger replaces the previous reaction. A `setState` cancels an active reaction. Queue overflow and task shutdown belong to the application; drain or discard pending commands before destroying the target. Keep callbacks short and never call playback methods recursively from a state-change callback.
