---
slug: 'website'
title: 'Website maintenance'
group: 'Website'
order: 60
description: 'Develop the Astro portal, maintain its guides, and extend the character-aware React interface.'
---

## Stack and structure

The portal is a static Astro site with React islands, Tailwind CSS 4, and locally owned shadcn/ui components. Markdown is rendered at build time. Interactive controls use Radix primitives through shadcn/ui; local search uses cmdk. DM Sans and Lora are bundled locally through Fontsource.

```text
site/
  src/content/docs/       Website-authored Markdown guides
  src/content.config.ts  Typed content collection
  src/layouts/Base.astro Shared document, header, theme bootstrap, footer
  src/pages/             Landing, docs, search index, sitemap, and 404
  src/components/        Playground, character icons, navigation, search
  src/components/ui/     Owned shadcn/ui components
  src/lib/               Character registry, preferences, renderer, docs helpers
  src/styles/global.css Theme tokens and responsive layouts
  scripts/               Public-document synchronization and validation
  public/                Static headers, favicon, and generated public assets
```

The core SDK and firmware remain separate. Editing website animation code does not change the C++ renderer or board firmware.

## Local setup

From the repository root, use Node.js 22.12 or newer and npm:

```sh
npm ci
npm run dev
```

Open the printed local URL, normally `http://127.0.0.1:4321`. The server binds to your computer. For another port, run `npm run dev --workspace @mochi/website -- --port 4322`.

```sh
npm run check:site
npm run build
npm run test:site
npm run preview
```

Preview serves the production build. The original dependency-free browser demo remains available through `npm run dev:legacy` at port 4173.

## Documentation sources

Canonical SDK, board, and community guides live in their original repository Markdown files. `site/scripts/docs-manifest.mjs` explicitly lists the public files included in the portal. `sync-docs.mjs` copies them into a generated content directory and resolves relative repository links into site or GitHub links. It also copies the approved brand artwork and screenshots.

Sync runs before `dev`, `check`, and `build`. Generated Markdown and copied assets are build outputs; edit the original source instead. Restart the dev server after changing an imported source guide. Each page's **Improve this page** link points to the correct source.

To add a website-authored guide, create `site/src/content/docs/your-page.md`:

```yaml
---
slug: your-page
title: Your page
description: A short, useful summary.
group: SDK
order: 35
---
```

Write `##` headings for the table of contents. Use site-relative links such as `/docs/installation/`. Navigation, search, pagination, and sitemap entries derive from collection metadata. Keep slugs and order values unique. Existing groups are Start here, SDK, Hardware, Website, and Project. The full-text search index is generated from all published pages.

Fence every code sample with its actual language, such as `cpp`, `sh`, `cmake`, `ini`, `json`, or `yaml`, so Shiki can tokenize it. Use `text` only for plain output and directory trees. Markdown samples use the shared GitHub light/dark themes from `astro.config.mjs`; the landing-page example supplies the dark theme for both theme keys because it always appears on a dark panel. The production-site check verifies language IDs, highlighted tokens, and dark-theme token values in every built code block.

## Character personalities

Edit `src/lib/characters.ts` for names, captions, body/face colors, and browser pacing. Edit `global.css` for the corresponding `data-character` accent tokens in light and dark themes. Keep sufficient contrast in both themes. New silhouettes need updates in both `CharacterIcon.tsx` and the Canvas renderer in `lib/animation.js`.

`usePreferences()` shares the selected character and theme across React islands. `setPreferences()` validates and stores changes. The layout applies a small pre-paint bootstrap so the system/saved theme appears before hydration. Keep the bootstrap's accepted values synchronized with the registry when adding characters or modes.

The preference key is `mochi.preferences`; the stored object contains only `character` and `theme`. Storage failures fall back to the current session. A storage event synchronizes open tabs. No user identifier is stored.

## Components and styling

Use the existing source components under `components/ui/`. Add components with the shadcn CLI from the repository root:

```sh
npx shadcn@4.21.0 add @shadcn/tooltip --cwd site
```

Review generated imports and dependency changes. Use `@/lib/utils` for `cn`, and semantic theme tokens for backgrounds, accents, borders, and text. Preserve accessible labels, keyboard behavior, focus indicators, and dialog titles. Avoid remote fonts or images on the critical rendering path.

## Animation lifecycle

The React playground owns its Canvas renderer. It cleans up the animation frame, resize observer, intersection observer, and media-query listener on unmount. It skips drawing when off-screen or the tab is hidden. Reduced motion starts paused; explicit Play is a local opt-in. State selection preserves pause and character selection preserves the current state and timeline.

Browser rendering is a JavaScript visual reference, not a compiled C++ runtime or packaged React SDK. Keep state names and silhouettes aligned with the portable SDK while documenting intentional browser-only behavior.

## Checks and maintenance

Run type checking, the production build, and site validation before a pull request. Validate internal links and anchors, search coverage, theme preferences, and all character/state poses. Manually check a narrow mobile viewport, keyboard navigation, copy buttons, search results/empty state, reload persistence, and OS light/dark/reduced-motion settings.

Use `npm audit` after dependency updates. The lockfile pins the complete installation. A root override pins Sharp to its patched release because the deployment tool's dependency can lag behind; review and remove it when upstream no longer needs it. CI builds the site alongside the SDK and firmware checks.

See [Deployment](/docs/deployment/) for Cloudflare Pages settings and [Privacy & accessibility](/docs/accessibility/) for the public behavior contract.
