import http from 'node:http';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const root = path.resolve(fileURLToPath(new URL('../web/', import.meta.url)));
const types = { '.html': 'text/html', '.css': 'text/css', '.js': 'text/javascript', '.svg': 'image/svg+xml' };
const port = Number(process.env.PORT || 4173);
const server = http.createServer(async (req, res) => {
  try {
    const route = decodeURIComponent(new URL(req.url, 'http://localhost').pathname);
    const target = path.resolve(root, '.' + (route === '/' ? '/index.html' : route));
    if (!target.startsWith(root + path.sep)) { res.writeHead(403).end(); return; }
    const content = await readFile(target);
    res.writeHead(200, { 'Content-Type': `${types[path.extname(target)] || 'application/octet-stream'}; charset=utf-8`, 'Cache-Control': 'no-store' });
    res.end(content);
  } catch { res.writeHead(404).end('Not found'); }
});
server.listen(port, '127.0.0.1', () => console.log(`Mochi playground: http://127.0.0.1:${server.address().port}`));
