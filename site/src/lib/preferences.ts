import { useSyncExternalStore } from 'react';
import { characters, type CharacterId } from './characters';
export type Theme = 'system' | 'light' | 'dark';
type Preferences = { character: CharacterId; theme: Theme };
const defaults: Preferences = { character: 'mochi', theme: 'system' };
let current = defaults;
const listeners = new Set<() => void>();
export function parsePreferences(value: string | null): Preferences {
  try {
    const data = JSON.parse(value || '{}');
    return {
      character: characters.some((c) => c.id === data?.character) ? data.character : 'mochi',
      theme: ['light', 'dark', 'system'].includes(data?.theme) ? data.theme : 'system',
    };
  } catch {
    return defaults;
  }
}
function apply() {
  document.documentElement.dataset.character = current.character;
  document.documentElement.classList.toggle(
    'dark',
    current.theme === 'dark' ||
      (current.theme === 'system' && matchMedia('(prefers-color-scheme: dark)').matches),
  );
  document.documentElement.dataset.theme = current.theme;
}
if (typeof window !== 'undefined') {
  try {
    current = parsePreferences(localStorage.getItem('mochi.preferences'));
  } catch {
    /* Storage is optional. */
  }
  apply();
  matchMedia('(prefers-color-scheme: dark)').addEventListener('change', apply);
  window.addEventListener('storage', (event) => {
    if (event.key !== 'mochi.preferences' && event.key !== null) return;
    current = parsePreferences(event.newValue);
    apply();
    listeners.forEach((fn) => fn());
  });
}
export function setPreferences(patch: Partial<Preferences>) {
  current = parsePreferences(JSON.stringify({ ...current, ...patch }));
  try {
    localStorage.setItem('mochi.preferences', JSON.stringify(current));
  } catch {
    /* Keep the session usable in private/restricted storage. */
  }
  apply();
  listeners.forEach((fn) => fn());
}
export function usePreferences() {
  return useSyncExternalStore(
    (fn) => {
      listeners.add(fn);
      return () => {
        listeners.delete(fn);
      };
    },
    () => current,
    () => defaults,
  );
}
