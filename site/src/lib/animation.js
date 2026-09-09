import { characterById } from './characters';
export const STATES = [
  { id: 'idle', name: 'Idle', detail: 'Just happy to be here.' },
  { id: 'blink', name: 'Blinking', detail: 'A tiny blink. A little hello.' },
  { id: 'happy', name: 'Happy', detail: 'You made my day.' },
  { id: 'listening', name: 'Listening', detail: 'You have my full attention.' },
  { id: 'thinking', name: 'Thinking', detail: 'Let me think for a moment…' },
  { id: 'speaking', name: 'Speaking', detail: 'So many little things to tell you.' },
  { id: 'sleeping', name: 'Sleeping', detail: 'Dreaming of something lovely.' },
  { id: 'surprised', name: 'Surprised', detail: 'Oh! I wasn’t expecting that.' },
  { id: 'sad', name: 'Sad', detail: 'Could use a little company.' },
  { id: 'wave', name: 'Waving', detail: 'There you are. Hello, friend!' },
];

const TAU = Math.PI * 2;
const clamp = (n, lo, hi) => Math.min(hi, Math.max(lo, n));
const blinkAt = (t, at) => clamp((0.14 - Math.abs(t - at)) / 0.075, 0, 1);

// Time is supplied by the caller: the same pose can be replayed deterministically.
export function samplePose(state, t, blink = 0, reduced = false) {
  if (reduced) t = 0.7;
  const p = {
    x: 0,
    y: 0,
    sx: 1,
    sy: 1,
    tilt: 0,
    gazeX: 0,
    gazeY: 0,
    eyeL: 1,
    eyeR: 1,
    happyEyes: 0,
    smile: 1,
    open: 0,
    round: 0,
    mouthX: 0,
    browL: 0,
    browR: 0,
    browSad: 0,
    leftX: -153,
    leftY: 35,
    leftAngle: -0.2,
    rightX: 153,
    rightY: 35,
    rightAngle: 0.2,
    cheeks: 1,
  };
  const breath = Math.sin(t * 1.7);
  p.sy += breath * 0.012;
  p.sx -= breath * 0.006;
  p.y -= breath * 1.1;
  switch (state) {
    case 'blink': {
      const phase = t % 2.8;
      blink = Math.max(blink, blinkAt(phase, 0.6), blinkAt(phase, 0.98));
      p.tilt = Math.sin(t * 0.8) * 0.025;
      break;
    }
    case 'happy': {
      const bounce = Math.max(0, Math.sin(t * 4.5));
      p.y -= bounce * 13;
      p.sx += 0.026 * Math.cos(t * 4.5);
      p.sy -= 0.025 * Math.cos(t * 4.5);
      p.happyEyes = 1;
      p.open = 0.65;
      p.smile = 1.2;
      p.leftY = -22 - 5 * bounce;
      p.rightY = -22 - 5 * bounce;
      p.leftX = -151;
      p.rightX = 151;
      p.leftAngle = -0.7;
      p.rightAngle = 0.7;
      p.tilt = Math.sin(t * 2.25) * 0.04;
      break;
    }
    case 'listening':
      p.tilt = -0.075 + Math.sin(t * 1.3) * 0.014;
      p.rightX = 140;
      p.rightY = -42;
      p.rightAngle = -0.35;
      p.gazeX = -5;
      p.eyeL = 1.08;
      p.eyeR = 1.08;
      p.smile = 0.65;
      p.y += Math.sin(t * 2.4) * 1.7;
      break;
    case 'thinking':
      p.tilt = 0.045 + Math.sin(t) * 0.018;
      p.gazeX = 7 + Math.sin(t * 0.65) * 3;
      p.gazeY = -8;
      p.browR = 1;
      p.eyeR = 0.85;
      p.mouthX = 5;
      p.smile = 0.08;
      p.rightX = 29;
      p.rightY = 43;
      p.rightAngle = -0.6 + Math.sin(t * 2) * 0.025;
      break;
    case 'speaking': {
      const speech = (Math.sin(t * 13) * 0.5 + 0.5) * (0.7 + 0.3 * Math.sin(t * 6.7));
      const phrase = t % 4.3 < 3.5 ? 1 : 0;
      p.open = (0.12 + speech * 0.85) * phrase;
      p.round = 0.18;
      p.tilt = Math.sin(t * 1.8) * 0.025;
      p.y += Math.sin(t * 3) * 1.7;
      p.leftY = 9 + Math.sin(t * 3.2) * 12;
      p.leftX = -161;
      p.leftAngle = -0.8;
      break;
    }
    case 'sleeping':
      p.eyeL = p.eyeR = 0.025;
      p.smile = 0.65;
      p.sy = 0.9 + Math.sin(t * 1.2) * 0.025;
      p.sx = 1.04 - Math.sin(t * 1.2) * 0.01;
      p.y = 15 - Math.sin(t * 1.2) * 2;
      p.leftY = p.rightY = 54;
      p.tilt = -0.025;
      break;
    case 'surprised': {
      const pop = Math.exp(-(t % 3.7) * 3);
      p.sy = 1.045 + 0.04 * pop;
      p.sx = 0.955;
      p.y = -5 - 7 * pop;
      p.eyeL = p.eyeR = 1.16;
      p.open = 0.88;
      p.round = 1;
      p.smile = 0;
      p.leftX = -71;
      p.rightX = 71;
      p.leftY = p.rightY = 44;
      p.leftAngle = -0.55;
      p.rightAngle = 0.55;
      break;
    }
    case 'sad':
      p.smile = -0.75;
      p.eyeL = p.eyeR = 0.8;
      p.gazeY = 5;
      p.browL = p.browR = p.browSad = 1;
      p.sy = 0.945;
      p.sx = 1.025;
      p.y = 9 + Math.sin(t * 1.4) * 2;
      p.leftY = p.rightY = 54;
      p.cheeks = 0.65;
      p.tilt = -0.035;
      break;
    case 'wave':
      p.leftX = -151;
      p.leftY = -32;
      p.leftAngle = -0.6 + Math.sin(t * 7) * 0.42;
      p.tilt = 0.065 + Math.sin(t * 3.5) * 0.015;
      p.smile = 1.15;
      p.y -= Math.sin(t * 2) * 2.5;
      break;
    default:
      p.gazeX = Math.sin(t * 0.55) * 3.2;
      p.tilt = Math.sin(t * 0.8) * 0.013;
  }
  if (state === 'blink' && reduced) p.eyeL = p.eyeR = 0.03;
  if (state !== 'sleeping' && !reduced) {
    const natural = t % 5.7;
    blink = Math.max(blink, blinkAt(natural, 4.4));
    p.eyeL *= 1 - 0.97 * blink;
    p.eyeR *= 1 - 0.97 * blink;
  }
  return p;
}

export class Mochi {
  constructor(canvas, options = {}) {
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d');
    this.character = 'mochi';
    this.visible = true;
    this.state = 'idle';
    this.time = 0;
    this.speed = 1;
    this.paused = false;
    this.reduced = Boolean(options.reduced);
    this.pose = samplePose('idle', 0);
    this.lastTime = null;
    this.blinkStart = -100;
    this.frameCount = 0;
    this.resizeObserver = new ResizeObserver(() => this.resize());
    this.resizeObserver.observe(canvas);
    this.loop = this.loop.bind(this);
    this.raf = requestAnimationFrame(this.loop);
  }
  resize() {
    const box = this.canvas.getBoundingClientRect();
    this.width = box.width;
    this.height = box.height;
    this.dpr = Math.min(window.devicePixelRatio || 1, 2);
    this.canvas.width = Math.round(box.width * this.dpr);
    this.canvas.height = Math.round(box.height * this.dpr);
    this.draw();
  }
  setState(id) {
    if (!STATES.some((s) => s.id === id)) throw new RangeError(`Unknown state: ${id}`);
    this.state = id;
    this.time = 0;
    this.blinkStart = -100;
    if (this.paused || this.reduced) {
      this.pose = samplePose(id, 0.7, 0, this.reduced);
      this.draw();
    }
  }
  blink() {
    this.blinkStart = this.time;
  }
  setPaused(value) {
    this.paused = Boolean(value);
    this.lastTime = null;
  }
  loop(now) {
    const dt = this.lastTime === null ? 0 : Math.min((now - this.lastTime) / 1000, 0.05);
    this.lastTime = now;
    if (!this.paused && this.visible && !document.hidden) {
      this.time += dt * this.speed * characterById(this.character).pace;
      const blink = blinkAt(this.time - this.blinkStart, 0.14);
      const target = samplePose(this.state, this.time, blink, this.reduced);
      const mix = 1 - Math.exp(-dt * 13);
      for (const key of Object.keys(target)) {
        const rate = key === 'eyeL' || key === 'eyeR' ? 1 - Math.exp(-dt * 45) : mix;
        this.pose[key] += (target[key] - this.pose[key]) * rate;
      }
      this.draw();
      this.frameCount++;
    }
    this.raf = requestAnimationFrame(this.loop);
  }
  draw() {
    const c = this.ctx,
      p = this.pose,
      colors = characterById(this.character);
    if (!this.width || !this.height || !c) return;
    c.setTransform(this.dpr, 0, 0, this.dpr, 0, 0);
    c.clearRect(0, 0, this.width, this.height);
    const scale = Math.min((this.width * 0.78) / 350, (this.height * 0.98) / 270);
    const centerY = this.height * 0.51;
    c.save();
    c.translate(this.width / 2 + p.x, centerY + p.y * scale);
    c.scale(scale, scale);
    c.rotate(p.tilt);
    c.scale(p.sx, p.sy);
    c.fillStyle = colors.body;
    if (this.character === 'nimbus') {
      for (const [x, y, rx, ry] of [
        [0, 30, 147, 94],
        [-94, -13, 58, 70],
        [-27, -58, 67, 66],
        [68, -30, 68, 73],
        [117, 24, 38, 55],
      ]) {
        c.beginPath();
        c.ellipse(x, y, rx, ry, 0, 0, TAU);
        c.fill();
      }
    } else {
      if (this.character === 'peach') {
        for (const side of [-1, 1]) {
          c.save();
          c.scale(side, 1);
          c.beginPath();
          c.moveTo(115, -44);
          c.quadraticCurveTo(132, -104, 109, -132);
          c.quadraticCurveTo(75, -134, 48, -83);
          c.fill();
          c.fillStyle = colors.accent;
          c.beginPath();
          c.moveTo(105, -83);
          c.lineTo(105, -118);
          c.lineTo(73, -88);
          c.fill();
          c.restore();
          c.fillStyle = colors.body;
        }
      }
      const top = this.character === 'mochi' ? -128 : -100;
      c.beginPath();
      c.moveTo(0, top);
      c.bezierCurveTo(79, top - 2, 123, -69, 145, -3);
      c.bezierCurveTo(161, 42, 171, 80, 146, 101);
      c.bezierCurveTo(124, 122, 59, 124, 0, 124);
      c.bezierCurveTo(-62, 124, -127, 121, -148, 103);
      c.bezierCurveTo(-174, 83, -162, 42, -145, -5);
      c.bezierCurveTo(-122, -75, -72, top - 1, 0, top);
      c.closePath();
      c.fill();
      if (this.character === 'sprout') {
        c.strokeStyle = colors.accent;
        c.lineWidth = 6;
        c.beginPath();
        c.moveTo(0, -91);
        c.bezierCurveTo(-2, -104, 0, -114, 4, -121);
        c.stroke();
        for (const side of [-1, 1]) {
          c.fillStyle = colors.accent;
          c.beginPath();
          c.moveTo(2, -113);
          c.bezierCurveTo(side * 5, -139, side * 28, -140, side * 46, side < 0 ? -127 : -138);
          c.bezierCurveTo(side * 34, -111, side * 17, -103, 2, -113);
          c.fill();
        }
      }
    }
    this.hand(p.leftX, p.leftY, p.leftAngle, -1);
    this.hand(p.rightX, p.rightY, p.rightAngle, 1);
    c.save();
    c.translate(p.gazeX, p.gazeY);
    c.fillStyle = colors.blush;
    c.globalAlpha = p.cheeks;
    for (const x of [-68, 68]) {
      c.beginPath();
      c.ellipse(x, 17, 13, 8.5, 0.07, 0, TAU);
      c.fill();
    }
    c.globalAlpha = 1;
    this.eye(-48, -15, p.eyeL, p.happyEyes);
    this.eye(48, -15, p.eyeR, p.happyEyes);
    c.lineCap = 'round';
    c.strokeStyle = colors.face;
    c.lineWidth = 4.5;
    for (const [x, amount, side] of [
      [-48, p.browL, -1],
      [48, p.browR, 1],
    ]) {
      if (amount < 0.01) continue;
      c.globalAlpha = amount;
      c.beginPath();
      c.moveTo(x - 9, -47 + side * 4 * p.browSad);
      c.quadraticCurveTo(x, -53, x + 9, -47 - side * 4 * p.browSad);
      c.stroke();
    }
    if (this.character === 'peach') {
      c.strokeStyle = colors.crease;
      c.lineWidth = 2.2;
      for (const side of [-1, 1])
        for (const y of [5, 15]) {
          c.beginPath();
          c.moveTo(side * 93, y);
          c.lineTo(side * 106, y - 3);
          c.stroke();
        }
      c.fillStyle = colors.face;
      c.beginPath();
      c.moveTo(-4, 8);
      c.lineTo(4, 8);
      c.lineTo(0, 12);
      c.fill();
      c.strokeStyle = colors.face;
    }
    c.globalAlpha = 1;
    c.translate(p.mouthX, 19);
    const mouthOpen = clamp(p.open, 0, 1);
    c.fillStyle = colors.face;
    if (mouthOpen > 0.06) {
      const width = 14 - p.round * 5,
        height = 3 + mouthOpen * 11;
      c.beginPath();
      c.ellipse(0, 3 + height / 2, width, height, 0, 0, TAU);
      c.fill();
    } else {
      c.lineWidth = 5;
      c.beginPath();
      c.moveTo(-12, 0);
      c.bezierCurveTo(-6, p.smile * 12, 6, p.smile * 12, 12, -1);
      c.stroke();
    }
    c.restore();
    c.restore();
  }
  hand(x, y, angle, side) {
    const c = this.ctx,
      colors = characterById(this.character);
    c.save();
    c.translate(x, y);
    c.rotate(angle);
    c.fillStyle = colors.body;
    c.beginPath();
    c.ellipse(0, 0, 20, 28, 0, 0, TAU);
    c.fill();
    c.strokeStyle = colors.crease;
    c.lineWidth = 2;
    c.lineCap = 'round';
    c.beginPath();
    if (side < 0) c.ellipse(0, 0, 17, 25, 0, 0.1, 1.75);
    else c.ellipse(0, 0, 17, 25, 0, 1.4, 3.05);
    c.stroke();
    c.restore();
  }
  eye(x, y, openness, happy) {
    const c = this.ctx,
      colors = characterById(this.character);
    c.fillStyle = colors.face;
    if (happy > 0.65) {
      c.lineWidth = 5.5;
      c.strokeStyle = colors.face;
      c.lineCap = 'round';
      c.beginPath();
      c.moveTo(x - 10, y);
      c.quadraticCurveTo(x, y - 15 * Math.min(1, openness + 0.2), x + 10, y);
      c.stroke();
    } else if (openness < 0.12) {
      c.lineWidth = 4.5;
      c.strokeStyle = colors.face;
      c.lineCap = 'round';
      c.beginPath();
      c.moveTo(x - 9, y + 2);
      c.quadraticCurveTo(x, y + 9, x + 9, y + 2);
      c.stroke();
    } else {
      const h = 34 * openness;
      c.beginPath();
      c.roundRect(x - 8.5, y - h / 2, 17, h, Math.min(8.5, h / 2));
      c.fill();
    }
  }
  destroy() {
    cancelAnimationFrame(this.raf);
    this.resizeObserver.disconnect();
  }
}
