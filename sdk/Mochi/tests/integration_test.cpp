#include <Mochi.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

using namespace mochi;
void testReactions() {
  Mascot buddy, other;
  int callbacks = 0;
  buddy.onStateChange([](State, State, void* p) { ++*static_cast<int*>(p); }, &callbacks);
  assert(buddy.setState(State::Listening));
  assert(buddy.trigger(State::Happy, .25f));
  assert(buddy.baseState() == State::Listening && buddy.state() == State::Happy && callbacks == 2);
  buddy.update(.1f);
  const double remaining = buddy.triggerRemaining();
  buddy.setPaused(true);
  assert(!buddy.update(.1f) && buddy.triggerRemaining() == remaining);
  buddy.setVisible(false);
  assert(buddy.triggered());
  assert(buddy.setCharacter(Character::Peach));
  assert(buddy.state() == State::Happy && buddy.paused() && !buddy.visible());
  buddy.setPaused(false);
  buddy.setSpeed(2);
  assert(buddy.update(10)); // Same 100 ms cap as Animator, then 2x speed.
  assert(!buddy.triggered() && buddy.state() == State::Listening && callbacks == 3);
  assert(other.state() == State::Idle && other.character() == Character::Mochi && other.visible());

  buddy.trigger(State::Waving, 2);
  buddy.update(.1f);
  const int before = callbacks;
  buddy.trigger(State::Waving, 3);
  assert(buddy.elapsed() == 0 && buddy.triggerRemaining() == 3 && callbacks == before);
  buddy.trigger(State::Surprised, .1f);
  buddy.update(.1f);
  assert(buddy.state() == State::Listening); // Retriggering did not stack Waving.
  buddy.trigger(State::Happy, 1);
  buddy.setState(State::Thinking);
  assert(!buddy.triggered() && buddy.baseState() == State::Thinking);
  buddy.trigger(State::Sad, 1);
  buddy.cancelTrigger();
  assert(buddy.state() == State::Thinking && buddy.triggerRemaining() == 0);

  buddy.trigger(State::Happy, 1);
  const float nan = std::numeric_limits<float>::quiet_NaN();
  for (float duration : {nan, -1.f, 0.f, std::numeric_limits<float>::infinity(), kMaxTriggerSeconds + 1})
    assert(!buddy.trigger(State::Sad, duration));
  assert(!buddy.trigger(State::Count, 1) && !buddy.setState(State::Count));
  assert(!buddy.setCharacter(Character::Count) && !buddy.setSpeed(0));
  assert(!buddy.update(nan) && !buddy.update(-1));
  assert(buddy.state() == State::Happy && buddy.baseState() == State::Thinking && buddy.triggerRemaining() == 1);
  assert(buddy.trigger(State::Waving, kMaxTriggerSeconds));
  buddy.update(.1f);
  assert(buddy.triggerRemaining() < kMaxTriggerSeconds);
}

void testComposition() {
  constexpr int width = 200, height = 120, stride = 211;
  constexpr uint16_t sentinel = 0x534a;
  std::vector<uint16_t> pixels(stride * height + 2, sentinel);
  Surface screen{pixels.data() + 1, size_t(stride * height), width, height, stride};
  Surface left = sub_surface(screen, 10, 20, 80, 80);
  Surface right = sub_surface(screen, 110, 20, 80, 80);
  assert(valid_surface(left) && valid_surface(right) && left.stride == stride);
  assert(left.pixels == screen.pixels + 20 * stride + 10);
  assert(valid_surface(sub_surface(left, 79, 79, 1, 1)));
  for (Surface invalid : {sub_surface({}, 0, 0, 1, 1), sub_surface(screen, -1, 0, 1, 1),
       sub_surface(screen, 0, 0, 0, 1), sub_surface(screen, width, 0, 1, 1),
       sub_surface(left, 70, 0, 11, 10), sub_surface(screen, 0, 0, INT32_MAX, 1)})
    assert(!valid_surface(invalid));
  Mascot first, second;
  first.setCharacter(Character::Sprout);
  second.setCharacter(Character::Peach);
  second.setState(State::Sleeping);
  Renderer(left).clear(); Renderer(right).clear();
  assert(first.draw(left, 40, 40, .7f)); // Deliberately oversized, clipped to widget.
  assert(second.draw(right, 40, 40, .2f));
  for (int y = 0; y < height; ++y) for (int x = 0; x < stride; ++x) {
    const bool widget = y >= 20 && y < 100 && ((x >= 10 && x < 90) || (x >= 110 && x < 190));
    if (!widget) assert(screen.pixels[y * stride + x] == sentinel);
  }
  assert(pixels.front() == sentinel && pixels.back() == sentinel);
  const auto before = pixels;
  first.setVisible(false);
  assert(first.draw(left, 40, 40, 1) && pixels == before);
  assert(!first.draw({}, 0, 0, 1) && !first.draw(left, 0, 0, 0));
  first.setVisible(true);
  assert(!first.draw(left, 0, std::numeric_limits<float>::infinity(), 1) && pixels == before);
  Palette colors = first.palette(); colors.body = rgb565(0xDD66AA);
  first.setPalette(colors);
  first.setCharacter(Character::Sprout);
  assert(first.palette().body == colors.body);
  Renderer(left).clear(); first.draw(left, 40, 40, .2f);
  assert(std::count(pixels.begin(), pixels.end(), colors.body) > 1000);
  first.setCharacter(Character::Nimbus);
  assert(first.palette().body == default_palette(Character::Nimbus).body);
}

void testCommands() {
  Mascot buddy;
  assert(buddy.dispatch(Command::setCharacter(Character::Nimbus)));
  assert(buddy.dispatch(Command::setState(State::Thinking)));
  assert(buddy.dispatch(Command::trigger(State::Waving, 1)));
  assert(buddy.triggered() && buddy.baseState() == State::Thinking);
  assert(buddy.dispatch(Command::cancelTrigger()) && !buddy.triggered());
  assert(buddy.dispatch(Command::setPaused(true)) && buddy.paused());
  assert(buddy.dispatch(Command::setSpeed(.5f)) && buddy.speed() == .5f);
  assert(buddy.dispatch(Command::setVisible(false)) && !buddy.visible());
  assert(buddy.dispatch(Command::setState(State::Speaking)));
  assert(buddy.dispatch(Command::setSpeechLevel(1)));
  buddy.setPaused(false);
  for (int i = 0; i < 50; ++i) buddy.update(.05f);
  assert(buddy.pose().mouth_open > .999f);
  assert(buddy.dispatch(Command::clearSpeechLevel()));
  assert(buddy.dispatch(Command::blink()));
  assert(!buddy.dispatch(Command::setSpeed(10)) && buddy.speed() == .5f);
  Command invalid; invalid.type = static_cast<CommandType>(255);
  assert(!buddy.dispatch(invalid));
}

void testStripes() {
  constexpr int width = 120, height = 96, rows = 7;
  std::vector<uint16_t> full(width * height), stripe(width * rows);
  Mascot buddy;
  buddy.setState(State::Waving);
  for (int i = 0; i < 30; ++i) buddy.update(1.f / 30);
  for (int character = 0; character < kCharacterCount; ++character) {
    buddy.setCharacter(static_cast<Character>(character));
    Surface frame{full.data(), full.size(), width, height};
    Renderer(frame).clear(); buddy.draw(frame, 60, 48, .35f);
    for (int y = 0; y < height; y += rows) {
      const int count = std::min(rows, height - y);
      Surface tile{stripe.data(), stripe.size(), width, count};
      Renderer(tile).clear(); buddy.draw(tile, 60, 48.f - y, .35f);
      for (int i = 0; i < width * count; ++i) {
        const uint16_t a = full[y * width + i], b = stripe[i];
        // Translating float coordinates can round an antialiased boundary by one
        // RGB565 step; the geometry and all solid pixels must still agree.
        assert(std::abs(int(a >> 11) - int(b >> 11)) <= 1);
        assert(std::abs(int((a >> 5) & 63) - int((b >> 5) & 63)) <= 1);
        assert(std::abs(int(a & 31) - int(b & 31)) <= 1);
      }
    }
  }
}

int main() { testReactions(); testComposition(); testCommands(); testStripes(); }
