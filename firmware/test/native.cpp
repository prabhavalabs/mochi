#include <Mochi.h>
#include "../src/character_layout.h"
#include "../src/serial_line.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  using namespace mochi;
  playground::SerialLine input;
  auto feed = [&](const std::string& text) {
    auto result = playground::LineResult::Incomplete;
    for (char ch : text) result = input.push(ch);
    return result;
  };
  assert(feed("state 7\r\n") == playground::LineResult::Ready && !std::strcmp(input.line(), "state 7"));
  assert(feed(std::string(63, 'x') + "\n") == playground::LineResult::Ready);
  assert(feed(std::string(64, 'x') + "state 7\n") == playground::LineResult::Rejected);
  assert(feed(std::string("pause\0play\n", 11)) == playground::LineResult::Rejected);
  assert(feed("status\n") == playground::LineResult::Ready && !std::strcmp(input.line(), "status"));
  constexpr int w = 320, h = 320;
  constexpr uint16_t sentinel = 0x52ab;
  std::vector<uint16_t> guarded(w * h + 2, sentinel);
  auto* pixels = guarded.data() + 1;
  Renderer renderer({pixels, size_t(w*h), w, h});
  std::vector<uint16_t> screen(480*480,kBackground);
  Renderer screen_renderer({screen.data(), screen.size(), 480, 480});
  float min_blink = 1, max_mouth = 0, min_mouth = 1;
  for (int s = 0; s < kStateCount; ++s) {
    Pose blended;
    for (int frame = 0; frame < 360; ++frame) {
      const Pose pose = sample_pose(static_cast<State>(s), frame / 60.f);
      blend_pose(blended, pose, 1.f / 60);
      assert(std::isfinite(pose.y) && std::isfinite(pose.tilt));
      assert(pose.sx > .8f && pose.sx < 1.2f && pose.sy > .8f && pose.sy < 1.2f);
      assert(pose.eye_l >= 0 && pose.eye_l < 1.3f && pose.eye_r >= 0 && pose.eye_r < 1.3f);
      if (s == int(State::Blinking)) min_blink = std::min(min_blink, blended.eye_l);
      if (s == int(State::Speaking)) {
        max_mouth = std::max(max_mouth, pose.mouth_open);
        min_mouth = std::min(min_mouth, pose.mouth_open);
      }
      if (s == int(State::Sleeping)) assert(pose.eye_l < .1f && pose.eye_r < .1f);
      if (frame % 12 == 0) for (int ch=0;ch<kCharacterCount;ch++) {
        const auto character=static_cast<Character>(ch);
        renderer.setCharacter(character);
        screen_renderer.setCharacter(character);
        std::fill(pixels, pixels + w * h, kBackground);
        // Exercise clipping on all four edges as poses change.
        renderer.draw(blended, (frame % 120 < 60) ? 20 : 300, (frame % 90 < 30) ? 20 : 300, 1.1f);
        assert(guarded.front() == sentinel && guarded.back() == sentinel);
        std::fill(screen.begin(),screen.end(),kBackground);
        screen_renderer.draw(pose,240,207,playground::stage_scale(character));
        // Partial display updates must contain every pixel in every pose.
        assert(std::all_of(screen.begin(),screen.begin()+48*480,[](uint16_t p){return p==kBackground;}));
        assert(std::all_of(screen.begin()+348*480,screen.end(),[](uint16_t p){return p==kBackground;}));
      }
    }
  }
  assert(min_blink < .08f && max_mouth > .8f && min_mouth == 0);
  assert(blink_at(.12f, .12f) == 1 && blink_at(1, .12f) == 0);
  if (argc > 1) {
    std::vector<uint16_t> sheet(w * 5 * h * 2, kBackground);
    Renderer preview({sheet.data(), sheet.size(), w * 5, h * 2});
    for (int s = 0; s < kStateCount; ++s)
      preview.draw(sample_pose(static_cast<State>(s), s == int(State::Blinking) ? .6f : 1.f),
                   w * (s % 5) + 160, h * (s / 5) + 160, .79f);
    FILE* file = std::fopen(argv[1], "wb");
    assert(file);
    std::fprintf(file, "P6\n%d %d\n255\n", w * 5, h * 2);
    for (const uint16_t p : sheet) {
      const unsigned char rgb[] = {static_cast<unsigned char>((p >> 11) * 255 / 31),
        static_cast<unsigned char>(((p >> 5) & 63) * 255 / 63), static_cast<unsigned char>((p & 31) * 255 / 31)};
      std::fwrite(rgb, 1, 3, file);
    }
    std::fclose(file);
  }
  std::puts("PASS: serial line rejection/recovery, 4 characters x 10 states, blink/speech/sleep, clipped raster guards, partial display bounds");
}
