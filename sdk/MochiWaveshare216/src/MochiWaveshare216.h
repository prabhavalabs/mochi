#pragma once
#include <Arduino_GFX_Library.h>
#include <mochi/Input.h>

namespace mochi::waveshare216 {
constexpr int kWidth = 480, kHeight = 480;
constexpr int kUserButton = 18, kBootButton = 0;
struct BoardStatus { bool display = false, touch = false, power = false; };
const BoardStatus& status();
// One board instance per MCU. Repeated begin calls return the first result.
bool begin();
// Use only after begin succeeds. The adapter owns the framebuffer.
Arduino_Canvas& canvas();
// Full-width rows; clips to screen and expands to the panel's even alignment.
bool present(int y = 0, int height = kHeight);
bool setBrightness(uint8_t percent);
TouchReport readTouch(int16_t& x, int16_t& y);
}
