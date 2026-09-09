import { useEffect, useRef, type RefObject } from 'react';
import { Check, ChevronDown, ChevronLeft, ChevronRight, Pause, Play, X } from 'lucide-react';
import { Button } from './ui/button';
import CharacterIcon from './CharacterIcon';
import { characters, characterById } from '@/lib/characters';
import { STATES } from '@/lib/animation';
import { nextDevicePace, type DeviceView } from '@/lib/device-preview-state';

type DevicePreviewProps = {
  canvas: RefObject<HTMLCanvasElement | null>;
  character: string;
  state: string;
  paused: boolean;
  speed: number;
  tour: boolean;
  ready: boolean;
  view: DeviceView;
  onViewChange: (view: DeviceView) => void;
  onCharacterChange: (id: string) => void;
  onStateChange: (id: string) => void;
  onPausedChange: (paused: boolean) => void;
  onSpeedChange: (speed: number) => void;
  onTourChange: (tour: boolean) => void;
  onBlink: () => void;
};

export default function DevicePreview({
  canvas,
  character,
  state,
  paused,
  speed,
  tour,
  ready,
  view,
  onViewChange,
  onCharacterChange,
  onStateChange,
  onPausedChange,
  onSpeedChange,
  onTourChange,
  onBlink,
}: DevicePreviewProps) {
  const c = characterById(character);
  const detail = STATES.find((candidate) => candidate.id === state) ?? STATES[0];
  const pointerStart = useRef<{ x: number; y: number } | null>(null);
  const opener = useRef<HTMLButtonElement | null>(null);
  const characterFocus = useRef<HTMLButtonElement | null>(null);
  const moodFocus = useRef<HTMLButtonElement | null>(null);

  useEffect(() => {
    if (view === 'stage') return;
    const frame = requestAnimationFrame(() =>
      (view === 'characters' ? characterFocus.current : moodFocus.current)?.focus(),
    );
    return () => cancelAnimationFrame(frame);
  }, [view]);

  const openView = (next: Exclude<DeviceView, 'stage'>, button: HTMLButtonElement) => {
    opener.current = button;
    onViewChange(next);
  };

  const closeView = () => {
    onViewChange('stage');
    requestAnimationFrame(() => opener.current?.focus());
  };

  const chooseCharacter = (id: string) => {
    onCharacterChange(id);
    closeView();
  };

  const chooseState = (id: string) => {
    onStateChange(id);
    closeView();
  };

  const stepWithPhysicalButton = (direction: 'previous' | 'next') => {
    onStateChange(direction);
    if (view !== 'stage') onViewChange('stage');
  };

  return (
    <div className="device-preview-wrap" id="playground">
      <div className="device-preview-meta">
        <span>Interactive touchscreen</span>
      </div>
      <div className="device-scene">
        <div className="device-assembly">
          <div className="device-top-buttons" role="group" aria-label="Physical device controls">
            <button type="button" onClick={() => stepWithPhysicalButton('next')} title="Next mood">
              <span aria-hidden="true">+</span>
              <span className="sr-only">Next mood</span>
            </button>
            <button
              type="button"
              onClick={() => onPausedChange(!paused)}
              title={paused ? 'Play animation' : 'Pause animation'}
            >
              {paused ? <Play /> : <Pause />}
              <span className="sr-only">{paused ? 'Play animation' : 'Pause animation'}</span>
            </button>
            <button
              type="button"
              onClick={() => stepWithPhysicalButton('previous')}
              title="Previous mood"
            >
              <span aria-hidden="true">−</span>
              <span className="sr-only">Previous mood</span>
            </button>
          </div>

          <div className="device-body">
            <div className="device-face">
              <div
                className="device-screen"
                onKeyDown={(event) => {
                  if (event.key === 'Escape' && view !== 'stage') {
                    event.stopPropagation();
                    closeView();
                  }
                }}
              >
                <section
                  className="device-stage"
                  data-visible={view === 'stage'}
                  inert={view !== 'stage'}
                  aria-hidden={view !== 'stage'}
                  aria-label={`${c.name} animation stage`}
                >
                  <header className="device-screen-header">
                    <button
                      type="button"
                      className="device-character-trigger"
                      onClick={(event) => openView('characters', event.currentTarget)}
                      aria-haspopup="dialog"
                      aria-expanded={view === 'characters'}
                    >
                      {c.name} <ChevronDown />
                    </button>
                    <button
                      type="button"
                      className="device-tour-button"
                      data-active={tour}
                      onClick={() => onTourChange(!tour)}
                      aria-pressed={tour}
                    >
                      {tour ? 'Tour on' : 'Tour'}
                    </button>
                  </header>

                  <button
                    type="button"
                    className="device-canvas-button"
                    aria-label={`Blink ${c.name}. Swipe left or right to change mood.`}
                    onPointerDown={(event) => {
                      pointerStart.current = { x: event.clientX, y: event.clientY };
                      event.currentTarget.setPointerCapture(event.pointerId);
                    }}
                    onPointerCancel={() => {
                      pointerStart.current = null;
                    }}
                    onPointerUp={(event) => {
                      if (!pointerStart.current) return;
                      const dx = event.clientX - pointerStart.current.x;
                      const dy = event.clientY - pointerStart.current.y;
                      pointerStart.current = null;
                      if (Math.abs(dx) > 55 && Math.abs(dx) > Math.abs(dy))
                        onStateChange(dx < 0 ? 'next' : 'previous');
                      else if (Math.hypot(dx, dy) < 25) onBlink();
                    }}
                    onClick={(event) => {
                      if (event.detail === 0) onBlink();
                    }}
                  >
                    {!ready ? <CharacterIcon id={character} className="canvas-fallback" /> : null}
                    <canvas
                      ref={canvas}
                      aria-label={`${c.name} is ${detail.name.toLowerCase()}`}
                      role="img"
                    />
                  </button>

                  <div className="device-stage-caption" aria-live="polite">
                    <span className="sr-only">{c.name}: </span>
                    <strong>{detail.name}</strong>
                    <span>{detail.detail}</span>
                  </div>

                  <footer className="device-stage-controls">
                    <button
                      type="button"
                      onClick={() => onStateChange('previous')}
                      aria-label="Previous mood"
                    >
                      <ChevronLeft />
                    </button>
                    <button
                      type="button"
                      ref={(node) => {
                        if (view === 'stage') opener.current = opener.current ?? node;
                      }}
                      onClick={(event) => openView('moods', event.currentTarget)}
                      aria-haspopup="dialog"
                      aria-expanded={view === 'moods'}
                    >
                      Choose a mood
                    </button>
                    <button
                      type="button"
                      onClick={() => onStateChange('next')}
                      aria-label="Next mood"
                    >
                      <ChevronRight />
                    </button>
                  </footer>
                </section>

                <section
                  className="device-menu device-character-menu"
                  data-visible={view === 'characters'}
                  inert={view !== 'characters'}
                  aria-hidden={view !== 'characters'}
                  role="dialog"
                  aria-modal="false"
                  aria-labelledby="device-character-menu-title"
                >
                  <header className="device-menu-header">
                    <h2 id="device-character-menu-title">Meet your buddies</h2>
                    <Button
                      variant="ghost"
                      size="icon"
                      onClick={closeView}
                      aria-label="Close character picker"
                    >
                      <X />
                    </Button>
                  </header>
                  <div className="device-character-grid">
                    {characters.map((candidate) => {
                      const selected = candidate.id === character;
                      return (
                        <button
                          type="button"
                          key={candidate.id}
                          ref={selected ? characterFocus : undefined}
                          className="device-character-card"
                          data-selected={selected}
                          onClick={() => chooseCharacter(candidate.id)}
                          aria-pressed={selected}
                        >
                          <CharacterIcon id={candidate.id} />
                          <strong>{candidate.name}</strong>
                          <small>{selected ? 'Selected' : candidate.short}</small>
                          {selected ? <Check className="device-selection-check" /> : null}
                        </button>
                      );
                    })}
                  </div>
                  <p>Pick a friend. Keep your mood.</p>
                </section>

                <section
                  className="device-menu device-mood-menu"
                  data-visible={view === 'moods'}
                  inert={view !== 'moods'}
                  aria-hidden={view !== 'moods'}
                  role="dialog"
                  aria-modal="false"
                  aria-labelledby="device-mood-menu-title"
                >
                  <header className="device-menu-header">
                    <h2 id="device-mood-menu-title">Choose a mood</h2>
                    <Button
                      variant="ghost"
                      size="icon"
                      onClick={closeView}
                      aria-label="Close mood picker"
                    >
                      <X />
                    </Button>
                  </header>
                  <div className="device-mood-grid">
                    {STATES.map((candidate) => {
                      const selected = candidate.id === state;
                      return (
                        <button
                          type="button"
                          key={candidate.id}
                          ref={selected ? moodFocus : undefined}
                          data-selected={selected}
                          onClick={() => chooseState(candidate.id)}
                          aria-pressed={selected}
                        >
                          <span>{candidate.name}</span>
                          {selected ? <Check /> : null}
                        </button>
                      );
                    })}
                  </div>
                  <div className="device-playback-row">
                    <button
                      type="button"
                      onClick={() => onPausedChange(!paused)}
                      aria-pressed={paused}
                    >
                      {paused ? <Play /> : <Pause />}
                      {paused ? 'Play' : 'Pause'}
                    </button>
                    <button type="button" onClick={() => onTourChange(!tour)} aria-pressed={tour}>
                      Tour{tour ? ' on' : ''}
                    </button>
                    <button type="button" onClick={() => onSpeedChange(nextDevicePace(speed))}>
                      {speed === 0.5 ? '.5' : speed.toFixed(1).replace('.0', '')}× pace
                    </button>
                  </div>
                </section>
              </div>
            </div>
          </div>
          <div className="device-usb" aria-hidden="true">
            <span />
          </div>
        </div>
      </div>
    </div>
  );
}
