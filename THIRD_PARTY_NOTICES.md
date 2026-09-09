# Third-party notices

Mochi's original source, documentation and included character artwork use the
[MIT License](LICENSE). A package's license does not replace the licenses of the
libraries or tools it depends on.

## Board initialization

The board adapter preserves the copyright and MIT permission notice for reused
initialization code in
[`sdk/MochiWaveshare216/THIRD_PARTY_NOTICES.md`](sdk/MochiWaveshare216/THIRD_PARTY_NOTICES.md).
Keep that notice with redistributed copies of the adapter.

## Separately downloaded dependencies

These sources are downloaded by PlatformIO rather than vendored in the repository:

| Dependency | Pinned version | Upstream license information |
| --- | --- | --- |
| Arduino_GFX | 1.6.7 | [BSD-style license and retained notices](https://github.com/moononournation/Arduino_GFX/blob/v1.6.7/license.txt) |
| SensorLib | 0.2.6 | [MIT](https://github.com/lewisxhe/SensorLib) |
| XPowersLib | 0.2.7 | [MIT](https://github.com/lewisxhe/XPowersLib) |
| Arduino-ESP32 | 3.3.8 | [LGPL-2.1-or-later package metadata](https://github.com/espressif/arduino-esp32/blob/3.3.8/package.json); individual components may carry additional notices |
| pioarduino platform | 55.03.38-1 | [Platform sources and notices](https://github.com/pioarduino/platform-espressif32) |

Consult the license files in the exact versions used by your build. If distributing
firmware binaries, preserve required notices and meet applicable source/relinking
requirements for included libraries. The MIT license on Mochi does not relicense
the Arduino framework or its components.

## Website

The Astro portal uses Astro, React, Tailwind CSS, Radix UI, shadcn/ui component
source, cmdk, and their dependencies under their respective upstream licenses.
The lockfile records exact installed versions. The shadcn/ui MIT notice is retained in
`site/public/licenses/shadcn.txt`. Lucide icons use the ISC license.
DM Sans and Lora font files are bundled through Fontsource under the SIL Open
Font License 1.1; the retained font notices are in `site/public/licenses/` and
ship with the static site. The original `web/` demo still uses platform fonts.

The screenshots in `docs/images/` are rendered from the project's own character
code and contain no device identifiers. Mochi character artwork remains covered
by the project's MIT license.
