---
slug: 'playground'
title: 'Playground guide'
group: 'Start here'
order: 4
description: 'Try every companion and emotion, adjust the pace, and make the portal feel like your own.'
---

## Choose a companion

Open the [playground](/#playground), then tap the companion name on the device screen. The **Meet your buddies** view lets you choose Mochi, Sprout, Peach, or Nimbus without leaving the simulated touchscreen. Your current mood stays selected when you switch companions. The name, personality, captions, miniature artwork, and subtle accent colors also update in the controls below the device and throughout the portal. The chosen character follows you into the documentation and survives a reload.

The browser gives each companion a small pacing difference: Mochi 1.0, Sprout 1.08, Peach 1.16, and Nimbus 0.8, multiplied by the speed slider. This is a website personality effect. The C++ core changes appearance independently of playback speed; set the speed yourself if you want the same effect in a device application.

## Explore a state

Tap **Choose a mood** inside the device, or use the buttons below it, to choose Idle, Blinking, Happy, Listening, Thinking, Speaking, Sleeping, Surprised, Sad, or Waving. All ten work with all four characters. The device and the page controls share the same selection, playback setting, tour, and pace.

- **Pause / Play** freezes or resumes movement. Selecting a new state while paused shows its still pose.
- **Blink once** adds a quick blink while playing. Sleeping keeps the eyes closed.
- **Restart** begins the current state again.
- **Speed** adjusts playback from 0.5× to 2.0×, before the character's personality multiplier. The touchscreen pace button cycles through 0.5×, 1×, and 1.5×.
- **Tour all states** advances every five seconds while the stage is visible and playback is running. Selecting a state ends the tour.

Tap the character to blink. Swipe horizontally at least 55 pixels to change states. The on-screen arrows and the device's `+` and `−` buttons step through the same list; the middle physical button pauses or plays the preview. Opening either touchscreen picker temporarily suspends the animation and tour, then resumes them when you choose an item, close the picker, or press Escape. It does not discard the paused state, speed, or tour setting.

Use the **3D / Front** control above the device to change its presentation. Pointer movement adds a small tilt in 3D view. The device returns to a front view while its controls have keyboard focus, while a touchscreen picker is open, and when reduced motion is enabled.

## Set your theme

The header's theme button offers Light, Dark, or Use system setting. System is the default, and follows changes to your operating system preference. Each character has an accessible light and dark accent palette. Your choice is saved locally in this browser, with no account.

## Reduced motion

With reduced motion enabled in your system, the preview starts paused and the device stays front-on. You can browse still poses or explicitly press Play to animate. Turning on reduced motion while the page is open pauses it again. Automatic rendering skips the stage when it is off-screen or the browser tab is hidden.

## What this preview does

The cream device is an interactive browser representation of the Waveshare ESP32-S3-Touch-AMOLED-2.16 playground. Its screen layout follows the firmware's stage, mood picker, and character picker, while the shell uses HTML and CSS perspective. The website draws the character locally using Canvas 2D and JavaScript. It does not flash or control your board. Speaking animates a mouth; no audio is played, no microphone is requested, and no speech recognition runs. For device installation, use the [flashing guide](/docs/flashing/).

Search documentation with the search box, or press **⌘ K** on macOS / **Ctrl K** elsewhere. Search works against a static index of the full guide; arrow keys choose a result, Enter opens it, and Escape closes the dialog.
