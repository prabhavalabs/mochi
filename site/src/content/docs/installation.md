---
slug: 'installation'
title: 'Installation'
group: 'Start here'
order: 2
description: 'Start with a local build, add the source package to an application, or install a relocatable CMake library.'
---

## Requirements

For the portable core, use CMake 3.16 or newer and a C++17 compiler: Clang, GCC, or Visual Studio C++ tools. Node.js is only needed to develop the website. Arduino and PlatformIO are only needed for the hardware examples.

```sh
git clone https://github.com/prabhavalabs/mochi.git
cd mochi
cmake -S sdk/Mochi -B build/sdk \
  -DMOCHI_BUILD_TESTS=ON -DMOCHI_BUILD_EXAMPLE=ON
cmake --build build/sdk --config Release
ctest --test-dir build/sdk -C Release --output-on-failure
```

## Render your first image

On macOS or Linux:

```sh
./build/sdk/mochi_offscreen build/mochi.ppm
./build/sdk/mochi_cast build/characters.ppm
```

Visual Studio generators place executables in `build/sdk/Release/` with an `.exe` suffix. Open the result in a PPM-compatible image viewer. The cast example renders all four characters.

## Add to an existing CMake project

```cmake
add_subdirectory(path/to/mochi/sdk/Mochi)
target_link_libraries(your_application PRIVATE Mochi::Mochi)
```

Or install the library to a prefix you choose:

```sh
cmake --install build/sdk --config Release --prefix build/mochi-install
```

In the consuming project:

```cmake
find_package(Mochi 0.3 CONFIG REQUIRED)
target_link_libraries(your_application PRIVATE Mochi::Mochi)
```

Pass `-DCMAKE_PREFIX_PATH=/path/to/mochi-install` when configuring the consumer. The compiled library must match your target architecture and toolchain; cross-compile it for embedded targets.

## PlatformIO applications

The repository contains separate core and adapter packages. Link the package directory, not the repository root:

```ini
lib_deps = symlink:///absolute/path/to/mochi/sdk/Mochi
build_unflags = -std=gnu++11
build_flags = -std=gnu++17
```

For the supported board, include the adapter too and use its complete [Waveshare configuration](/docs/waveshare/). The existing `firmware/` example already links both packages.

## Next step

[Create your first character](/docs/first-character/) or read the full [integration guide](/docs/integration/) for installation, ownership, and application patterns.
