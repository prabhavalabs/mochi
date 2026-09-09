export function GET({ site }: { site: URL }) {
  return new Response(`User-agent: *\nAllow: /\nSitemap: ${new URL('/sitemap.xml', site).href}\n`);
}
