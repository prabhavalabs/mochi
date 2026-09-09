import { test } from 'node:test';
import vm from 'node:vm';
import assert from 'node:assert/strict';
import { build } from 'esbuild';
import { fileURLToPath } from 'node:url';
const load = async (relative) => {
  const result = await build({
    entryPoints: [fileURLToPath(new URL(relative, import.meta.url))],
    bundle: true,
    platform: 'node',
    format: 'esm',
    write: false,
  });
  return import(
    'data:text/javascript;base64,' + Buffer.from(result.outputFiles[0].contents).toString('base64')
  );
};
const { parsePreferences } = await load('../src/lib/preferences.ts');
const { characters } = await load('../src/lib/characters.ts');
const { samplePose, STATES, Mochi } = await load('../src/lib/animation.js');
const { adjacentStateId, nextDevicePace, previewShouldPause } = await load(
  '../src/lib/device-preview-state.ts',
);
test('Stored preferences tolerate missing, malformed, and unsupported values', () => {
  for (const value of [
    null,
    '',
    'broken',
    'null',
    '42',
    '[]',
    '{"character":"unknown","theme":"unknown"}',
  ]) {
    assert.deepEqual(parsePreferences(value), { character: 'mochi', theme: 'system' });
  }
  for (const c of characters)
    for (const theme of ['light', 'dark', 'system'])
      assert.deepEqual(parsePreferences(JSON.stringify({ character: c.id, theme })), {
        character: c.id,
        theme,
      });
});
test('Every state has a finite, expressive still pose with reduced motion', () => {
  assert.equal(STATES.length, 10);
  for (const state of STATES) {
    const pose = samplePose(state.id, 1, 0, true);
    assert.ok(Object.values(pose).every(Number.isFinite));
    assert.deepEqual(pose, samplePose(state.id, 1000, 0, true));
  }
  assert.ok(samplePose('sleeping', 1).eyeL < 0.1);
  assert.ok(samplePose('blink', 1, 0, true).eyeL < 0.1);
  assert.ok(samplePose('speaking', 1).open > 0);
  assert.ok(samplePose('happy', 1).happyEyes > 0.5);
});
test('All 40 character/state combinations produce valid canvas geometry', () => {
  const calls = [];
  const ctx = new Proxy(
    {},
    {
      get:
        (_, name) =>
        (...args) => {
          for (const arg of args)
            if (typeof arg === 'number')
              assert.ok(Number.isFinite(arg), `${name}: finite coordinate`);
          calls.push(name);
        },
      set: () => true,
    },
  );
  globalThis.ResizeObserver = class {
    observe() {}
    disconnect() {}
  };
  globalThis.requestAnimationFrame = () => 1;
  globalThis.cancelAnimationFrame = () => {};
  globalThis.window = { devicePixelRatio: 1 };
  const canvas = {
    getContext: () => ctx,
    getBoundingClientRect: () => ({ width: 320, height: 320 }),
  };
  const buddy = new Mochi(canvas);
  buddy.resize();
  buddy.setPaused(true);
  for (const character of characters)
    for (const state of STATES) {
      calls.length = 0;
      buddy.character = character.id;
      buddy.setState(state.id);
      buddy.draw();
      assert.ok(calls.includes('fill'), `${character.id}/${state.id} draws a body`);
      assert.equal(
        calls.filter((c) => c === 'save').length,
        calls.filter((c) => c === 'restore').length,
        'Balanced canvas transforms',
      );
      assert.equal(buddy.paused, true, 'State previews preserve pause');
    }
  assert.throws(() => buddy.setState('invalid'), RangeError);
  buddy.destroy();
});

test('Device controls wrap moods and offer the firmware pace sequence', () => {
  assert.equal(adjacentStateId(STATES[0].id, -1), STATES.at(-1).id);
  assert.equal(adjacentStateId(STATES.at(-1).id, 1), STATES[0].id);
  for (let index = 0; index < STATES.length; index++) {
    const next = adjacentStateId(STATES[index].id, 1);
    assert.equal(next, STATES[(index + 1) % STATES.length].id);
    assert.equal(adjacentStateId(next, -1), STATES[index].id);
  }
  assert.throws(() => adjacentStateId('unknown', 1), RangeError);
  assert.equal(nextDevicePace(0.5), 1);
  assert.equal(nextDevicePace(1), 1.5);
  assert.equal(nextDevicePace(1.5), 0.5);
  assert.equal(nextDevicePace(0.8), 1);
  assert.equal(nextDevicePace(1.8), 0.5);
});

test('Opening a device menu suspends playback without changing its stored pause state', () => {
  assert.equal(previewShouldPause(false, 'stage'), false);
  assert.equal(previewShouldPause(false, 'characters'), true);
  assert.equal(previewShouldPause(false, 'moods'), true);
  assert.equal(previewShouldPause(true, 'stage'), true);
});

test('System theme changes, explicit overrides, cross-tab updates, and restricted storage', async () => {
  const result = await build({
    entryPoints: [fileURLToPath(new URL('../src/lib/preferences.ts', import.meta.url))],
    bundle: true,
    platform: 'browser',
    format: 'iife',
    globalName: 'Preferences',
    define: { 'process.env.NODE_ENV': '"production"' },
    write: false,
  });
  const root = {
    dataset: {},
    classList: {
      toggle: (_, value) => {
        root.dark = value;
      },
    },
  };
  let onSystemChange, onStorage;
  const system = {
    matches: true,
    addEventListener: (_, handler) => {
      onSystemChange = handler;
    },
  };
  const context = vm.createContext({
    document: { documentElement: root },
    window: {
      addEventListener: (_, handler) => {
        onStorage = handler;
      },
    },
    matchMedia: () => system,
    localStorage: {
      getItem() {
        throw new Error('Storage blocked');
      },
      setItem() {
        throw new Error('Storage blocked');
      },
    },
  });
  vm.runInContext(result.outputFiles[0].text, context);
  assert.equal(root.dark, true, 'System dark mode applies even without storage');
  system.matches = false;
  onSystemChange();
  assert.equal(root.dark, false);
  context.Preferences.setPreferences({ theme: 'dark', character: 'peach' });
  assert.equal(root.dataset.character, 'peach');
  onSystemChange();
  assert.equal(root.dark, true, 'Explicit theme wins');
  onStorage({ key: 'unrelated', newValue: '{}' });
  assert.equal(root.dataset.character, 'peach');
  onStorage({ key: 'mochi.preferences', newValue: '{"character":"nimbus","theme":"light"}' });
  assert.equal(root.dataset.character, 'nimbus');
  assert.equal(root.dark, false);
  onStorage({ key: null, newValue: null });
  assert.equal(root.dataset.character, 'mochi');
  assert.equal(root.dataset.theme, 'system');
});
