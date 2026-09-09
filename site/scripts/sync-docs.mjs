import { readFile, writeFile, mkdir, cp, rm } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import { sources } from './docs-manifest.mjs';
const root = fileURLToPath(new URL('../../', import.meta.url));
const site = path.join(root, 'site');
const generated = path.join(site, 'src/content/docs/generated');
await rm(generated, { recursive: true, force: true });
await mkdir(generated, { recursive: true });
await mkdir(path.join(site, 'public/brand'), { recursive: true });
await mkdir(path.join(site, 'public/media'), { recursive: true });
await cp(path.join(root, 'docs/brand/mochi.png'), path.join(site, 'public/brand/mochi.png'));
await cp(
  path.join(root, 'docs/brand/repository-cover.jpg'),
  path.join(site, 'public/brand/repository-cover.jpg'),
);
await cp(path.join(root, 'docs/images'), path.join(site, 'public/media'), { recursive: true });
const routes = new Map(sources.map(([slug, , , , source]) => [source, `/docs/${slug}/`]));
for (const [slug, title, group, order, source, description] of sources) {
  let content = (await readFile(path.join(root, source), 'utf8')).replace(/^# .+\r?\n/, '');
  // Resolve authored repository links before the Markdown is moved into the collection.
  content = content.replace(/(!?\[[^\]]*\])\(([^\s)]+)\)/g, (match, label, target) => {
    if (/^(https?:|mailto:|#)/.test(target)) return match;
    const [file, hash] = target.split('#');
    const resolved = path.posix.normalize(path.posix.join(path.posix.dirname(source), file));
    const destination =
      routes.get(resolved) ||
      (resolved.startsWith('docs/images/')
        ? `/media/${path.posix.basename(resolved)}`
        : resolved === 'docs/brand/mochi.png'
          ? '/brand/mochi.png'
          : resolved === 'docs/brand/repository-cover.jpg'
            ? '/brand/repository-cover.jpg'
            : `https://github.com/prabhavalabs/mochi/blob/main/${resolved}`);
    return `${label}(${destination}${hash ? `#${hash}` : ''})`;
  });
  const metadata = { slug, title, group, order, description, source };
  await writeFile(
    path.join(generated, `${slug}.md`),
    `---\n${Object.entries(metadata)
      .map(([key, value]) => `${key}: ${JSON.stringify(value)}`)
      .join('\n')}\n---\n${content}`,
  );
}
console.log(`Synced ${sources.length} public guides and brand assets.`);
