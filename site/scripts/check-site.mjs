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
const plainTextLanguages = new Set(['text', 'txt', 'plaintext']);
let highlightedCodeBlocks = 0;
let plainTextBlocks = 0;
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

  for (const match of html.matchAll(/<pre\b([^>]*)>([\s\S]*?)<\/pre>/g)) {
    const [, attributes, code] = match;
    const language = attributes.match(/\bdata-language="([^"]+)"/)?.[1];
    assert.ok(attributes.includes('astro-code'), `${route}: code block is rendered by Shiki`);
    assert.ok(language, `${route}: code block declares its language`);
    assert.ok(code.includes('<code>'), `${route}: code block contains code markup`);

    if (plainTextLanguages.has(language)) {
      plainTextBlocks += 1;
      continue;
    }

    const lightColors = [...code.matchAll(/style="color:([^;\"]+)/g)].map((item) => item[1]);
    const darkColors = [...code.matchAll(/--shiki-dark:([^;\"]+)/g)].map((item) => item[1]);
    assert.ok(lightColors.length > 0, `${route}: ${language} code has highlighted tokens`);
    assert.equal(
      darkColors.length,
      lightColors.length,
      `${route}: ${language} tokens include the dark theme`,
    );
    if (route === '/') {
      assert.ok(
        new Set(lightColors).size > 1,
        `${route}: landing example has distinct token colors`,
      );
      assert.ok(
        new Set(darkColors).size > 1,
        `${route}: landing example keeps distinct token colors in dark mode`,
      );
    }
    highlightedCodeBlocks += 1;
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
  `Validated ${pages.size} pages, ${highlightedCodeBlocks} highlighted code blocks, ${plainTextBlocks} plaintext blocks, internal links and anchors, metadata, and ${search.length} search entries.`,
);
