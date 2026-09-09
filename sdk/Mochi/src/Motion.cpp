#include "mochi/Motion.h"
#include <algorithm>
#include <cmath>

namespace mochi {
namespace {
// Reduce phases before converting to float so long-running states keep smooth motion.
float wave(double t, double frequency) { return std::sin(float(std::remainder(t * frequency, 6.283185307179586))); }
float wave_cos(double t, double frequency) { return std::cos(float(std::remainder(t * frequency, 6.283185307179586))); }
const char* names[] = {"Idle", "Blinking", "Happy", "Listening", "Thinking",
  "Speaking", "Sleeping", "Surprised", "Sad", "Waving"};
const char* details[] = {"Just happy to be here", "A tiny blink, a little hello",
  "You made my day", "You have my full attention", "Let me think a moment...",
  "So much to tell you", "Dreaming of something lovely", "Oh! What was that?",
  "Could use a little company", "Hello, friend!"};
}
const char* state_name(State s) { return names[static_cast<unsigned>(s) % kStateCount]; }
const char* state_detail(State s) { return details[static_cast<unsigned>(s) % kStateCount]; }
float blink_at(float t, float center) { return std::clamp((.14f - std::fabs(t-center))/.075f, 0.f, 1.f); }

Pose sample_pose(State state, double t, float blink) {
  Pose p;
  const float breath = wave(t,1.7f);
  p.sy += breath*.012f; p.sx -= breath*.006f; p.y -= breath*1.1f;
  switch (state) {
    case State::Blinking: {
      const float phase = std::fmod(t,2.8f);
      blink = std::max({blink, blink_at(phase,.6f), blink_at(phase,.98f)});
      p.tilt = wave(t,.8f)*.025f; break;
    }
    case State::Happy: {
      const float bounce = std::max(0.f,wave(t,4.5f));
      p.y -= bounce*13; p.sx += .026f*wave_cos(t,4.5f); p.sy -= .025f*wave_cos(t,4.5f);
      p.happy_eyes=1; p.mouth_open=.65f; p.smile=1.2f;
      p.left_y=p.right_y=-22-5*bounce; p.left_x=-151; p.right_x=151;
      p.left_angle=-.7f; p.right_angle=.7f; p.tilt=wave(t,2.25f)*.04f; break;
    }
    case State::Listening:
      p.tilt=-.075f+wave(t,1.3f)*.014f; p.right_x=140; p.right_y=-42;
      p.right_angle=-.35f; p.gaze_x=-5; p.eye_l=p.eye_r=1.08f;
      p.smile=.65f; p.y+=wave(t,2.4f)*1.7f; break;
    case State::Thinking:
      p.tilt=.045f+wave(t,1)*.018f; p.gaze_x=7+wave(t,.65f)*3;
      p.gaze_y=-8; p.brow_r=1; p.eye_r=.85f; p.mouth_x=5; p.smile=.08f;
      p.right_x=29; p.right_y=43; p.right_angle=-.6f+wave(t,2)*.025f; break;
    case State::Speaking: {
      const float speech=(wave(t,13)*.5f+.5f)*(.7f+.3f*wave(t,6.7f));
      p.mouth_open=std::fmod(t,4.3f)<3.5f ? .12f+speech*.85f : 0;
      p.mouth_round=.18f; p.tilt=wave(t,1.8f)*.025f; p.y+=wave(t,3)*1.7f;
      p.left_y=9+wave(t,3.2f)*12; p.left_x=-161; p.left_angle=-.8f; break;
    }
    case State::Sleeping:
      p.eye_l=p.eye_r=.025f; p.smile=.65f; p.sy=.9f+wave(t,1.2f)*.025f;
      p.sx=1.04f-wave(t,1.2f)*.01f; p.y=15-wave(t,1.2f)*2;
      p.left_y=p.right_y=54; p.tilt=-.025f; break;
    case State::Surprised: {
      const float pop=std::exp(-std::fmod(t,3.7f)*3);
      p.sy=1.045f+.04f*pop; p.sx=.955f; p.y=-5-7*pop;
      p.eye_l=p.eye_r=1.16f; p.mouth_open=.88f; p.mouth_round=1; p.smile=0;
      p.left_x=-71; p.right_x=71; p.left_y=p.right_y=44;
      p.left_angle=-.55f; p.right_angle=.55f; break;
    }
    case State::Sad:
      p.smile=-.75f; p.eye_l=p.eye_r=.8f; p.gaze_y=5;
      p.brow_l=p.brow_r=p.brow_sad=1; p.sy=.945f; p.sx=1.025f;
      p.y=9+wave(t,1.4f)*2; p.left_y=p.right_y=54; p.cheeks=.65f; p.tilt=-.035f; break;
    case State::Waving:
      p.left_x=-151; p.left_y=-32; p.left_angle=-.6f+wave(t,7)*.42f;
      p.tilt=.065f+wave(t,3.5f)*.015f; p.smile=1.15f;
      p.y-=wave(t,2)*2.5f; break;
    default: p.gaze_x=wave(t,.55f)*3.2f; p.tilt=wave(t,.8f)*.013f;
  }
  if (state!=State::Sleeping) {
    blink=std::max(blink,blink_at(std::fmod(t,5.7f),4.4f));
    p.eye_l*=1-.97f*blink; p.eye_r*=1-.97f*blink;
  }
  return p;
}
void blend_pose(Pose& p, const Pose& t, float dt) {
  const float a=1-std::exp(-dt*13), e=1-std::exp(-dt*45);
#define MIX(field) p.field+=(t.field-p.field)*a
  MIX(y); MIX(sx); MIX(sy); MIX(tilt); MIX(gaze_x); MIX(gaze_y);
  MIX(happy_eyes); MIX(smile); MIX(mouth_open); MIX(mouth_round); MIX(mouth_x);
  MIX(brow_l); MIX(brow_r); MIX(brow_sad); MIX(left_x); MIX(left_y); MIX(left_angle);
  MIX(right_x); MIX(right_y); MIX(right_angle); MIX(cheeks);
#undef MIX
  p.eye_l+=(t.eye_l-p.eye_l)*e; p.eye_r+=(t.eye_r-p.eye_r)*e;
}
}
