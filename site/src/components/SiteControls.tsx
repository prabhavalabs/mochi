import { useEffect, useState } from 'react';
import { CharacterPicker } from './Playground';
import { Moon, Sun, Monitor, Menu, Search, ArrowUpRight, FileText } from 'lucide-react';
import { Button } from './ui/button';
import {
  DropdownMenu,
  DropdownMenuTrigger,
  DropdownMenuContent,
  DropdownMenuRadioGroup,
  DropdownMenuRadioItem,
  DropdownMenuLabel,
  DropdownMenuSeparator,
} from './ui/dropdown-menu';
import {
  Sheet,
  SheetTrigger,
  SheetContent,
  SheetHeader,
  SheetTitle,
  SheetDescription,
} from './ui/sheet';
import { Dialog, DialogContent, DialogTitle, DialogDescription } from './ui/dialog';
import {
  Command,
  CommandInput,
  CommandList,
  CommandEmpty,
  CommandGroup,
  CommandItem,
} from './ui/command';
import { usePreferences, setPreferences, type Theme } from '@/lib/preferences';
export type NavEntry = { slug: string; title: string; group: string };
export function ThemeControl() {
  const { theme } = usePreferences();
  const Icon = theme === 'system' ? Monitor : theme === 'dark' ? Moon : Sun;
  return (
    <DropdownMenu>
      <DropdownMenuTrigger asChild>
        <Button variant="ghost" size="icon" aria-label={`Color theme: ${theme}`}>
          <Icon />
        </Button>
      </DropdownMenuTrigger>
      <DropdownMenuContent align="end">
        <DropdownMenuLabel>Make yourself comfortable</DropdownMenuLabel>
        <DropdownMenuSeparator />
        <DropdownMenuRadioGroup
          value={theme}
          onValueChange={(value) => setPreferences({ theme: value as Theme })}
        >
          {(['light', 'dark', 'system'] as const).map((value) => (
            <DropdownMenuRadioItem value={value} key={value}>
              {value === 'system'
                ? 'Use system setting'
                : `${value[0].toUpperCase()}${value.slice(1)} theme`}
            </DropdownMenuRadioItem>
          ))}
        </DropdownMenuRadioGroup>
      </DropdownMenuContent>
    </DropdownMenu>
  );
}
export function NavLinks({ entries, current }: { entries: NavEntry[]; current?: string }) {
  return (
    <>
      {[...new Set(entries.map((e) => e.group))].map((group) => (
        <div className="nav-group" key={group}>
          <h2>{group}</h2>
          {entries
            .filter((e) => e.group === group)
            .map((e) => (
              <a
                key={e.slug}
                href={`/docs/${e.slug}/`}
                aria-current={current === e.slug ? 'page' : undefined}
              >
                {e.title}
              </a>
            ))}
        </div>
      ))}
    </>
  );
}
export function MobileNavigation({ entries, current }: { entries: NavEntry[]; current?: string }) {
  return (
    <Sheet>
      <SheetTrigger asChild>
        <Button variant="ghost" size="icon" className="mobile-menu" aria-label="Open navigation">
          <Menu />
        </Button>
      </SheetTrigger>
      <SheetContent side="left" className="overflow-y-auto">
        <SheetHeader>
          <SheetTitle>Make yourself at home.</SheetTitle>
          <SheetDescription>The Mochi field guide.</SheetDescription>
        </SheetHeader>
        <nav aria-label="Mobile navigation" className="mobile-nav">
          <p className="eyebrow">Your companion</p>
          <CharacterPicker compact />
          <a href="/">Playground</a>
          <a href="https://github.com/prabhavalabs/mochi">
            GitHub <ArrowUpRight size={14} />
          </a>
          <NavLinks entries={entries} current={current} />
        </nav>
      </SheetContent>
    </Sheet>
  );
}
type SearchEntry = NavEntry & { description: string; text: string };
export function SearchDocs() {
  const [open, setOpen] = useState(false);
  const [entries, setEntries] = useState<SearchEntry[]>([]);
  const [status, setStatus] = useState('');
  useEffect(() => {
    const handle = (e: KeyboardEvent) => {
      if ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === 'k') {
        e.preventDefault();
        setOpen((value) => !value);
      }
    };
    document.addEventListener('keydown', handle);
    return () => document.removeEventListener('keydown', handle);
  }, []);
  useEffect(() => {
    if (!open || entries.length) return;
    const controller = new AbortController();
    setStatus('Loading the guide…');
    fetch('/search-index.json', { signal: controller.signal })
      .then((r) => {
        if (!r.ok) throw new Error('Search unavailable');
        return r.json();
      })
      .then((data) => {
        setEntries(data);
        setStatus('');
      })
      .catch((error) => {
        if (error.name !== 'AbortError')
          setStatus('Search couldn’t load. Close and reopen to retry, or browse the guide.');
      });
    return () => controller.abort();
  }, [open, entries.length]);
  return (
    <>
      <Button variant="outline" className="search-trigger" onClick={() => setOpen(true)}>
        <Search />
        <span>Search documentation…</span>
        <kbd>⌘ K</kbd>
      </Button>
      <Dialog open={open} onOpenChange={setOpen}>
        <DialogContent className="p-0 overflow-hidden" showCloseButton={false}>
          <DialogTitle className="sr-only">Search documentation</DialogTitle>
          <DialogDescription className="sr-only">
            Search the entire guide. Use arrow keys to select a result and Enter to open it.
          </DialogDescription>
          <Command
            filter={(value, query, keywords) => {
              const terms = query.toLowerCase().split(/\s+/).filter(Boolean);
              const title = value.toLowerCase();
              const text = (keywords || []).join(' ').toLowerCase();
              return terms.every((term) => title.includes(term) || text.includes(term))
                ? title.includes(query.toLowerCase())
                  ? 1
                  : 0.5
                : 0;
            }}
          >
            <CommandInput placeholder="What would you like to make?" />
            <CommandList>
              <CommandEmpty>
                {status || 'No pages found. Try “rendering”, “touch”, or “install”.'}
              </CommandEmpty>
              <CommandGroup heading={status || 'The field guide'}>
                {entries.map((e) => (
                  <CommandItem
                    key={e.slug}
                    value={e.title}
                    keywords={[e.description, e.text]}
                    onSelect={() => {
                      window.location.assign(`/docs/${e.slug}/`);
                    }}
                  >
                    <FileText />
                    <span>
                      <strong className="block font-medium">{e.title}</strong>
                      <small className="text-muted-foreground">{e.group}</small>
                    </span>
                  </CommandItem>
                ))}
              </CommandGroup>
            </CommandList>
          </Command>
          <p className="search-footer">↑ ↓ to explore · Enter to open · Esc to close</p>
        </DialogContent>
      </Dialog>
    </>
  );
}
