#include <Mochi.h>
#include <array>
#include <cstdio>

int main(int argc, char** argv) {
  constexpr int width = 320, height = 320;
  std::array<uint16_t, width * height> pixels{};
  mochi::Animator buddy;
  mochi::Renderer renderer({pixels.data(), pixels.size(), width, height});
  buddy.setState(mochi::State::Waving);
  for (int frame = 0; frame < 30; ++frame) buddy.update(1.f / 30);
  if (!renderer.clear() || !renderer.draw(buddy.pose(), 160, 160, .8f)) return 1;

  const char* filename = argc > 1 ? argv[1] : "mochi.ppm";
  FILE* file = std::fopen(filename, "wb");
  if (!file) { std::perror(filename); return 1; }
  std::fprintf(file, "P6\n%d %d\n255\n", width, height);
  for (const uint16_t pixel : pixels) {
    const unsigned char rgb[] = {static_cast<unsigned char>((pixel >> 11) * 255 / 31),
      static_cast<unsigned char>(((pixel >> 5) & 63) * 255 / 63), static_cast<unsigned char>((pixel & 31) * 255 / 31)};
    if (std::fwrite(rgb, 1, 3, file) != 3) { std::fclose(file); return 1; }
  }
  if (std::fclose(file) != 0) return 1;
  std::printf("Rendered Mochi SDK %s to %s\n", mochi::kVersion, filename);
}
