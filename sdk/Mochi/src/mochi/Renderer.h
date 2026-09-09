#pragma once
#include "mochi/Character.h"
#include "mochi/Motion.h"
#include "mochi/Surface.h"
#include <cstddef>
#include <cstdint>

namespace mochi {
constexpr uint16_t rgb565(unsigned rgb) {
  return static_cast<uint16_t>(((rgb>>8)&0xf800)|((rgb>>5)&0x7e0)|((rgb>>3)&0x1f));
}
constexpr uint16_t kBackground=rgb565(0x0B0D10);
struct Palette {
  uint16_t body = rgb565(0xF8F3E6), face = rgb565(0x121610);
  uint16_t blush = rgb565(0xF4BA9F), crease = rgb565(0xC6B79E);
  uint16_t accent = rgb565(0x65AD83), detail = rgb565(0xC4E8A5);
};
// Unknown values return Mochi's palette; Renderer rejects invalid selections.
Palette default_palette(Character character);
struct Point { float x, y; };
class Renderer {
 public:
  explicit Renderer(Surface surface, Palette palette = {});
  bool valid() const { return valid_; }
  // A different character loads its default palette. Selecting the current
  // character is a no-op, preserving custom colors. Invalid values change nothing.
  bool setCharacter(Character character);
  Character character() const { return character_; }
  void setPalette(Palette palette) { palette_ = palette; }
  bool clear(uint16_t color = kBackground);
  // Does not clear or present the buffer. Invalid inputs leave it untouched.
  bool draw(const Pose& pose, float x, float y, float scale);
 private:
  Point transform(Point p) const;
  void pixel(int x,int y,uint16_t color,float alpha=1);
  void polygon(const Point* points, size_t count, uint16_t color);
  void ellipse(float x,float y,float rx,float ry,float angle,uint16_t color);
  void stroke(const Point* points,size_t count,float width,uint16_t color);
  void curve(Point a,Point b,Point c,Point d,float width,uint16_t color);
  void hand(float x,float y,float angle,int side);
  void eye(float x,float openness,float happy);
  void body();
  void cat_ear(int side);
  uint16_t* pixels_;
  int width_,height_,stride_;
  bool valid_ = false;
  Palette palette_;
  Character character_ = Character::Mochi;
  Pose pose_{};
  float cx_=0,cy_=0,scale_=1,cos_=1,sin_=0;
};
}
