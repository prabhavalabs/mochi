#include <Mochi.h>
#include <algorithm>
#include <array>
#include <cstdio>

// A display can receive eight rows at a time, using 3,840 bytes of pixel storage.
// This host example writes the same stripes to an image instead of a panel.
int main(int argc, char** argv) {
  constexpr int width = 240, height = 240, stripeHeight = 8;
  std::array<uint16_t, width * stripeHeight> pixels{};
  mochi::Mascot buddy;
  buddy.setCharacter(mochi::Character::Nimbus);
  buddy.setState(mochi::State::Waving);
  for (int i = 0; i < 30; ++i) buddy.update(1.f / 30);
  const char* filename = argc > 1 ? argv[1] : "stripes.ppm";
  FILE* file = std::fopen(filename, "wb");
  if (!file) return 1;
  std::fprintf(file, "P6\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y += stripeHeight) {
    const int rows = std::min(stripeHeight, height - y);
    mochi::Surface stripe{pixels.data(), pixels.size(), width, rows};
    mochi::Renderer(stripe).clear();
    // Keep the pose unchanged until every stripe of this frame is presented.
    if (!buddy.draw(stripe, width / 2.f, height / 2.f - y, .6f)) { std::fclose(file); return 1; }
    for (int i = 0; i < width * rows; ++i) {
      const uint16_t pixel = pixels[i];
      const unsigned char rgb[] = {static_cast<unsigned char>((pixel >> 11) * 255 / 31),
        static_cast<unsigned char>(((pixel >> 5) & 63) * 255 / 63), static_cast<unsigned char>((pixel & 31) * 255 / 31)};
      if (std::fwrite(rgb, 1, 3, file) != 3) { std::fclose(file); return 1; }
    }
  }
  return std::fclose(file) == 0 ? 0 : 1;
}
