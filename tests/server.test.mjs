import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { request } from 'node:http';
import { readFile } from 'node:fs/promises';
import { once } from 'node:events';
import test from 'node:test';

test('local server serves the preview and confines file access', { timeout: 10000 }, async t => {
  const child = spawn(process.execPath, ['scripts/serve.mjs'], {
    cwd: new URL('../', import.meta.url),
    env: { ...process.env, PORT: '0' },
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  t.after(async () => {
    if (child.exitCode === null && child.signalCode === null) {
      const exited = once(child, 'exit');
      child.kill();
      await exited;
    }
  });
  const url = await new Promise((resolve, reject) => {
    let output = '';
    child.once('error', reject);
    child.once('exit', code => reject(new Error(`Server exited: ${code}`)));
    child.stdout.on('data', data => {
      output += data;
      const match = output.match(/http:\/\/127\.0\.0\.1:\d+/);
      if (match) resolve(new URL(match[0]));
    });
  });
  const get = path => new Promise((resolve, reject) => {
    const req = request({ hostname: url.hostname, port: url.port, path }, res => {
      let body = '';
      res.setEncoding('utf8');
      res.on('data', data => { body += data; });
      res.on('end', () => resolve({ status: res.statusCode, body, headers: res.headers }));
    });
    req.once('error', reject);
    req.end();
  });
  const home = await get('/');
  assert.equal(home.status, 200);
  assert.match(home.body, /<title>Mochi/);
  assert.match((await get('/app.js')).headers['content-type'], /javascript/);
  for (const path of ['/../package.json', '/%2e%2e%2fpackage.json', '/.git/config', '/%00', '/%zz']) {
    const response = await get(path);
    assert.ok([403, 404].includes(response.status), `${path}: ${response.status}`);
    assert.doesNotMatch(response.body, /mochi-playground|repositoryformatversion/);
  }
  const css = await readFile(new URL('../web/style.css', import.meta.url), 'utf8');
  assert.doesNotMatch(css, /@import|https?:\/\//);
});
