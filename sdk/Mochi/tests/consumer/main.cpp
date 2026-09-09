#include <Mochi.h>
#include <array>

int main() {
  std::array<uint16_t, 64 * 64> pixels{};
  mochi::Mascot companion;
  if (!companion.setCharacter(mochi::Character::Nimbus)) return 1;
  if (!companion.dispatch(mochi::Command::trigger(mochi::State::Happy, .1f))) return 1;
  companion.update(.1f);
  if (companion.state() != mochi::State::Idle) return 1;
  return companion.draw({pixels.data(), pixels.size(), 64, 64}, 32, 32, .16f) ? 0 : 1;
}
