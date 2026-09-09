#include "mochi/Surface.h"

namespace mochi {
bool valid_surface(Surface surface) {
  const int stride = surface.stride == 0 ? surface.width : surface.stride;
  if (!surface.pixels || surface.width <= 0 || surface.height <= 0 ||
      surface.width > 16384 || surface.height > 16384 || stride < surface.width) return false;
  const uint64_t required = uint64_t(surface.height - 1) * uint64_t(stride) + uint64_t(surface.width);
  return required <= surface.capacity;
}
Surface sub_surface(Surface parent, int x, int y, int width, int height) {
  if (!valid_surface(parent) || x < 0 || y < 0 || width <= 0 || height <= 0 ||
      x >= parent.width || y >= parent.height ||
      width > parent.width - x || height > parent.height - y) return {};
  const int stride = parent.stride == 0 ? parent.width : parent.stride;
  const size_t offset = size_t(y) * size_t(stride) + size_t(x);
  return {parent.pixels + offset, parent.capacity - offset, width, height, stride};
}
}
