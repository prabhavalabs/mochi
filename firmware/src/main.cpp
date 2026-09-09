#include <Arduino.h>
#include <Mochi.h>
#include <MochiWaveshare216.h>
#include "character_layout.h"
#include "serial_line.h"
#include <cmath>
#include <cstring>

namespace {
using namespace mochi;
namespace board = mochi::waveshare216;
constexpr int kWidth = board::kWidth, kHeight = board::kHeight;
constexpr uint16_t white = rgb565(0xF8F3E6), muted = rgb565(0x9BA3AF);
constexpr uint16_t peach = rgb565(0xF4BA9F), surface = rgb565(0x1C2129);
constexpr uint16_t border = rgb565(0x343B46);
Animator buddy;
GestureRecognizer gestures;
enum class View { Stage, Moods, Characters };
View view = View::Stage;
Character character = Character::Mochi;
bool ready = false, tour = false, dirty = true;
float tour_elapsed = 0;
uint32_t previous_ms = 0, frame_ms = 0, frame_count = 0, fps_ms = 0, touch_count = 0;
float fps = 0;
uint32_t render_us = 0, transfer_us = 0;

void text(const char* label, int x, int y, uint8_t size, uint16_t color = white) {
  auto& c = board::canvas();
  c.setTextSize(size); c.setTextColor(color); c.setCursor(x, y); c.print(label);
}
void centered(const char* label, int x, int y, uint8_t size, uint16_t color = white) {
  text(label, x - strlen(label) * 3 * size, y, size, color);
}
void button(int x, int y, int w, int h, const char* label, bool active = false) {
  auto& c = board::canvas();
  c.fillRoundRect(x, y, w, h, 14, active ? peach : surface);
  c.drawRoundRect(x, y, w, h, 14, active ? peach : border);
  centered(label, x + w / 2, y + (h - 16) / 2 + 1, 2, active ? kBackground : white);
}
bool inside(int x, int y, int bx, int by, int w, int h) {
  return x >= bx && x < bx + w && y >= by && y < by + h;
}
void select_state(int index) {
  index = (index % kStateCount + kStateCount) % kStateCount;
  buddy.setState(static_cast<State>(index));
  tour_elapsed = 0;
  dirty = true;
  Serial.printf("Mochi state: %d %s\n", index, state_name(buddy.state()));
}
void select_character(Character selected) {
  if (static_cast<unsigned>(selected) >= kCharacterCount) return;
  character = selected;
  dirty = true;
  Serial.printf("Character: %s\n", character_name(character));
}
void status_json() {
  const auto& b = board::status();
  Serial.printf("{\"app\":\"mochi-sdk-example\",\"version\":\"%s\",\"ready\":%s,"
      "\"display\":%s,\"touch\":%s,\"pmic\":%s,\"psram\":%u,"
      "\"state\":\"%s\",\"character\":\"%s\",\"view\":\"%s\",\"fps\":%.1f,\"touch_events\":%u,\"heap\":%u,\"render_us\":%u,\"transfer_us\":%u}\n",
      kVersion, ready ? "true" : "false", b.display ? "true" : "false", b.touch ? "true" : "false",
      b.power ? "true" : "false", ESP.getPsramSize(), state_name(buddy.state()), character_name(character),
      view == View::Stage ? "stage" : view == View::Moods ? "moods" : "characters",
      fps, touch_count, ESP.getFreeHeap(), render_us, transfer_us);
}
void tap(int x, int y) {
  dirty = true;
  if (view == View::Characters) {
    if (inside(x, y, 404, 18, 52, 44)) { view = View::Stage; return; }
    for (int i = 0; i < kCharacterCount; ++i) {
      if (inside(x, y, 24 + (i % 2) * 224, 78 + (i / 2) * 164, 208, 148)) {
        select_character(static_cast<Character>(i)); view = View::Stage; return;
      }
    }
    return;
  }
  if (view == View::Moods) {
    if (inside(x, y, 404, 18, 52, 44)) { view = View::Stage; return; }
    for (int i = 0; i < kStateCount; ++i) {
      if (inside(x, y, 24 + (i % 2) * 224, 78 + (i / 2) * 58, 208, 48)) {
        select_state(i); view = View::Stage; return;
      }
    }
    if (inside(x, y, 24, 392, 136, 52)) buddy.setPaused(!buddy.paused());
    if (inside(x, y, 172, 392, 136, 52)) { tour = !tour; tour_elapsed = 0; }
    if (inside(x, y, 320, 392, 136, 52)) buddy.setSpeed(buddy.speed() == 1 ? 1.5f : buddy.speed() == 1.5f ? .5f : 1.f);
    return;
  }
  if (inside(x, y, 24, 18, 196, 38)) view = View::Characters;
  else if (inside(x, y, 342, 18, 114, 38)) { tour = !tour; tour_elapsed = 0; }
  else if (inside(x, y, 24, 408, 64, 48)) select_state(static_cast<int>(buddy.state()) - 1);
  else if (inside(x, y, 392, 408, 64, 48)) select_state(static_cast<int>(buddy.state()) + 1);
  else if (inside(x, y, 100, 408, 280, 48)) view = View::Moods;
  else if (y > 60 && y < 342) {
    buddy.blink();
  }
}
void input() {
  int16_t x = 0, y = 0;
  const TouchReport report = board::readTouch(x, y);
  if (report == TouchReport::Pressed && !gestures.pressed()) ++touch_count;
  const Gesture gesture = gestures.update(report, x, y, millis());
  if (view == View::Stage && gesture.type == GestureType::SwipeLeft) select_state(static_cast<int>(buddy.state()) + 1);
  else if (view == View::Stage && gesture.type == GestureType::SwipeRight) select_state(static_cast<int>(buddy.state()) - 1);
  else if (gesture.type == GestureType::Tap) tap(gesture.x, gesture.y);
  static bool previous[2] = {true, true}, stable[2] = {true, true};
  static uint32_t changed[2]{};
  const int pins[2] = {board::kUserButton, board::kBootButton};
  for (int i = 0; i < 2; ++i) {
    const bool level = digitalRead(pins[i]);
    if (level != previous[i]) { previous[i] = level; changed[i] = millis(); }
    if (level != stable[i] && millis() - changed[i] >= 30) {
      stable[i] = level;
      if (!level) { view = View::Stage; select_state(static_cast<int>(buddy.state()) + (i == 0 ? 1 : -1)); }
    }
  }
}
void serial_input() {
  static playground::SerialLine line;
  while (Serial.available()) {
    const auto result = line.push(static_cast<char>(Serial.read()));
    if (result == playground::LineResult::Rejected) {
      Serial.println("Command rejected: use at most 63 printable ASCII characters");
    } else if (result == playground::LineResult::Ready) {
      const char* command = line.line();
      if (!strcmp(command, "status")) status_json();
      else if (!strncmp(command, "state ", 6)) {
        char* end; const long index = strtol(command + 6, &end, 10);
        if (end != command + 6 && !*end && index >= 0 && index < kStateCount) select_state(index);
        else Serial.println("Use state 0..9");
      } else if (!strncmp(command, "character ", 10)) {
        char* end; const long index = strtol(command + 10, &end, 10);
        if (end != command + 10 && !*end && index >= 0 && index < kCharacterCount) select_character(static_cast<Character>(index));
        else Serial.println("Use character 0..3");
      } else if (!strcmp(command, "menu characters")) { view = View::Characters; dirty = true; }
      else if (!strcmp(command, "menu moods")) { view = View::Moods; dirty = true; }
      else if (!strcmp(command, "menu close")) { view = View::Stage; dirty = true; }
      else if (!strcmp(command, "tour on")) { tour = true; tour_elapsed = 0; dirty = true; }
      else if (!strcmp(command, "tour off")) { tour = false; dirty = true; }
      else if (!strcmp(command, "pause")) { buddy.setPaused(true); dirty = true; }
      else if (!strcmp(command, "play")) { buddy.setPaused(false); dirty = true; }
      else Serial.println("Commands: status, state 0..9, character 0..3, menu characters/moods/close, tour on/off, pause, play");
    }
  }
}
void draw() {
  const uint32_t started = micros();
  auto& c = board::canvas();
  if (dirty || view != View::Stage) c.fillScreen(kBackground);
  else c.fillRect(0, 48, kWidth, 300, kBackground);
  if (view == View::Characters) {
    text("Meet your buddies", 24, 29, 2);
    button(404, 18, 52, 44, "x");
    Renderer renderer({c.getFramebuffer(), size_t(kWidth * kHeight), kWidth, kHeight});
    for (int i = 0; i < kCharacterCount; ++i) {
      const Character candidate = static_cast<Character>(i);
      const int x = 24 + i % 2 * 224, y = 78 + i / 2 * 164;
      const bool selected = candidate == character;
      c.fillRoundRect(x, y, 208, 148, 16, surface);
      c.drawRoundRect(x, y, 208, 148, 16, selected ? peach : border);
      renderer.setCharacter(candidate);
      renderer.draw(sample_pose(State::Idle, 0), x + 104, y + 59, .31f);
      centered(character_name(candidate), x + 104, y + 107, 2, selected ? peach : white);
      centered(selected ? "Selected" : character_detail(candidate), x + 104, y + 132, 1, muted);
    }
    centered("Pick a friend. Keep your mood.", 240, 421, 1, muted);
    centered("Every buddy has all ten animations", 240, 445, 1, muted);
  } else if (view == View::Moods) {
    text("Choose a mood", 24, 28, 3);
    button(404, 18, 52, 44, "x");
    for (int i = 0; i < kStateCount; ++i)
      button(24 + i % 2 * 224, 78 + i / 2 * 58, 208, 48,
             state_name(static_cast<State>(i)), i == static_cast<int>(buddy.state()));
    button(24, 392, 136, 52, buddy.paused() ? "Play" : "Pause", buddy.paused());
    button(172, 392, 136, 52, tour ? "Tour on" : "Tour", tour);
    button(320, 392, 136, 52, buddy.speed() == 1 ? "1x pace" : buddy.speed() > 1 ? "1.5x pace" : ".5x pace");
    centered("Ten little ways to feel", 240, 459, 1, muted);
  } else {
    button(24, 18, 196, 38, character_name(character));
    c.fillTriangle(201, 32, 211, 32, 206, 38, muted);
    button(342, 18, 114, 38, tour ? "Tour on" : "Tour", tour);
    Renderer renderer({c.getFramebuffer(), size_t(kWidth * kHeight), kWidth, kHeight});
    renderer.setCharacter(character);
    renderer.draw(buddy.pose(), 240, 207, playground::stage_scale(character));
    // Small state cues live outside the silhouette.
    const float t = buddy.elapsed();
    if (buddy.state() == State::Sleeping) text("z Z", 353, 104 - int(4 * sinf(t)), 2, peach);
    if (buddy.state() == State::Thinking) for (int i = 0; i < 3; ++i)
      c.fillCircle(350 + i * 17, 101, 3 + (int(t * 2) % 3 == i ? 2 : 0), peach);
    if (buddy.paused()) text("PAUSED", 24, 64, 1, muted);
    centered(state_name(buddy.state()), 240, 352, 3);
    centered(state_detail(buddy.state()), 240, 382, 1, muted);
    button(24, 408, 64, 48, "<");
    button(100, 408, 280, 48, "Choose a mood");
    button(392, 408, 64, 48, ">");
    if (!board::status().touch) centered("Touch offline - use + and -", 240, 467, 1, peach);
  }
  render_us = micros() - started;
  const uint32_t transfer_started = micros();
  board::present(dirty || view != View::Stage ? 0 : 48, dirty || view != View::Stage ? kHeight : 300);
  transfer_us = micros() - transfer_started;
  ++frame_count;
  dirty = false;
}
}

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
  Serial.printf("Mochi SDK %s example starting\n", kVersion);
  ready = board::begin();
  previous_ms = fps_ms = millis();
  status_json();
}
void loop() {
  serial_input();
  if (!ready) { delay(10); return; }
  input();
  const uint32_t now = millis();
  const float dt = fminf((now - previous_ms) / 1000.f, .1f);
  previous_ms = now;
  if (!buddy.paused() && view == View::Stage) {
    tour_elapsed += dt * buddy.speed();
    if (tour && tour_elapsed >= 5.5f) select_state(static_cast<int>(buddy.state()) + 1);
    buddy.update(dt);
  }
  if ((dirty || (!buddy.paused() && view == View::Stage)) && now - frame_ms >= 33) { frame_ms = now; draw(); }
  if (now - fps_ms >= 3000) {
    fps = frame_count * 1000.f / (now - fps_ms); frame_count = 0; fps_ms = now;
  }
  delay(1);
}
