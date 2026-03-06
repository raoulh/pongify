// Pongify Live — Cloudflare Worker
// Stores encrypted tournament data in KV, serves the mobile webapp SPA.
// Deployed automatically by pongify-setup.exe via the Cloudflare API.
//
// KV binding: env.KV
// Secret:     env.ADMIN_SECRET

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

async function sha256(message) {
  const data = new TextEncoder().encode(message);
  const hash = await crypto.subtle.digest('SHA-256', data);
  return [...new Uint8Array(hash)].map(b => b.toString(16).padStart(2, '0')).join('');
}

function corsHeaders(extra = {}) {
  return {
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Expose-Headers': 'X-Updated-At',
    ...extra,
  };
}

function jsonResponse(body, status = 200, extra = {}) {
  return new Response(JSON.stringify(body), {
    status,
    headers: { 'Content-Type': 'application/json', ...corsHeaders(extra) },
  });
}

function textResponse(text, status = 200, extra = {}) {
  return new Response(text, { status, headers: corsHeaders(extra) });
}

// ---------------------------------------------------------------------------
// CORS preflight
// ---------------------------------------------------------------------------

function handleOptions() {
  return new Response(null, {
    status: 204,
    headers: {
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET, PUT, DELETE, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type, X-Write-Secret, X-Admin-Secret, X-Pongify-Version',
      'Access-Control-Max-Age': '86400',
    },
  });
}

// ---------------------------------------------------------------------------
// Tournament handlers
// ---------------------------------------------------------------------------

async function handleTournamentPut(request, env, uuid) {
  const writeSecret = request.headers.get('X-Write-Secret');
  if (!writeSecret || writeSecret.length < 32) {
    return textResponse('Missing or invalid secret', 401);
  }

  // Rate limiting (KV-based, eventually-consistent — good enough to prevent abuse)
  const clientIP = request.headers.get('CF-Connecting-IP');
  const rateLimitKey = `ratelimit:${clientIP}:${uuid}`;
  const lastPut = await env.KV.get(rateLimitKey);
  if (lastPut && Date.now() - parseInt(lastPut) < 1000) {
    return textResponse('Rate limited', 429);
  }
  await env.KV.put(rateLimitKey, Date.now().toString(), { expirationTtl: 60 });

  // Body validation
  const body = await request.arrayBuffer();
  if (body.byteLength > 512000) {
    return textResponse('Payload too large', 413);
  }
  if (body.byteLength < 28) {
    // Minimum: 12 bytes IV + 16 bytes GCM tag = 28 bytes (empty plaintext)
    return textResponse('Payload too small', 400);
  }

  const secretHash = await sha256(writeSecret);
  const existing = await env.KV.get(`meta:${uuid}`, 'json');

  if (existing) {
    // UUID exists → verify same owner
    if (secretHash !== existing.secretHash) {
      return textResponse('Forbidden', 403);
    }
  }

  // Store metadata (not encrypted, no personal data)
  const pongifyVersion = request.headers.get('X-Pongify-Version') || 'unknown';
  await env.KV.put(`meta:${uuid}`, JSON.stringify({
    secretHash: secretHash,
    pongifyVersion: pongifyVersion,
    updatedAt: Date.now(),
  }), { expirationTtl: 3 * 86400 });

  // Store encrypted blob
  await env.KV.put(`data:${uuid}`, body, { expirationTtl: 3 * 86400 });

  // Renew webapp TTL for this version (touch index.html)
  const webappIndex = await env.KV.get(`webapp:${pongifyVersion}/index.html`);
  if (webappIndex) {
    await env.KV.put(`webapp:${pongifyVersion}/index.html`, webappIndex, { expirationTtl: 7 * 86400 });
  }

  return textResponse('OK', existing ? 200 : 201);
}

async function handleTournamentGet(env, uuid) {
  const data = await env.KV.get(`data:${uuid}`, 'arrayBuffer');
  if (!data) return textResponse('Not found', 404);

  const meta = await env.KV.get(`meta:${uuid}`, 'json');

  return new Response(data, {
    headers: {
      'Content-Type': 'application/octet-stream',
      ...corsHeaders({
        'X-Updated-At': meta ? meta.updatedAt.toString() : '0',
      }),
    },
  });
}

async function handleVersionGet(env, uuid) {
  const meta = await env.KV.get(`meta:${uuid}`, 'json');
  if (!meta) return textResponse('Not found', 404);

  return new Response(JSON.stringify({ updatedAt: meta.updatedAt }), {
    headers: {
      'Content-Type': 'application/json',
      ...corsHeaders({
        'Cache-Control': 'public, max-age=3', // CDN coalesces polls from all clients
      }),
    },
  });
}

async function handleTournamentDelete(request, env, uuid) {
  const writeSecret = request.headers.get('X-Write-Secret');
  if (!writeSecret) return textResponse('Missing secret', 401);

  const meta = await env.KV.get(`meta:${uuid}`, 'json');
  if (!meta) return textResponse('Not found', 404);

  const secretHash = await sha256(writeSecret);
  if (secretHash !== meta.secretHash) {
    return textResponse('Forbidden', 403);
  }

  await env.KV.delete(`data:${uuid}`);
  await env.KV.delete(`meta:${uuid}`);
  return textResponse('Deleted', 200);
}

// ---------------------------------------------------------------------------
// Webapp handlers
// ---------------------------------------------------------------------------

async function handleWebappPut(request, env, version, path) {
  const adminSecret = request.headers.get('X-Admin-Secret');
  if (!adminSecret || adminSecret !== env.ADMIN_SECRET) {
    return textResponse('Forbidden', 403);
  }

  const body = await request.text();
  if (body.length > 512000) {
    return textResponse('Too large', 413);
  }

  await env.KV.put(`webapp:${version}/${path}`, body, { expirationTtl: 7 * 86400 });
  return textResponse('OK', 200);
}

async function handleWebappCheck(env, version) {
  const exists = await env.KV.get(`webapp:${version}/index.html`);
  return textResponse(exists ? 'OK' : 'Not found', exists ? 200 : 404);
}

async function handleWebappServe(env, uuid, path) {
  const meta = await env.KV.get(`meta:${uuid}`, 'json');
  if (!meta) return textResponse('Tournament not found', 404);

  const version = meta.pongifyVersion;
  const file = path || 'index.html';
  const content = await env.KV.get(`webapp:${version}/${file}`);
  if (!content) return textResponse('File not found', 404);

  const ext = file.split('.').pop();
  const contentTypes = {
    'html': 'text/html; charset=utf-8',
    'js': 'application/javascript',
    'css': 'text/css',
    'ico': 'image/x-icon',
    'png': 'image/png',
    'svg': 'image/svg+xml',
    'json': 'application/json',
    'woff2': 'font/woff2',
    'woff': 'font/woff',
    'ttf': 'font/ttf',
  };

  return new Response(content, {
    headers: {
      'Content-Type': contentTypes[ext] || 'application/octet-stream',
      'Cache-Control': 'public, max-age=86400',
      ...corsHeaders(),
    },
  });
}

// ---------------------------------------------------------------------------
// Router
// ---------------------------------------------------------------------------

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    const method = request.method;
    const path = url.pathname;

    // CORS preflight
    if (method === 'OPTIONS') {
      return handleOptions();
    }

    // Health check
    if (method === 'GET' && path === '/api/health') {
      return jsonResponse({ ok: true });
    }

    // --- Tournament routes: /api/tournament/:uuid[/version] ---
    const tournamentMatch = path.match(/^\/api\/tournament\/([a-f0-9-]{36})(\/version)?$/);
    if (tournamentMatch) {
      const uuid = tournamentMatch[1];
      const isVersion = tournamentMatch[2] === '/version';

      if (method === 'PUT' && !isVersion) {
        return handleTournamentPut(request, env, uuid);
      }
      if (method === 'GET' && isVersion) {
        return handleVersionGet(env, uuid);
      }
      if (method === 'GET' && !isVersion) {
        return handleTournamentGet(env, uuid);
      }
      if (method === 'DELETE' && !isVersion) {
        return handleTournamentDelete(request, env, uuid);
      }
    }

    // --- Webapp upload: PUT /api/webapp/:version/*path ---
    const webappPutMatch = path.match(/^\/api\/webapp\/([^/]+)\/(.+)$/);
    if (method === 'PUT' && webappPutMatch) {
      const version = webappPutMatch[1];
      const filePath = webappPutMatch[2];
      return handleWebappPut(request, env, version, filePath);
    }

    // --- Webapp version check: GET /api/webapp/:version/check ---
    const webappCheckMatch = path.match(/^\/api\/webapp\/([^/]+)\/check$/);
    if (method === 'GET' && webappCheckMatch) {
      return handleWebappCheck(env, webappCheckMatch[1]);
    }

    // --- Webapp serve: GET /t/:uuid[/*path] ---
    const serveMatch = path.match(/^\/t\/([a-f0-9-]{36})(?:\/(.+))?$/);
    if (method === 'GET' && serveMatch) {
      const uuid = serveMatch[1];
      const filePath = serveMatch[2] || null;
      return handleWebappServe(env, uuid, filePath);
    }

    // Catch-all
    return textResponse('Not found', 404);
  },
};
