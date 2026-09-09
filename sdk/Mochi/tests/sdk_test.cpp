#include <Mochi.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

using namespace mochi;
bool near(float a, float b) { return std::fabs(a-b) < .0001f; }

void testAnimator() {
  struct Events { int count = 0; State from{}, to{}; } events;
  Animator buddy, other;
  buddy.onStateChange([](State from, State to, void* context) {
    auto& e = *static_cast<Events*>(context); ++e.count; e.from = from; e.to = to;
  }, &events);
  assert(buddy.setState(State::Waving));
  assert(events.count == 1 && events.from == State::Idle && events.to == State::Waving);
  assert(buddy.setState(State::Waving) && events.count == 1);
  assert(!buddy.setState(State::Count) && !buddy.setState(static_cast<State>(255)));
  assert(events.count == 1 && other.state() == State::Idle);
  const float start_hand = buddy.pose().left_y;
  assert(buddy.update(.02f));
  assert(buddy.pose().left_y < start_hand && buddy.pose().left_y > -32);
  assert(other.elapsed() == 0);

  buddy.setPaused(true);
  const float frozen = buddy.elapsed(), frozen_hand = buddy.pose().left_y;
  assert(!buddy.update(.1f) && buddy.elapsed() == frozen && buddy.pose().left_y == frozen_hand);
  assert(buddy.setState(State::Sleeping));
  assert(buddy.pose().eye_l < .1f && buddy.elapsed() == 0);
  buddy.setPaused(false);
  assert(buddy.setSpeed(2));
  assert(buddy.update(10) && near(buddy.elapsed(), .2f)); // Stall cap, then speed.
  const float before = buddy.elapsed();
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float inf = std::numeric_limits<float>::infinity();
  assert(!buddy.update(nan) && !buddy.update(inf) && !buddy.update(-1) && !buddy.update(0));
  assert(buddy.elapsed() == before);
  assert(!buddy.setSpeed(nan) && !buddy.setSpeed(0) && !buddy.setSpeed(5) && buddy.speed() == 2);

  buddy.setPaused(true);
  buddy.setState(State::Idle);
  buddy.previous(); assert(buddy.state() == State::Waving);
  buddy.next(); assert(buddy.state() == State::Idle);
  buddy.blink(); // Queued while paused, rather than advancing a frozen pose.
  assert(buddy.pose().eye_l == 1);
  buddy.setSpeed(1); buddy.setPaused(false);
  float min_eye = 1;
  for (int i = 0; i < 30; ++i) { buddy.update(.02f); min_eye = std::min(min_eye, buddy.pose().eye_l); }
  assert(min_eye < .08f && buddy.pose().eye_l > .99f);

  buddy.setState(State::Speaking);
  assert(buddy.setSpeechLevel(0));
  for (int i = 0; i < 20; ++i) buddy.update(.05f);
  assert(buddy.pose().mouth_open < .001f);
  assert(buddy.setSpeechLevel(1));
  for (int i = 0; i < 20; ++i) buddy.update(.05f);
  assert(buddy.pose().mouth_open > .999f);
  assert(!buddy.setSpeechLevel(nan) && !buddy.setSpeechLevel(-1) && !buddy.setSpeechLevel(2));
  buddy.clearSpeechLevel();
  float min_mouth = 1, max_mouth = 0;
  for (int i = 0; i < 150; ++i) {
    buddy.update(.03f); min_mouth = std::min(min_mouth, buddy.pose().mouth_open);
    max_mouth = std::max(max_mouth, buddy.pose().mouth_open);
  }
  assert(min_mouth < .1f && max_mouth > .5f);
  const Pose late = sample_pose(State::Idle, 10000000.0);
  const Pose later = sample_pose(State::Idle, 10000000.02);
  assert(std::fabs(late.y-later.y) + std::fabs(late.gaze_x-later.gaze_x) > .0001f);
  buddy.onStateChange(nullptr);
  const int count = events.count;
  buddy.next(); assert(events.count == count);
}

void testGestures() {
  GestureRecognizer touch;
  assert(touch.update(TouchReport::Released, 0, 0, 0).type == GestureType::None);
  touch.update(TouchReport::Pressed, 200, 200, 10);
  touch.update(TouchReport::None, 0, 0, 20);
  const auto tap = touch.update(TouchReport::Released, 0, 0, 30);
  assert(tap.type == GestureType::Tap && tap.x == 200 && tap.y == 200 && !touch.pressed());
  touch.update(TouchReport::Pressed, 250, 200, 100);
  touch.update(TouchReport::Pressed, 100, 210, 120);
  assert(touch.update(TouchReport::Released, 0, 0, 130).type == GestureType::SwipeLeft);
  touch.update(TouchReport::Pressed, 100, 200, 200);
  touch.update(TouchReport::Pressed, 250, 210, 220);
  assert(touch.update(TouchReport::Released, 0, 0, 230).type == GestureType::SwipeRight);
  // A drag out and back is not a tap.
  touch.update(TouchReport::Pressed, 100, 100, 300);
  touch.update(TouchReport::Pressed, 150, 150, 320);
  touch.update(TouchReport::Pressed, 100, 100, 340);
  assert(touch.update(TouchReport::Released, 0, 0, 360).type == GestureType::Cancelled);
  // Losing a release report must never synthesize a button activation.
  touch.update(TouchReport::Pressed, 100, 100, 400);
  assert(touch.update(TouchReport::None, 0, 0, 2201).type == GestureType::Cancelled);
  assert(!touch.pressed());
  // A millisecond timer wrap is a normal short touch, not a timeout.
  touch.update(TouchReport::Pressed, 100, 100, UINT32_MAX-10);
  assert(touch.update(TouchReport::Released, 0, 0, 20).type == GestureType::Tap);
  touch.update(TouchReport::Pressed, 100, 100, 50); touch.cancel();
  assert(touch.update(TouchReport::Released, 0, 0, 60).type == GestureType::None);
}

void testSurface() {
  constexpr int w = 128, h = 128, stride = 139;
  constexpr uint16_t sentinel = 0xa531;
  std::vector<uint16_t> storage(stride*h+2, sentinel);
  auto* pixels = storage.data()+1;
  Palette palette; palette.body = rgb565(0x55DD88);
  Renderer renderer({pixels, size_t(stride*h), w, h, stride}, palette);
  assert(renderer.valid() && renderer.clear() && renderer.draw(Pose{}, 64, 64, .35f));
  assert(std::count(storage.begin(), storage.end(), palette.body) > 1000);
  assert(storage.front() == sentinel && storage.back() == sentinel);
  for (int y=0;y<h;y++) for (int x=w;x<stride;x++) assert(pixels[y*stride+x] == sentinel);

  const auto before = storage;
  Pose broken; broken.tilt = std::numeric_limits<float>::quiet_NaN();
  assert(!renderer.draw(broken, 64, 64, 1));
  assert(!renderer.draw(Pose{}, 64, 64, 0));
  assert(!renderer.draw(Pose{}, std::numeric_limits<float>::infinity(), 64, 1));
  assert(storage == before);
  Renderer empty({}); assert(!empty.valid() && !empty.clear() && !empty.draw(Pose{}, 0, 0, 1));
  Renderer short_buffer({pixels, 3, w, h}); assert(!short_buffer.valid() && !short_buffer.clear());
  Renderer bad_stride({pixels, storage.size()-2, w, h, w-1}); assert(!bad_stride.valid());
  Renderer huge({pixels, 10, 1000000, 1000000}); assert(!huge.valid());
}

void testCharacters() {
  constexpr int w=320, h=320;
  std::vector<uint16_t> pixels(w*h);
  Renderer renderer({pixels.data(), pixels.size(), w, h});
  Renderer other({pixels.data(), pixels.size(), w, h});
  Palette custom; custom.body=rgb565(0xFF00FF);
  assert(renderer.character() == Character::Mochi);
  renderer.setPalette(custom);
  assert(renderer.setCharacter(Character::Mochi));
  renderer.clear(); renderer.draw(Pose{},160,160,.8f);
  assert(std::count(pixels.begin(),pixels.end(),custom.body)>10000);
  assert(!renderer.setCharacter(Character::Count));
  assert(!renderer.setCharacter(static_cast<Character>(255)));
  const auto before=pixels;
  renderer.clear(); renderer.draw(Pose{},160,160,.8f);
  assert(pixels==before && renderer.character()==Character::Mochi);
  assert(!std::strcmp(character_name(Character::Count), "Unknown"));

  std::vector<std::vector<uint16_t>> silhouettes;
  for(int i=0;i<kCharacterCount;i++) {
    const auto character=static_cast<Character>(i);
    assert(renderer.setCharacter(character));
    assert(renderer.character()==character && other.character()==Character::Mochi);
    assert(std::strlen(character_detail(character))>0);
    if(i>0) {
      renderer.clear(); renderer.draw(Pose{},160,160,.8f);
      assert(std::count(pixels.begin(),pixels.end(),default_palette(character).body)>10000);
    }
    // All colors identical: characters must still have distinct silhouettes.
    const uint16_t ink=rgb565(0xFFFFFF);
    renderer.setPalette({ink,ink,ink,ink,ink,ink});
    renderer.clear(); renderer.draw(Pose{},160,160,.8f);
    for(const auto& previous:silhouettes) {
      size_t differences=0;
      for(size_t p=0;p<pixels.size();p++) if(previous[p]!=pixels[p]) ++differences;
      assert(differences>500);
    }
    silhouettes.push_back(pixels);
    renderer.setPalette(default_palette(character));
    Pose happy=sample_pose(State::Happy,0);
    renderer.clear(); renderer.draw(happy,160,160,.8f);
    const auto open=pixels;
    happy.eye_l=happy.eye_r=.02f;
    renderer.clear(); renderer.draw(happy,160,160,.8f);
    assert(open!=pixels); // Happy eyes must visibly blink for every character.
  }
}

int main() {
  testAnimator(); testGestures(); testSurface(); testCharacters();
  std::puts("PASS: SDK playback, speech, gestures, buffers, 4 distinct character silhouettes, palette selection and visible happy blinks");
}
