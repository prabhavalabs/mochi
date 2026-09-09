// Board initialization. Copyright (c) 2026 Nipun Theekshana.
// MIT license; see THIRD_PARTY_NOTICES.md.
#include "MochiWaveshare216.h"
#include <Wire.h>
#include <esp_heap_caps.h>
#define XPOWERS_CHIP_AXP2101
#include <TouchDrvCSTXXX.hpp>
#include <XPowersLib.h>

namespace mochi::waveshare216 {
namespace {
Arduino_ESP32QSPI bus(12, 38, 4, 5, 6, 7);
Arduino_CO5300 panel(&bus, 39, 0, kWidth, kHeight, 0, 0, 0, 0);
XPowersPMU power;
TouchDrvCST92xx touch;
BoardStatus board_status;
bool started = false, ready = false;
volatile bool touch_pending = false;
void IRAM_ATTR touch_interrupt() { touch_pending = true; }

class PsramCanvas : public Arduino_Canvas {
 public:
  PsramCanvas() : Arduino_Canvas(kWidth, kHeight, nullptr) {}
  bool allocate() {
    _framebuffer = static_cast<uint16_t*>(heap_caps_aligned_alloc(
        16, kWidth * kHeight * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    return _framebuffer != nullptr;
  }
} framebuffer;
}

const BoardStatus& status() { return board_status; }
Arduino_Canvas& canvas() { return framebuffer; }
bool begin() {
  if (started) return ready;
  started = true;
  pinMode(kUserButton, INPUT_PULLUP);
  pinMode(kBootButton, INPUT_PULLUP);
  Wire.begin(15, 14);
  Wire.setClock(400000);
  board_status.power = power.begin(Wire, AXP2101_SLAVE_ADDRESS, 15, 14);
  if (board_status.power) {
    power.enableBattDetection();
    power.enableBattVoltageMeasure();
    power.enableVbusVoltageMeasure();
  }
  touch.setPins(40, 11);
  board_status.touch = touch.begin(Wire, CST92XX_SLAVE_ADDRESS, 15, 14);
  Wire.setClock(400000);
  if (board_status.touch) {
    touch.setMaxCoordinates(kWidth, kHeight);
    touch.setSwapXY(true);
    touch.setMirrorXY(true, false);
    pinMode(11, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(11), touch_interrupt, FALLING);
    touch_pending = true;
  }
  board_status.display = panel.begin();
  if (!board_status.display) return false;
  // Portrait, USB socket at the bottom. Must match the touch transform.
  bus.writeC8D8(0x36, 0xA0);
  panel.fillScreen(RGB565_BLACK);
  panel.setBrightness(140);
  if (!psramFound() || !framebuffer.allocate()) {
    panel.fillScreen(RGB565_RED);
    Serial.println("Mochi: framebuffer allocation failed");
    return false;
  }
  framebuffer.setTextWrap(false);
  ready = true;
  return true;
}
bool present(int y, int height) {
  if (!ready || height <= 0 || y >= kHeight || int64_t(y) + height <= 0) return false;
  int end = int(int64_t(y) + height > kHeight ? kHeight : int64_t(y) + height);
  y = (y < 0 ? 0 : y) & ~1;
  end = (end + 1) & ~1;
  panel.draw16bitRGBBitmap(0, y, framebuffer.getFramebuffer() + y * kWidth, kWidth, end - y);
  return true;
}
bool setBrightness(uint8_t percent) {
  if (!ready || percent > 100) return false;
  panel.setBrightness(uint16_t(percent) * 255 / 100);
  return true;
}
TouchReport readTouch(int16_t& x, int16_t& y) {
  if (!ready || !board_status.touch) return TouchReport::None;
  noInterrupts();
  const bool pending = touch_pending;
  touch_pending = false;
  interrupts();
  if (!pending && digitalRead(11) != LOW) return TouchReport::None;
  int16_t xs[5]{}, ys[5]{};
  const auto count = touch.getPoint(xs, ys, touch.getSupportTouchPoint());
  if (!count) return TouchReport::Released;
  x = constrain(xs[0], 0, kWidth - 1);
  y = constrain(ys[0], 0, kHeight - 1);
  return TouchReport::Pressed;
}
}
