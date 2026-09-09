---
slug: 'accessibility'
title: 'Privacy & accessibility'
group: 'Website'
order: 62
description: 'A local-first playground with keyboard controls, system preferences, and no microphone or account requirement.'
---

## What stays in your browser

The site stores only your chosen character and theme in local storage under `mochi.preferences`. Clear site data in your browser to reset them. No account, identifier, analytics script, or advertising tracker is added by this project. Cloudflare still handles normal web requests and may process standard hosting/security logs according to its service policies.

Search downloads a static index from this same site. The query is matched locally and is not sent to a search service. Fonts, icons, and artwork are served with the site. Copy buttons write only the selected example to your clipboard after you press them.

## Keyboard and screen readers

A skip link goes directly to page content. Navigation, theme selection, character and state groups, playback buttons, and search are keyboard accessible. Tab moves between controls, arrows operate toggle groups and sliders, and Enter/Space activates buttons. Search uses ⌘ K or Ctrl K, arrow keys, Enter, and Escape.

The stage announces the character's name and state caption without announcing every animation frame. The canvas has an accessible description. Decorative miniature faces are hidden from screen readers. Docs pages include landmark navigation and heading-based tables of contents.

## Motion and contrast

The theme defaults to your operating system. You can choose a fixed light/dark mode in the header. Focus indicators and primary text retain contrast in each character's palette.

Reduced motion starts the preview paused and reduces interface transitions. You can choose still poses or press Play to opt into animation. A change to the system motion preference is observed while the page is open. Off-screen and hidden-tab rendering is skipped.

## Audio and hardware

Speaking and Listening are visual animations. The browser does not request USB, microphone, camera, geolocation, or audio permissions. It cannot flash a board. Use the separate firmware tools documented in [Flashing & recovery](/docs/flashing/).

## Without JavaScript

Documentation text, code, links, and the site navigation remain in the static HTML. Search, character selection, copy buttons, and playground controls need JavaScript. The default preview and brand remain visible during loading; the SDK itself has no JavaScript requirement.

## Report a problem

Open a [GitHub issue](https://github.com/prabhavalabs/mochi/issues) with the affected page, browser, input method, and expected behavior. Avoid including private device details. Security reports follow the [private reporting guide](/docs/security/).
