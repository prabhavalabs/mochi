import { STATES } from './animation';

export type DeviceView = 'stage' | 'characters' | 'moods';

export const DEVICE_PACES = [0.5, 1, 1.5] as const;

export function adjacentStateId(current: string, direction: -1 | 1) {
  const index = STATES.findIndex((state) => state.id === current);
  if (index < 0) throw new RangeError(`Unknown state: ${current}`);
  return STATES[(index + direction + STATES.length) % STATES.length].id;
}

export function nextDevicePace(current: number) {
  return DEVICE_PACES.find((pace) => pace > current + Number.EPSILON) ?? DEVICE_PACES[0];
}

export function previewShouldPause(paused: boolean, view: DeviceView) {
  return paused || view !== 'stage';
}
