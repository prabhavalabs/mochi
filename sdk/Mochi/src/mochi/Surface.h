#pragma once
#include <cstddef>
#include <cstdint>

namespace mochi {
// Borrowed native-endian RGB565 pixels. Capacity and stride are pixel counts.
// Zero stride means tightly packed rows. The allocation must outlive every view.
struct Surface {
  uint16_t* pixels = nullptr;
  size_t capacity = 0;
  int width = 0, height = 0, stride = 0;
};
bool valid_surface(Surface surface);
// Exact, zero-copy rectangle with coordinates relative to the parent surface.
// Invalid parents or rectangles return an empty surface; no clamping/allocation.
Surface sub_surface(Surface parent, int x, int y, int width, int height);
}
