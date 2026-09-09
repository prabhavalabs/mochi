import { readFile, readdir, stat } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';
const dist = fileURLToPath(new URL('../dist/', import.meta.url));
async function walk(dir) {
  return (
    await Promise.all(
      (await readdir(dir, { withFileTypes: true })).map((e) =>
        e.isDirectory() ? walk(path.join(dir, e.name)) : path.join(dir, e.name),
      ),
    )
  ).flat();
}
const htmlFiles = (await walk(dist)).filter((f) => f.endsWith('.html'));
const pages = new Map(
  await Promise.all(htmlFiles.map(async (file) => [file, await readFile(file, 'utf8')])),
);
const failures = [];
for (const [file, html] of pages) {
  const route =
    '/' +
    path
      .relative(dist, file)
      .replaceAll(path.sep, '/')
      .replace(/index\.html$/, '');
  for (const [, value] of html.matchAll(/(?:href|src)="([^"]+)"/g)) {
    if (/^(https?:|mailto:|data:|tel:)/.test(value)) continue;
    const url = new URL(value.replaceAll('&amp;', '&'), 'https://local.invalid' + route);
    const target = path.join(dist, decodeURIComponent(url.pathname));
    let targetFile = target;
    try {
      if ((await stat(target)).isDirectory()) targetFile = path.join(target, 'index.html');
      await stat(targetFile);
    } catch {
      failures.push(`${route}: missing ${url.pathname}`);
      continue;
    }
    if (url.hash && targetFile.endsWith('.html')) {
      const targetHtml = pages.get(targetFile) || (await readFile(targetFile, 'utf8'));
      const ids = [...targetHtml.matchAll(/\bid="([^"]+)"/g)].map((m) => m[1]);
      if (!ids.includes(decodeURIComponent(url.hash.slice(1))))
        failures.push(`${route}: missing anchor ${url.pathname}${url.hash}`);
    }
  }
  if (!file.endsWith('/docs/index.html')) {
    assert.equal((html.match(/<h1(?:\s|>)/g) || []).length, 1, `${route}: one page heading`);
    assert.ok(html.includes('rel="canonical"'), `${route}: canonical URL`);
    assert.ok(html.includes('name="description"'), `${route}: page description`);
  }
}
const search = JSON.parse(await readFile(path.join(dist, 'search-index.json'), 'utf8'));
assert.equal(new Set(search.map((p) => p.slug)).size, search.length, 'Unique guide routes');
assert.ok(search.length >= 26, 'The complete guide is indexed');
for (const item of search) {
  assert.ok(item.text.length > 200, `${item.slug}: full-text search content`);
  assert.ok(
    pages.has(path.join(dist, 'docs', item.slug, 'index.html')),
    `${item.slug}: searchable page exists`,
  );
}
assert.equal(failures.length, 0, failures.join('\n'));
console.log(
  `Validated ${pages.size} pages, internal links and anchors, metadata, and ${search.length} search entries.`,
);
