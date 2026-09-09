# Contributing to Mochi

Contributions are welcome: bug reports, documentation, accessibility improvements,
characters, tests and board adapters. You do not need hardware to work on the
portable SDK or browser preview.

## Start a contribution

1. Search existing issues and pull requests to avoid duplicating work.
2. For a new public API, renderer redesign or board family, open an issue explaining
   the use case and intended behavior before writing a large change.
3. Fork the repository, create a focused branch, and keep unrelated changes separate.
4. Add or update documentation and tests appropriate to the behavior being changed.
5. Open a pull request explaining the problem, solution and validation results.

Small fixes and documentation improvements can go straight to a pull request.
Maintainers review public API and dependency changes before merging. Do not include
private credentials, device identifiers, firmware backups or unrelated files.

## Local setup and checks

Follow the [README](README.md) for installation. From the project root:

```sh
npm run check
npm run test:web
python3 scripts/test-sdk.py
pio run --project-dir firmware
```

The sanitizer runner is intended for Clang on macOS/Linux. You can also build the
core with CMake and run CTest, including on Windows. A change that only touches
documentation does not need a hardware flash. Report which checks you actually ran.

## Code conventions

- Keep the core C++17 and independent of Arduino, operating systems and drivers.
- Keep animation/rendering allocation-free. The application owns the pixel buffer.
- Validate public inputs and preserve the documented failure behavior.
- Put board-specific GPIO, display and touch code in an adapter package.
- Keep the browser reference dependency-free unless a concrete need justifies a change.
- Follow nearby formatting and naming; avoid unrelated formatting changes.
- Explain buffer ownership, units and task/thread constraints in API documentation.
- Include license information for any new code, artwork, fonts or dependencies.

## Testing characters and board changes

For a new character, check every animation state, palette switching, tiny previews,
clipping, partial-update bounds, and a monochrome silhouette. Reuse the shared pose
contract so playback remains independent of appearance.

For firmware or adapter changes, record the board model and dependency versions,
then verify what your change affects:

- Boot with USB at the bottom; check display orientation and colors.
- Read `status` and confirm expected display, touch, power and PSRAM detection.
- Tap each picker, select all characters and moods, and swipe in both directions.
- Check blink, pause/play, pace, tour, and physical next/previous buttons.
- Watch for rendering trails, clipping, crashes and unexpected memory growth.
- Test reconnect/reboot and confirm the console remains usable.

If hardware is unavailable, say so in the pull request. A maintainer can help
validate it before merging. Automated tests cannot certify physical input or power behavior.

## Bug reports and security

For bugs, include a minimal reproduction, expected/actual behavior, commit or
version, operating system and relevant board model. Share only the smallest
necessary log excerpt and remove device identifiers or private data.

Report suspected vulnerabilities privately using [SECURITY.md](SECURITY.md).
Do not attach full device flash dumps to public issues.

## Licensing and conduct

By submitting a contribution, you agree to license it under the project's MIT
License, unless a separate license is explicitly documented and accepted. You
must have the rights to contribute the material. Preserve existing copyright
and third-party notices. No contributor license agreement is currently required.

Please follow the [Code of Conduct](CODE_OF_CONDUCT.md).
