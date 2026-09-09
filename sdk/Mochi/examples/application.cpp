#include <Mochi.h>
#include <array>
#include <cstdio>

// Two independent companions inside an existing application's framebuffer.
// No board driver, display menu, dedicated animation task or extra buffer needed.
namespace {
constexpr int width = 320, height = 200;
std::array<uint16_t, width * height> pixels{};
mochi::Mascot statusBuddy, notificationBuddy;
mochi::CommandQueue<8> notifications;

// Called by one application producer; may be a different task. Check false
// rather than silently losing an event if the bounded queue is full.
bool onNotification() {
  return notifications.push(mochi::Command::trigger(mochi::State::Waving, .75f));
}
}

int main(int argc, char** argv) {
  using namespace mochi;
  Surface screen{pixels.data(), pixels.size(), width, height};
  const Surface left = sub_surface(screen, 8, 32, 144, 144);
  const Surface right = sub_surface(screen, 168, 32, 144, 144);
  Renderer(screen).clear(rgb565(0x283341)); // Existing app background/header.
  statusBuddy.setCharacter(Character::Sprout);
  statusBuddy.setState(State::Thinking);
  notificationBuddy.setCharacter(Character::Peach);
  for (int frame = 0; frame < 45; ++frame) {
    if (frame == 15) {
      statusBuddy.setState(State::Idle); // Work completed; normal activity changes.
      statusBuddy.trigger(State::Happy, 1.5f);
      if (!onNotification()) return 1;
    }
    Command command;
    for (size_t count = 0; count < notifications.capacity && notifications.pop(command); ++count)
      if (!notificationBuddy.dispatch(command)) return 1;
    statusBuddy.update(1.f / 30); notificationBuddy.update(1.f / 30);
    Renderer(left).clear(); Renderer(right).clear();
    if (!statusBuddy.draw(left, 72, 76, .36f) || !notificationBuddy.draw(right, 72, 76, .36f)) return 1;
    // Host applications present their own screen here, at their chosen cadence.
  }
  const char* filename = argc > 1 ? argv[1] : "application.ppm";
  FILE* file = std::fopen(filename, "wb");
  if (!file) return 1;
  std::fprintf(file, "P6\n%d %d\n255\n", width, height);
  for (uint16_t pixel : pixels) {
    const unsigned char rgb[] = {static_cast<unsigned char>((pixel >> 11) * 255 / 31),
      static_cast<unsigned char>(((pixel >> 5) & 63) * 255 / 63), static_cast<unsigned char>((pixel & 31) * 255 / 31)};
    if (std::fwrite(rgb, 1, 3, file) != 3) { std::fclose(file); return 1; }
  }
  return std::fclose(file) == 0 ? 0 : 1;
}
