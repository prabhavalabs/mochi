import { readFile, readdir, stat } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';
import { siteUrl } from '../astro.config.mjs';

const dist = fileURLToPath(new URL('../dist/', import.meta.url));
const site = new URL(siteUrl);
const socialImages = [
  { name: 'social-preview.jpg', width: 1200, height: 630 },
  { name: 'social-preview-x.jpg', width: 1200, height: 600 },
  { name: 'social-square.jpg', width: 1200, height: 1200 },
];

function attribute(tag, name) {
  return tag.match(new RegExp(`\\b${name}="([^"]*)"`))?.[1];
}

function metaContents(html, attributeName, attributeValue) {
  return [...html.matchAll(/<meta\b[^>]*>/g)]
    .map((match) => match[0])
    .filter((tag) => attribute(tag, attributeName) === attributeValue)
    .map((tag) => attribute(tag, 'content'));
}

function jpegDimensions(image) {
  assert.equal(image.readUInt16BE(0), 0xffd8, 'Social image is a JPEG');
  const startOfFrame = new Set([
    0xc0, 0xc1, 0xc2, 0xc3, 0xc5, 0xc6, 0xc7, 0xc9, 0xca, 0xcb, 0xcd, 0xce, 0xcf,
  ]);
  let offset = 2;
  while (offset + 4 < image.length) {
    while (image[offset] === 0xff) offset += 1;
    const marker = image[offset++];
    if (marker === 0xd9 || marker === 0xda) break;
    if (marker === 0x01 || (marker >= 0xd0 && marker <= 0xd7)) continue;
    const length = image.readUInt16BE(offset);
    assert.ok(length >= 2 && offset + length <= image.length, 'Valid JPEG segment');
    if (startOfFrame.has(marker)) {
      return { height: image.readUInt16BE(offset + 3), width: image.readUInt16BE(offset + 5) };
    }
    offset += length;
  }
  assert.fail('JPEG dimensions are present');
}

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
    const canonicalPath = file.endsWith('/404.html') ? '/404/' : route;
    const expectedCanonical = new URL(canonicalPath, site).href;
    const canonicalTags = [...html.matchAll(/<link\b[^>]*>/g)]
      .map((match) => match[0])
      .filter((tag) => attribute(tag, 'rel') === 'canonical');
    assert.equal(canonicalTags.length, 1, `${route}: one canonical URL`);
    assert.equal(attribute(canonicalTags[0], 'href'), expectedCanonical, `${route}: canonical URL`);

    const title = html.match(/<title>([^<]+)<\/title>/)?.[1];
    const description = metaContents(html, 'name', 'description')[0];
    const openGraphImages = [
      new URL('/brand/social-preview.jpg', site).href,
      new URL('/brand/social-square.jpg', site).href,
    ];
    assert.ok(title, `${route}: page title`);
    assert.ok(description, `${route}: page description`);
    assert.deepEqual(metaContents(html, 'property', 'og:site_name'), ['Mochi']);
    assert.deepEqual(metaContents(html, 'property', 'og:locale'), ['en_US']);
    assert.deepEqual(metaContents(html, 'property', 'og:type'), ['website']);
    assert.deepEqual(metaContents(html, 'property', 'og:title'), [title]);
    assert.deepEqual(metaContents(html, 'property', 'og:description'), [description]);
    assert.deepEqual(metaContents(html, 'property', 'og:url'), [expectedCanonical]);
    assert.deepEqual(metaContents(html, 'property', 'og:image'), openGraphImages);
    assert.deepEqual(metaContents(html, 'property', 'og:image:width'), ['1200', '1200']);
    assert.deepEqual(metaContents(html, 'property', 'og:image:height'), ['630', '1200']);
    assert.deepEqual(metaContents(html, 'property', 'og:image:type'), ['image/jpeg', 'image/jpeg']);
    assert.equal(metaContents(html, 'property', 'og:image:alt').length, 2);
    assert.deepEqual(metaContents(html, 'name', 'twitter:card'), ['summary_large_image']);
    assert.deepEqual(metaContents(html, 'name', 'twitter:title'), [title]);
    assert.deepEqual(metaContents(html, 'name', 'twitter:description'), [description]);
    assert.deepEqual(metaContents(html, 'name', 'twitter:image'), [
      new URL('/brand/social-preview-x.jpg', site).href,
    ]);
    assert.equal(metaContents(html, 'name', 'twitter:image:alt').length, 1);
    for (const image of [...openGraphImages, ...metaContents(html, 'name', 'twitter:image')]) {
      assert.equal(new URL(image).protocol, 'https:', `${route}: social image uses HTTPS`);
      assert.equal(new URL(image).origin, site.origin, `${route}: social image uses site origin`);
    }
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

for (const expected of socialImages) {
  const image = await readFile(path.join(dist, 'brand', expected.name));
  assert.ok(image.length <= 1_000_000, `${expected.name}: no larger than 1 MB`);
  assert.deepEqual(jpegDimensions(image), {
    width: expected.width,
    height: expected.height,
  });
}

const expectedSitemap = new Set([
  new URL('/', site).href,
  ...search.map((item) => new URL(`/docs/${item.slug}/`, site).href),
]);
const sitemap = await readFile(path.join(dist, 'sitemap.xml'), 'utf8');
const sitemapLocations = [...sitemap.matchAll(/<loc>([^<]+)<\/loc>/g)].map((match) => match[1]);
assert.deepEqual(
  new Set(sitemapLocations),
  expectedSitemap,
  'Sitemap uses the configured site origin',
);
const robots = await readFile(path.join(dist, 'robots.txt'), 'utf8');
assert.ok(
  robots.includes(`Sitemap: ${new URL('/sitemap.xml', site).href}`),
  'robots.txt uses the configured sitemap origin',
);
assert.equal(failures.length, 0, failures.join('\n'));
console.log(
  `Validated ${pages.size} pages, ${highlightedCodeBlocks} highlighted code blocks, ${plainTextBlocks} plaintext blocks, social metadata, internal links and anchors, and ${search.length} search entries.`,
);
