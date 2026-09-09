import { useEffect, useRef, useState } from 'react';
import { ArrowRight, Play, Pause, Eye, RotateCcw, Check } from 'lucide-react';
import { Button } from './ui/button';
import { ToggleGroup, ToggleGroupItem } from './ui/toggle-group';
import { Slider } from './ui/slider';
import { Switch } from './ui/switch';
import CharacterIcon from './CharacterIcon';
import { characters, characterById } from '@/lib/characters';
import { usePreferences, setPreferences } from '@/lib/preferences';
import { Mochi, STATES } from '@/lib/animation';

export function CharacterPicker({ compact = false }: { compact?: boolean }) {
  const { character } = usePreferences();
  return (
    <ToggleGroup
      type="single"
      value={character}
      onValueChange={(value) => {
        if (value) setPreferences({ character: characterById(value).id });
      }}
      aria-label="Choose your companion"
      spacing={3}
      className={compact ? 'character-picker compact' : 'character-picker'}
    >
      {characters.map((c) => (
        <ToggleGroupItem
          key={c.id}
          value={c.id}
          className="character-choice"
          aria-label={`Choose ${c.name}`}
        >
          <CharacterIcon id={c.id} />
          <span>
            <strong>{c.name}</strong>
            {!compact && <small>{c.short}</small>}
          </span>
          {!compact && <Check className="selection-check" />}
        </ToggleGroupItem>
      ))}
    </ToggleGroup>
  );
}

export function Companion({ callout = false }: { callout?: boolean }) {
  const { character } = usePreferences();
  const c = characterById(character);
  return callout ? (
    <div className="companion-greeting">
      <CharacterIcon id={character} />
      <p>{c.greeting}</p>
    </div>
  ) : (
    <div className="doc-companion">
      <span className="eyebrow">Your companion</span>
      <CharacterIcon id={character} />
      <h3>{c.name}</h3>
      <p>{c.short}.</p>
      <CharacterPicker compact />
      <a href="/docs/characters/">
        Meet all four characters <ArrowRight size={14} />
      </a>
      <blockquote>“{c.quote}”</blockquote>
    </div>
  );
}

export default function Playground() {
  const { character } = usePreferences();
  const c = characterById(character);
  const canvas = useRef<HTMLCanvasElement>(null);
  const renderer = useRef<Mochi | null>(null);
  const [state, setState] = useState('wave');
  const [paused, setPaused] = useState(false);
  const [speed, setSpeed] = useState(1);
  const [tour, setTour] = useState(false);
  const [reduced, setReduced] = useState(false);
  const [ready, setReady] = useState(false);
  const pointer = useRef<{ x: number; y: number } | null>(null);
  useEffect(() => {
    if (!canvas.current) return;
    const media = matchMedia('(prefers-reduced-motion: reduce)');
    const instance = new Mochi(canvas.current, { reduced: media.matches });
    renderer.current = instance;
    instance.setState('wave');
    instance.setPaused(media.matches);
    instance.resize();
    setReady(true);
    const onMotion = () => {
      setReduced(media.matches);
      setPaused(media.matches);
      if (media.matches) setTour(false);
      instance.reduced = media.matches;
      instance.setState(instance.state);
    };
    onMotion();
    media.addEventListener('change', onMotion);
    const observer = new IntersectionObserver((entries) => {
      instance.visible = entries[0].isIntersecting;
      instance.lastTime = null;
    });
    observer.observe(canvas.current);
    return () => {
      media.removeEventListener('change', onMotion);
      observer.disconnect();
      instance.destroy();
      renderer.current = null;
    };
  }, []);
  useEffect(() => {
    if (renderer.current) {
      renderer.current.character = character;
      renderer.current.draw();
    }
  }, [character]);
  useEffect(() => {
    renderer.current?.setState(state);
  }, [state]);
  useEffect(() => {
    if (renderer.current) {
      renderer.current.reduced = reduced && paused;
      renderer.current.setPaused(paused);
    }
  }, [paused, reduced]);
  useEffect(() => {
    if (renderer.current) renderer.current.speed = speed;
  }, [speed]);
  useEffect(() => {
    if (!tour || paused) return;
    const timer = window.setInterval(() => {
      if (!document.hidden && renderer.current?.visible)
        setState(
          (current) => STATES[(STATES.findIndex((s) => s.id === current) + 1) % STATES.length].id,
        );
    }, 5000);
    return () => clearInterval(timer);
  }, [tour, paused]);
  const select = (id: string) => {
    setState(id);
    setTour(false);
  };
  const detail = STATES.find((s) => s.id === state)!;
  return (
    <>
      <section className="hero" aria-labelledby="hero-heading">
        <div className="hero-copy">
          <p className="eyebrow">
            <span className="tiny-dot" /> Open source. Full of character.
          </p>
          <h1 id="hero-heading">
            A little company.
            <br />
            <em>A lot of personality.</em>
          </h1>
          <p className="hero-description">
            Four little companions for your next big idea. An open-source animation SDK for embedded
            screens.
          </p>
          <div className="hero-actions">
            <Button asChild size="lg">
              <a href="/docs/installation/">
                Get started <ArrowRight />
              </a>
            </Button>
            <Button asChild variant="outline" size="lg">
              <a href="/docs/introduction/">Explore the SDK</a>
            </Button>
          </div>
          <p className="hero-footnote">C++17 · Any RGB565 display · Yours to make your own</p>
        </div>
        <div className="mascot-stage" id="playground">
          <div className="stage-top">
            <span>
              <span className="tiny-dot" />{' '}
              {paused ? 'Taking a little pause' : 'A little life, in real time'}
            </span>
            <span>0.3.0</span>
          </div>
          <button
            type="button"
            className="canvas-button"
            aria-label={`Blink ${c.name}. Swipe left or right to change state.`}
            onPointerDown={(event) => {
              pointer.current = { x: event.clientX, y: event.clientY };
              event.currentTarget.setPointerCapture(event.pointerId);
            }}
            onPointerCancel={() => {
              pointer.current = null;
            }}
            onPointerUp={(event) => {
              if (!pointer.current) return;
              const dx = event.clientX - pointer.current.x,
                dy = event.clientY - pointer.current.y;
              pointer.current = null;
              if (Math.abs(dx) > 55 && Math.abs(dx) > Math.abs(dy))
                select(
                  STATES[
                    (STATES.findIndex((s) => s.id === state) + (dx < 0 ? 1 : STATES.length - 1)) %
                      STATES.length
                  ].id,
                );
              else if (Math.hypot(dx, dy) < 25) renderer.current?.blink();
            }}
            onClick={(event) => {
              if (event.detail === 0) renderer.current?.blink();
            }}
          >
            {!ready && <CharacterIcon id={character} className="canvas-fallback" />}
            <canvas
              ref={canvas}
              aria-label={`${c.name} is ${detail.name.toLowerCase()}`}
              role="img"
            />
          </button>
          <div className="stage-caption" aria-live="polite">
            <h2>{c.name}</h2>
            <p>{c.detail}</p>
            <span>{detail.detail}</span>
          </div>
        </div>
      </section>
      <section className="cast-section" aria-label="Character selection">
        <CharacterPicker />
        <p className="selection-hint">Pick a companion. Make yourself at home.</p>
      </section>
      <section className="states-section" aria-labelledby="states-heading">
        <div className="section-heading">
          <div>
            <h2 id="states-heading">A feeling for every moment.</h2>
            <p>Bring {c.name} to life. Choose a state and see what happens.</p>
          </div>
          <Button
            variant="outline"
            onClick={() => setPaused((p) => !p)}
            aria-label={paused ? 'Play animation' : 'Pause animation'}
          >
            {paused ? <Play /> : <Pause />}
            {paused ? 'Play' : 'Pause'}
          </Button>
        </div>
        <ToggleGroup
          type="single"
          value={state}
          onValueChange={(value) => {
            if (value) select(value);
          }}
          aria-label="Animation state"
          spacing={2}
          className="state-picker"
        >
          {STATES.map((s) => (
            <ToggleGroupItem key={s.id} value={s.id}>
              {s.name}
            </ToggleGroupItem>
          ))}
        </ToggleGroup>
        <div className="playback-controls">
          <Button
            variant="ghost"
            disabled={paused || state === 'sleeping'}
            onClick={() => renderer.current?.blink()}
          >
            <Eye /> Blink once
          </Button>
          <Button
            variant="ghost"
            onClick={() => {
              if (renderer.current) {
                renderer.current.time = 0;
                renderer.current.setState(state);
                renderer.current.draw();
              }
            }}
          >
            <RotateCcw /> Restart
          </Button>
          <div className="speed-control">
            <label htmlFor="animation-speed">
              Speed <span>{speed.toFixed(1)}×</span>
            </label>
            <Slider
              id="animation-speed"
              aria-label="Animation speed"
              min={0.5}
              max={2}
              step={0.1}
              value={[speed]}
              onValueChange={([value]) => setSpeed(value)}
            />
          </div>
          <label className="tour-control" htmlFor="auto-tour">
            <Switch
              id="auto-tour"
              checked={tour}
              onCheckedChange={(value) => {
                setTour(value);
                if (value) setPaused(false);
              }}
            />
            Tour all states
          </label>
        </div>
        <p className="playback-note">
          {reduced
            ? 'Reduced motion is on. Press Play to animate, or explore still poses.'
            : 'Tap the character to blink, or swipe to change states.'}{' '}
          Speaking is a visual animation; no audio is recorded or played.
        </p>
      </section>
    </>
  );
}
