import { Mochi, STATES } from './mochi.js';

const $ = id => document.getElementById(id);
const canvas = $('mascot');
const reduced = matchMedia('(prefers-reduced-motion: reduce)').matches;
const mochi = new Mochi(canvas, { reduced });
let selected = 0, touring = false, elapsedTour = 0, lastTourTime = performance.now();

function faceIcon(id) {
  const happy = ['happy','sleeping','blink'].includes(id);
  const eyes = happy ? '<path d="M8 11q2-3 4 0m4 0q2-3 4 0"/>' : '<path d="M10 9v3m8-3v3"/>';
  const mouth = ['surprised','speaking'].includes(id) ? '<ellipse cx="14" cy="17" rx="2" ry="2.5" fill="currentColor" stroke="none"/>' : `<path d="M11 17q3 ${id==='sad'?-4:4} 6 0"/>`;
  return `<svg class="face-icon" aria-hidden="true" viewBox="0 0 28 28" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M3 20C3 12 6 3 14 3s11 9 11 17c0 4-5 5-11 5S3 24 3 20Z" stroke-width="1" opacity=".55"/>${eyes}${mouth}</svg>`;
}
for (const state of STATES) {
  const button = document.createElement('button'); button.className = 'mood-button'; button.dataset.state = state.id;
  button.setAttribute('aria-pressed', String(state.id === 'idle'));
  button.innerHTML = `${faceIcon(state.id)}<span class="mood-name">${state.name}</span><span class="selection-dot" aria-hidden="true"></span>`;
  button.addEventListener('click', () => select(state.id)); $('moods').append(button);
  const touchButton = document.createElement('button'); touchButton.textContent = state.name; touchButton.dataset.state = state.id;
  touchButton.setAttribute('aria-pressed', String(state.id === 'idle'));
  touchButton.addEventListener('click', () => { select(state.id); togglePicker(false); }); $('screen-moods').append(touchButton);
}
function select(id, fromTour = false) {
  selected = STATES.findIndex(s => s.id === id); if (selected < 0) return;
  mochi.setState(id); elapsedTour = 0;
  if (!fromTour) { touring = false; $('tour').checked = false; }
  const state = STATES[selected]; $('screen-state').textContent = state.name; $('screen-detail').textContent = state.detail;
  $('state-count').textContent = `${String(selected + 1).padStart(2,'0')} / ${STATES.length}`;
  canvas.setAttribute('aria-label', `Mochi in the ${state.name} state. Tap to blink or swipe to change state.`);
  document.querySelectorAll('[data-state]').forEach(el => el.setAttribute('aria-pressed', String(el.dataset.state === id)));
}
function step(amount) { select(STATES[(selected + amount + STATES.length) % STATES.length].id); }
function togglePicker(open) {
  $('screen-picker').hidden = !open; $('choose').setAttribute('aria-expanded', String(open));
  if (open) $('screen-moods').querySelector('[aria-pressed=true]').focus(); else $('choose').focus();
}
$('choose').addEventListener('click', () => togglePicker($('screen-picker').hidden));
$('close-picker').addEventListener('click', () => togglePicker(false));
$('previous').addEventListener('click', () => step(-1)); $('next').addEventListener('click', () => step(1));
$('play').addEventListener('click', () => {
  mochi.setPaused(!mochi.paused); $('play').setAttribute('aria-pressed', String(mochi.paused));
  $('play').textContent = mochi.paused ? '▶ Play' : 'Ⅱ Pause';
});
$('speed').addEventListener('input', e => { mochi.speed = Number(e.target.value); $('speed-value').value = `${mochi.speed}×`; });
$('tour').addEventListener('change', e => { touring = e.target.checked; elapsedTour = 0; });
$('fullscreen').addEventListener('click', async () => {
  try {
    if (document.fullscreenElement) await document.exitFullscreen(); else await $('device').requestFullscreen();
  } catch { $('fullscreen').title = 'Fullscreen is not available in this browser'; }
});
document.addEventListener('fullscreenchange', () => $('fullscreen').setAttribute('aria-label', document.fullscreenElement ? 'Exit fullscreen' : 'Enter fullscreen'));
document.addEventListener('keydown', e => {
  if (['INPUT','TEXTAREA','SELECT'].includes(e.target.tagName)) return;
  if (e.key === 'Escape' && !$('screen-picker').hidden) { togglePicker(false); return; }
  if (!$('screen-picker').hidden) return;
  if (e.key === 'ArrowRight') { e.preventDefault(); step(1); }
  if (e.key === 'ArrowLeft') { e.preventDefault(); step(-1); }
  if ((e.key === ' ' || e.key === 'Enter') && e.target === canvas) { e.preventDefault(); mochi.blink(); }
});
let pointer = null;
canvas.addEventListener('pointerdown', e => { pointer = { x:e.clientX, y:e.clientY, id:e.pointerId }; canvas.setPointerCapture(e.pointerId); });
canvas.addEventListener('pointerup', e => {
  if (!pointer || pointer.id !== e.pointerId) return;
  const dx=e.clientX-pointer.x, dy=e.clientY-pointer.y;
  if (Math.abs(dx)>45 && Math.abs(dx)>Math.abs(dy)*1.3) step(dx<0?1:-1);
  else if (Math.hypot(dx,dy)<20) mochi.blink();
  pointer=null;
});
canvas.addEventListener('pointercancel', () => { pointer = null; });
function tourTick(now) {
  const dt=Math.min((now-lastTourTime)/1000,.1); lastTourTime=now;
  if (touring && !mochi.paused && !document.hidden && $('screen-picker').hidden) {
    elapsedTour+=dt*mochi.speed;
    if(elapsedTour>=5.5) select(STATES[(selected+1)%STATES.length].id,true);
  }
  requestAnimationFrame(tourTick);
}
requestAnimationFrame(tourTick);
if(reduced) { mochi.setPaused(true); $('play').textContent='▶ Play'; $('play').setAttribute('aria-pressed','true'); }
// Explicit opt-in to motion after the system preference started the preview paused.
$('play').addEventListener('click', () => { if(!mochi.paused) mochi.reduced=false; });
