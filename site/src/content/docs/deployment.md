---
slug: 'deployment'
title: 'Cloudflare Pages deployment'
group: 'Website'
order: 61
description: 'Build and publish the static portal, then connect your own domain when you are ready.'
---

## Production configuration

The site uses a static Astro build and needs no server adapter, database, runtime secret, or Cloudflare Function. The Pages project remains `mochi-sdk`. Its public production address is [mochi.prabhavalabs.com](https://mochi.prabhavalabs.com/). Cloudflare also assigns the project a `mochi-sdk.pages.dev` provider hostname, but canonical links and social previews should use the official domain.

| Setting           | Value                                                       |
| ----------------- | ----------------------------------------------------------- |
| Repository root   | Repository root (leave the dashboard root field empty)      |
| Build command     | `npm run build`                                             |
| Output directory  | `site/dist`                                                 |
| Node.js           | 22.12 or newer; Node 24 is suitable                         |
| Production branch | `main`                                                      |
| Site URL          | `PUBLIC_SITE_URL`, default `https://mochi.prabhavalabs.com` |

## Deploy from your computer

Install dependencies and authenticate with an account that can write Pages projects:

```sh
npm ci
npx wrangler login
npx wrangler whoami
npm run deploy
```

`npm run deploy` builds the workspace and uploads `site/dist` to the configured project. Wrangler stores authentication locally; never place tokens in source code. The site itself does not need authentication variables.

For a first setup in a different account, create the project first:

```sh
npx wrangler pages project create mochi-sdk --production-branch main
```

If you choose a different project name, update `site/wrangler.jsonc`, the workspace deploy command, and `PUBLIC_SITE_URL`. Project names and domain availability are account/platform dependent.

## Git-connected builds

This repository includes CI build validation. Publishing uses the explicit deployment command above. To enable automatic publishing, connect the GitHub repository in the Cloudflare Pages dashboard and use the settings table. As described in [Cloudflare’s Direct Upload guide](https://developers.cloudflare.com/pages/get-started/direct-upload/), a direct-upload project cannot be converted to Git integration; create a separate Git-connected project if that is your preferred workflow. Alternatively, use a narrowly scoped Pages API token in your own CI secret store and call Wrangler from a deployment job.

Do not put account IDs, API tokens, or local auth files in the public repository. Preview deployments should use a separate branch or project when validating changes before production.

## Official custom domain

The official custom hostname is `mochi.prabhavalabs.com`. In a new Cloudflare account or replacement Pages project, add that hostname under **Custom domains** and follow Cloudflare's DNS verification instructions. Domain/DNS configuration is managed by the domain owner. Set the canonical site URL to the official HTTPS address and rebuild:

```sh
PUBLIC_SITE_URL=https://mochi.prabhavalabs.com npm run deploy
```

Use that same environment variable in a Git-connected build configuration. It controls canonical links, social image URLs, robots, and the sitemap. Do not put a trailing path into it; the site is designed for a domain root, not a subdirectory.

## Verify a deployment

Check the landing page, a nested documentation URL, a missing URL, `/search-index.json`, `/sitemap.xml`, and `/robots.txt`. Verify both themes and reload after choosing another companion. Confirm canonical and social metadata use `https://mochi.prabhavalabs.com/`, and verify the Open Graph and X image URLs return the expected JPEGs. Assets under `/_astro/` use content hashes and long-lived caching. Documents and the search index are not given an immutable cache rule.

Social platforms cache page metadata and images independently. After a metadata or artwork update, publish first, verify the public page source and image URLs, then request a fresh fetch through each platform's preview inspector or debugger. A new versioned image filename is the most predictable way to bypass an old image cache. Clients may still crop or render the same metadata differently.

`public/_headers` applies MIME-sniffing, framing, referrer, and hardware-permission protections. There is no catch-all SPA rewrite, so unknown routes return the custom 404 page. Keep the build artifact free of local settings and device data.

## Roll back

Use the Pages dashboard to roll production back to a previous successful production deployment, or check out a known-good revision and redeploy it. A domain change also needs a rebuild with the appropriate `PUBLIC_SITE_URL`. Test the restored landing page and documentation URLs after rollback.
