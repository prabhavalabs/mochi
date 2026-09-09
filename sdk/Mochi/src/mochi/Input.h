#pragma once
#include <cstdint>

namespace mochi {
enum class TouchReport : int8_t { None = -1, Released = 0, Pressed = 1 };
enum class GestureType : uint8_t { None, Tap, SwipeLeft, SwipeRight, Cancelled };
struct Gesture { GestureType type = GestureType::None; int16_t x = 0, y = 0; };
struct GestureConfig {
  uint16_t swipe_distance = 55;
  uint16_t tap_slop = 25;
  uint32_t release_timeout_ms = 1800;
};
// Coordinates are already rotated/calibrated display pixels. Timestamps use
// the same wrapping uint32_t millisecond clock on every call.
class GestureRecognizer {
 public:
  explicit GestureRecognizer(GestureConfig config = {}) : config_(config) {}
  Gesture update(TouchReport report, int16_t x, int16_t y, uint32_t now_ms);
  bool pressed() const { return pressed_; }
  void cancel() { pressed_ = false; }
 private:
  GestureConfig config_;
  bool pressed_ = false;
  int16_t start_x_ = 0, start_y_ = 0, x_ = 0, y_ = 0;
  int32_t max_dx_ = 0, max_dy_ = 0;
  uint32_t last_report_ = 0;
};
}
