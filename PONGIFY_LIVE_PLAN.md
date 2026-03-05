# Pongify Live — Plan d'implémentation complet

## Vue d'ensemble

Ajouter un système de publication live du tournoi vers un service cloud (Cloudflare Workers, gratuit, compte propre à l'utilisateur). Chaque vue broadcast (infos tournoi, bracket, round-robin, podium) affiche un **QR code contextuel** qui mène directement à la page correspondante sur la webapp mobile. Les spectateurs/joueurs scannent le QR code affiché sur l'écran de broadcast pour ouvrir une webapp mobile qui affiche les brackets, séries, scores et podiums en temps réel.

Les données sont **chiffrées côté client** (AES-256-GCM) avant envoi au cloud. La clé de déchiffrement est encodée dans le fragment URL (`#K=...`), jamais transmise au serveur (zero-knowledge).

### Architecture

```
┌──────────────────┐   HTTPS PUT (chiffré)   ┌─────────────────────┐   GET/5s      ┌──────────────┐
│ Pongify Desktop  │ ──────────────────────► │ Cloudflare Worker   │ ◄──────────── │  Smartphones │
│                  │  AES blob + Secret      │ (compte de l'orga)  │  blob chiffré │  (navigateur)│
│                  │                         │        ↕            │               │              │
│ pongify-setup.exe│─► crée Worker+KV        │  Cloudflare KV      │               │  Webapp SPA  │
│ QR code ─────────┼─► écran broadcast       │  (TTL auto 3 jours) │               │  (Vue.js)    │
│                  │  URL#K=<clé AES>        │  zero-knowledge     │               │  déchiffre   │
└──────────────────┘                         └─────────────────────┘               └──────────────┘
```

URL QR code : `https://<worker>.workers.dev/t/<uuid>#K=<clé-base64url>/<deep-link>`

Deep links contextuels par vue broadcast :
- **DefaultView** → `#K=<clé>/` → `HomePage.vue` (infos tournoi : nom, date, infoText)
- **SerieBracketView** → `#K=<clé>/serie/<index>?round=<round>` → `SeriePage.vue` (bracket au bon round)
- **RoundRobinView** → `#K=<clé>/serie/<index>` → `SeriePage.vue` (tableau round-robin)
- **HallOfFameView** → `#K=<clé>/podium/<index>` → `PodiumPage.vue` (podium de la série)

Le fragment `#K=...` n'est **jamais envoyé au serveur** (spécification HTTP). Le Worker ne peut pas lire les données même s'il le voulait.

### Stratégie de polling intelligent

Le polling naïf (tous les clients toutes les 5s pendant 8h) dépasserait le free tier. La webapp implémente un **polling adaptatif multi-niveaux** :

| Contexte | Intervalle | Raison |
|----------|-----------|--------|
| Page d'une série "playing" | **5 secondes** | Scores en temps réel |
| Page d'accueil (liste séries) | **15 secondes** | Détection de changement de statut |
| Page d'une série "stopped" | **30 secondes** | Rien ne bouge, vérification légère |
| Page d'une série "finished" / podium | **Polling coupé** | Les données ne changeront plus |
| Téléphone en veille / onglet masqué | **Polling coupé** | Via `document.visibilitychange` API. Reprise immédiate au réveil |

De plus, le Worker ajoute `Cache-Control: public, max-age=3` sur la route `/api/tournament/:uuid/version`. Le CDN Cloudflare sert la réponse cachée aux clients suivants pendant 3 secondes, ce qui **mutualise les requêtes** : 50 clients qui pollent en même temps = 1 seule requête Worker.

### Coûts

**Hypothèses réalistes** : 50 joueurs inscrits, tournoi sur 8h. Les joueurs ne sont pas connectés 8h non-stop — ils jouent leurs matchs (téléphone en veille), mangent, discutent. Estimation : chaque joueur consulte activement ~2h sur les 8h, avec le téléphone en veille le reste du temps.

| Paramètre | Valeur |
|-----------|--------|
| Spectateurs actifs simultanés (moyenne) | ~15 |
| Temps moyen de consultation active par joueur | ~2h |
| Répartition pages : playing 60%, accueil 20%, stopped 10%, finished 10% | — |
| Cache CDN `max-age=3s` sur version check | Mutualisation ~80% des requêtes |

**Calcul détaillé des requêtes Workers (après cache CDN)** :

- Pages "playing" (9 clients effectifs, poll 5s) : 9 × (3600/5) × 2h = **12 960 req** → après cache CDN (~80% servies par cache) = **~2 600 req**
- Pages accueil (3 clients, poll 15s) : 3 × (3600/15) × 2h = **1 440 req** → après cache = **~290 req**
- Pages "stopped" (1.5 clients, poll 30s) : 1.5 × (3600/30) × 2h = **360 req** → après cache = **~72 req**
- Pages "finished"/podium : **0 req** (polling coupé)
- Full data fetch (quand version change, ~200 changements) : 15 × 200 = **3 000 req**

**Total estimé : ~6 000 requêtes Workers/jour**

| Service | Free tier | Usage estimé | Marge | Coût |
|---------|-----------|-------------|-------|------|
| Workers requests | 100 000 /jour | ~6 000 | ×16 | **0€** |
| KV lectures | 100 000 /jour | ~6 000 | ×16 | **0€** |
| KV écritures | 1 000 /jour | ~600 | ×1.7 | **0€** |
| KV stockage | 1 GB | ~500 KB | ×2000 | **0€** |
| **Total** | | | | **0€** |

Même avec 200 spectateurs simultanés actifs, le cache CDN + polling adaptatif maintiennent l'usage sous les 100K requêtes.

### Sécurité

| Mécanisme | Description |
|-----------|-------------|
| **Chiffrement E2E** | AES-256-GCM côté client. Le Worker stocke un blob opaque. Clé dans le fragment URL. |
| **write_secret** | Secret 256-bit par tournoi, généré par Pongify, hash SHA-256 stocké dans KV. Empêche un tiers d'écraser les données. |
| **admin_secret** | Secret du compte Cloudflare (pour uploader la webapp). Stocké dans les variables d'env du Worker. |
| **UUID v4** | 122 bits d'entropie. Deviner un UUID actif est statistiquement impossible. |
| **HTTPS** | Obligatoire (Cloudflare Workers). Header X-Write-Secret chiffré en transit. |
| **Rate limiting** | 1 PUT/sec par IP par UUID dans le Worker. |
| **TTL 3 jours** | Données auto-supprimées du KV 3 jours après le dernier PUT. |

---

## Prérequis et conventions existantes

### Stack technique actuelle
- **Qt 6** : modules `core gui widgets quick quickcontrols2` (fichier `pongify.pro`)
- **C++17** (`CONFIG += c++17`)
- **QML** + **QuickFlux** pour le state management côté UI
- **Go** utilisé pour `cdsls_scrapper/` (outil externe lancé via `QProcess`)
- **QMake** comme build system (pas CMake)

### Patterns de code à respecter

**Singleton** (cf. `TStorage`, `PlayerModel`) :
```cpp
class MyClass {
public:
    static MyClass *Instance() {
        static MyClass m;
        return &m;
    }
private:
    MyClass();
};
```

**Propriétés QML** (cf. `src/qqmlhelpers.h`) :
```cpp
// Génère: get_name(), update_name(), nameChanged() signal, m_name backing field
QML_READONLY_PROPERTY(QString, name)
QML_WRITABLE_PROPERTY(bool, enabled)
```

**Signaux de sauvegarde** (cf. `src/Tournament.cpp` `addSerie()`) :
```cpp
connect(s, &TSerie::matchesUpdated, this, [this]() {
    TStorage::Instance()->saveToDisk(this);
});
```

**Image Provider QML** (cf. `src/BroadcastPreviewProvider.h`) :
```cpp
class MyProvider : public QQuickImageProvider {
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    // mutex-protected, thread-safe
};
// Enregistrement dans MainWindow::loadQmlApp():
view->engine()->addImageProvider("providername", provider);
// Accès QML: image://providername/id
```

**BroadcastView** (cf. `src/BroadcastModel.h`) :
```cpp
class BroadcastView: public QObject {
    QML_READONLY_PROPERTY(QObject *, viewSerie)
    QML_READONLY_PROPERTY(QString, viewUrl)
    QML_READONLY_PROPERTY(QString, name)
    QML_WRITABLE_PROPERTY(bool, viewVisible)
    QML_WRITABLE_PROPERTY(int, viewTime)
};
```

**Ajout de vue broadcast** (cf. `src/BroadcastWindow.cpp` `reloadViews()`) :
```cpp
auto v = new BroadcastView(this);
v->update_viewUrl("qrc:/qml/broadcast/MyView.qml");
v->update_name("Ma vue");
v->set_viewVisible(true);
v->set_viewTime(10000);
views->append(v);
```

**Outil Go externe** (cf. `cdsls_scrapper/`) :
- Binaire `.exe` dans le même répertoire que Pongify
- Lancé via `QProcess`, JSON sur stdout
- `Utils::getBinPath()` retourne le répertoire des binaires

---

## PHASE A — Cloudflare Worker + Outil de setup Go

### TODO A1 : Créer le code du Cloudflare Worker

**Fichier à créer : `pongify_setup/worker.js`** (~150 lignes)

Ce fichier JavaScript sera embarqué dans le binaire Go via `//go:embed` et déployé automatiquement par `pongify-setup.exe`.

**Routes à implémenter :**

| Méthode | Route | Auth | Description |
|---------|-------|------|-------------|
| `PUT` | `/api/tournament/:uuid` | `X-Write-Secret` | Stocker blob chiffré dans KV |
| `GET` | `/api/tournament/:uuid` | Aucune | Retourner le blob chiffré |
| `GET` | `/api/tournament/:uuid/version` | Aucune | Retourner juste `{ "updatedAt": timestamp }` |
| `DELETE` | `/api/tournament/:uuid` | `X-Write-Secret` | Supprimer du KV |
| `PUT` | `/api/webapp/:version/*path` | `X-Admin-Secret` | Uploader fichier webapp |
| `GET` | `/api/webapp/:version/check` | Aucune | Vérifier si version existe (200 ou 404) |
| `GET` | `/t/:uuid` | Aucune | Servir `index.html` de la webapp (lookup version via meta) |
| `GET` | `/t/:uuid/*path` | Aucune | Servir fichiers statiques webapp (JS, CSS) |
| `GET` | `/api/health` | Aucune | Health check simple, retourne `{ "ok": true }` |

**Structure KV :**

| Clé | Contenu | TTL |
|-----|---------|-----|
| `data:<uuid>` | Blob chiffré (ArrayBuffer) | 3 jours (renouvelé à chaque PUT) |
| `meta:<uuid>` | JSON: `{ "secretHash": "sha256...", "pongifyVersion": "2.1", "updatedAt": 1709640000 }` | 3 jours |
| `webapp:<version>/<file>` | Contenu du fichier (HTML/JS/CSS) | 7 jours |
| `ratelimit:<ip>:<uuid>` | Timestamp du dernier PUT | 60 secondes |

**Logique de sécurité du PUT `/api/tournament/:uuid` :**

```javascript
export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    // Router vers la bonne handler...
  }
};

async function handleTournamentPut(request, env, uuid) {
  const writeSecret = request.headers.get('X-Write-Secret');
  if (!writeSecret || writeSecret.length < 32) {
    return new Response('Missing or invalid secret', { status: 401 });
  }

  // Rate limiting
  const clientIP = request.headers.get('CF-Connecting-IP');
  const rateLimitKey = `ratelimit:${clientIP}:${uuid}`;
  const lastPut = await env.KV.get(rateLimitKey);
  if (lastPut && Date.now() - parseInt(lastPut) < 1000) {
    return new Response('Rate limited', { status: 429 });
  }
  await env.KV.put(rateLimitKey, Date.now().toString(), { expirationTtl: 60 });

  // Body validation
  const body = await request.arrayBuffer();
  if (body.byteLength > 512000) {
    return new Response('Payload too large', { status: 413 });
  }
  if (body.byteLength < 28) {
    // Minimum: 12 bytes IV + 16 bytes GCM tag = 28 bytes (empty plaintext)
    return new Response('Payload too small', { status: 400 });
  }

  const secretHash = await sha256(writeSecret);
  const existing = await env.KV.get(`meta:${uuid}`, 'json');

  if (existing) {
    // UUID existe → vérifier que c'est le même propriétaire
    if (secretHash !== existing.secretHash) {
      return new Response('Forbidden', { status: 403 });
    }
  }

  // Stocker les métadonnées (non chiffrées, pas de données perso)
  const pongifyVersion = request.headers.get('X-Pongify-Version') || 'unknown';
  await env.KV.put(`meta:${uuid}`, JSON.stringify({
    secretHash: secretHash,
    pongifyVersion: pongifyVersion,
    updatedAt: Date.now()
  }), { expirationTtl: 3 * 86400 });

  // Stocker le blob chiffré
  await env.KV.put(`data:${uuid}`, body, { expirationTtl: 3 * 86400 });

  // Renouveler le TTL des fichiers webapp de cette version
  const webappCheck = await env.KV.get(`webapp:${pongifyVersion}/index.html`);
  if (webappCheck) {
    // Touch pour renouveler le TTL (re-stocker avec nouveau TTL)
    await env.KV.put(`webapp:${pongifyVersion}/index.html`, webappCheck, { expirationTtl: 7 * 86400 });
  }

  return new Response('OK', { status: existing ? 200 : 201 });
}

async function handleTournamentGet(env, uuid) {
  const data = await env.KV.get(`data:${uuid}`, 'arrayBuffer');
  if (!data) return new Response('Not found', { status: 404 });

  const meta = await env.KV.get(`meta:${uuid}`, 'json');

  return new Response(data, {
    headers: {
      'Content-Type': 'application/octet-stream',
      'X-Updated-At': meta ? meta.updatedAt.toString() : '0',
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Expose-Headers': 'X-Updated-At'
    }
  });
}

async function handleVersionGet(env, uuid) {
  const meta = await env.KV.get(`meta:${uuid}`, 'json');
  if (!meta) return new Response('Not found', { status: 404 });

  return new Response(JSON.stringify({ updatedAt: meta.updatedAt }), {
    headers: {
      'Content-Type': 'application/json',
      'Access-Control-Allow-Origin': '*',
      'Cache-Control': 'public, max-age=3' // CDN mutualise les polls de tous les clients
    }
  });
}

async function handleTournamentDelete(request, env, uuid) {
  const writeSecret = request.headers.get('X-Write-Secret');
  if (!writeSecret) return new Response('Missing secret', { status: 401 });

  const meta = await env.KV.get(`meta:${uuid}`, 'json');
  if (!meta) return new Response('Not found', { status: 404 });

  const secretHash = await sha256(writeSecret);
  if (secretHash !== meta.secretHash) {
    return new Response('Forbidden', { status: 403 });
  }

  await env.KV.delete(`data:${uuid}`);
  await env.KV.delete(`meta:${uuid}`);
  return new Response('Deleted', { status: 200 });
}

async function handleWebappPut(request, env, version, path) {
  const adminSecret = request.headers.get('X-Admin-Secret');
  if (!adminSecret || adminSecret !== env.ADMIN_SECRET) {
    return new Response('Forbidden', { status: 403 });
  }

  const body = await request.text();
  if (body.length > 512000) {
    return new Response('Too large', { status: 413 });
  }

  await env.KV.put(`webapp:${version}/${path}`, body, { expirationTtl: 7 * 86400 });
  return new Response('OK', { status: 200 });
}

async function handleWebappCheck(env, version) {
  const exists = await env.KV.get(`webapp:${version}/index.html`);
  return new Response(exists ? 'OK' : 'Not found', { status: exists ? 200 : 404 });
}

async function handleWebappServe(env, uuid, path) {
  const meta = await env.KV.get(`meta:${uuid}`, 'json');
  if (!meta) return new Response('Tournament not found', { status: 404 });

  const version = meta.pongifyVersion;
  const file = path || 'index.html';
  const content = await env.KV.get(`webapp:${version}/${file}`);
  if (!content) return new Response('File not found', { status: 404 });

  const ext = file.split('.').pop();
  const contentTypes = {
    'html': 'text/html; charset=utf-8',
    'js': 'application/javascript',
    'css': 'text/css',
    'ico': 'image/x-icon',
    'png': 'image/png',
    'svg': 'image/svg+xml'
  };

  return new Response(content, {
    headers: {
      'Content-Type': contentTypes[ext] || 'application/octet-stream',
      'Cache-Control': 'public, max-age=86400'
    }
  });
}

// CORS preflight handler
function handleOptions() {
  return new Response(null, {
    headers: {
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET, PUT, DELETE, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type, X-Write-Secret, X-Admin-Secret, X-Pongify-Version',
      'Access-Control-Max-Age': '86400'
    }
  });
}

async function sha256(message) {
  const data = new TextEncoder().encode(message);
  const hash = await crypto.subtle.digest('SHA-256', data);
  return [...new Uint8Array(hash)].map(b => b.toString(16).padStart(2, '0')).join('');
}
```

**Router principal** (à ajouter dans le `fetch` handler) :
```javascript
// Routing logic:
// OPTIONS * → handleOptions()
// PUT    /api/tournament/:uuid → handleTournamentPut(request, env, uuid)
// GET    /api/tournament/:uuid → handleTournamentGet(env, uuid)
// GET    /api/tournament/:uuid/version → handleVersionGet(env, uuid)
// DELETE /api/tournament/:uuid → handleTournamentDelete(request, env, uuid)
// PUT    /api/webapp/:version/*path → handleWebappPut(request, env, version, path)
// GET    /api/webapp/:version/check → handleWebappCheck(env, version)
// GET    /t/:uuid → handleWebappServe(env, uuid, null)
// GET    /t/:uuid/*path → handleWebappServe(env, uuid, path)
// GET    /api/health → Response('{"ok":true}', 200)
```

---

### TODO A2 : Créer l'outil de setup Go

**Fichiers à créer :**
- `pongify_setup/go.mod`
- `pongify_setup/main.go`

**`pongify_setup/go.mod` :**
```go
module pongify_setup
go 1.21
```

Aucune dépendance externe — uniquement `net/http`, `encoding/json`, `crypto/rand`, `embed`, `fmt`, `os`, `bufio`.

**`pongify_setup/main.go`** (~200 lignes) :

```go
package main

import (
    "bufio"
    "crypto/rand"
    "embed"
    "encoding/hex"
    "encoding/json"
    "fmt"
    "io"
    "net/http"
    "os"
    "strings"
)

//go:embed worker.js
var workerJS string
```

**Flux d'exécution de `main()` :**

1. **Demander le token API** Cloudflare (input interactif via stdin, ou argument `--token`)
   ```
   === Pongify Live - Setup ===

   Ce programme configure votre compte Cloudflare pour Pongify Live.

   1. Allez sur https://dash.cloudflare.com/profile/api-tokens
   2. Créez un token avec les permissions:
      - Account > Workers Scripts > Edit
      - Account > Workers KV Storage > Edit
   3. Collez le token ci-dessous:

   Token API: ▮
   ```

2. **Vérifier le token** : `GET https://api.cloudflare.com/client/v4/user/tokens/verify`
   - Header: `Authorization: Bearer <token>`
   - Vérifier `result.status == "active"`
   - En cas d'erreur : afficher le message et quitter

3. **Récupérer l'Account ID** : `GET https://api.cloudflare.com/client/v4/accounts?page=1&per_page=5`
   - Prendre `result[0].id`
   - Si plusieurs comptes : prendre le premier (suffisant pour le use case)

4. **Créer le namespace KV** : `POST https://api.cloudflare.com/client/v4/accounts/{account_id}/storage/kv/namespaces`
   - Body: `{ "title": "PONGIFY_TOURNAMENTS" }`
   - Si erreur 409 (déjà existe) : récupérer l'ID existant via `GET .../namespaces?per_page=100` et filtrer par title
   - Retenir le `namespace_id`

5. **Générer le admin_secret** :
   ```go
   secretBytes := make([]byte, 32)
   _, err := rand.Read(secretBytes)
   adminSecret := hex.EncodeToString(secretBytes) // 64 chars hex
   ```

6. **Déployer le Worker** : `PUT https://api.cloudflare.com/client/v4/accounts/{account_id}/workers/scripts/pongify-live`
   - Content-Type: `multipart/form-data`
   - Part 1 : `name="worker.js"`, `type="application/javascript+module"`, contenu = `workerJS` (embarqué)
   - Part 2 : `name="metadata"`, `type="application/json"`, contenu :
     ```json
     {
       "main_module": "worker.js",
       "bindings": [
         {
           "type": "kv_namespace",
           "name": "KV",
           "namespace_id": "<namespace_id>"
         }
       ],
       "compatibility_date": "2024-01-01"
     }
     ```

7. **Configurer le secret ADMIN_SECRET** : `PUT https://api.cloudflare.com/client/v4/accounts/{account_id}/workers/scripts/pongify-live/secrets`
   - Body: `{ "name": "ADMIN_SECRET", "text": "<adminSecret>", "type": "secret_text" }`

8. **Activer la route** (subdomain workers.dev) : Le Worker est automatiquement accessible à `https://pongify-live.<subdomain>.workers.dev` après déploiement.

9. **Récupérer l'URL du Worker** : `GET https://api.cloudflare.com/client/v4/accounts/{account_id}/workers/subdomain`
   - L'URL sera : `https://pongify-live.<subdomain>.workers.dev`

10. **Afficher le résultat** :
    ```
    ✓ Worker déployé avec succès !

    URL du Worker:  https://pongify-live.xxx.workers.dev
    Admin Secret:   abcdef0123456789...

    Collez ces informations dans Pongify:
    Menu → Outils → Configuration publication web

    {"worker_url": "https://pongify-live.xxx.workers.dev", "admin_secret": "abcdef..."}
    ```
    La dernière ligne est du JSON parseable pour faciliter le copier-coller.

**Gestion d'erreurs** : chaque appel API doit vérifier le `success` field de la réponse Cloudflare et afficher un message clair en cas d'échec.

**Compilation** :
```bash
cd pongify_setup
# Windows
GOOS=windows GOARCH=amd64 go build -o pongify-setup.exe .
# Linux
GOOS=linux GOARCH=amd64 go build -o pongify-setup .
# macOS
GOOS=darwin GOARCH=amd64 go build -o pongify-setup-macos .
```

---

## PHASE B — Intégration Pongify (C++)

### TODO B1 : Ajouter la librairie QR Code (3rd party)

**Fichiers à créer :**
- `3rd_party/qrcodegen/qrcodegen.hpp` — copier depuis https://github.com/nayuki/QR-Code-generator (fichier `cpp/qrcodegen.hpp`)
- `3rd_party/qrcodegen/qrcodegen.cpp` — copier depuis https://github.com/nayuki/QR-Code-generator (fichier `cpp/qrcodegen.cpp`)

Licence MIT. Deux fichiers, zéro dépendance, header + source.

---

### TODO B2 : Modifier `pongify.pro`

**Fichier à modifier : `pongify.pro`**

Changements à effectuer :

1. **Ajouter le module `network`** à la ligne QT :
   ```
   QT += core gui widgets quick quickcontrols2 network
   ```

2. **Ajouter `INCLUDEPATH`** pour qrcodegen :
   ```
   INCLUDEPATH += src 3rd_party/qrcodegen
   ```

3. **Ajouter aux SOURCES** (à la fin de la liste SOURCES) :
   ```
   src/WebPublisher.cpp \
   src/DialogWebPublish.cpp \
   3rd_party/qrcodegen/qrcodegen.cpp
   ```

4. **Ajouter aux HEADERS** (à la fin de la liste HEADERS) :
   ```
   src/WebPublisher.h \
   src/DialogWebPublish.h \
   src/QrCodeProvider.h \
   3rd_party/qrcodegen/qrcodegen.hpp
   ```

5. **Ajouter aux FORMS** (à la fin de la liste FORMS) :
   ```
   src/DialogWebPublish.ui
   ```

---

### TODO B3 : Ajouter `writeSecret` et `encryptionKey` au modèle Tournament

**Fichier à modifier : `src/Tournament.h`**

Ajouter deux nouvelles propriétés après les propriétés existantes :
```cpp
QML_READONLY_PROPERTY(QString, writeSecret)
QML_READONLY_PROPERTY(QString, encryptionKey)
```

**Fichier à modifier : `src/Tournament.cpp`**

1. **Dans `toJson()`** — ajouter `write_secret` et `encryption_key` aux champs sérialisés (pour sauvegarde locale .tnm) :
   ```cpp
   { "write_secret", get_writeSecret() },
   { "encryption_key", get_encryptionKey() },
   ```
   Ajouter ces deux lignes dans le `return { ... }` de `toJson()`, après `default_view_visible`.

2. **Créer une nouvelle méthode `toJsonForPublish()`** — version filtrée du JSON pour la publication web.

   **Champs exclus** (internes à Pongify) : `write_secret`, `encryption_key`, `info_text`, `time_broadcast_change`, `broadcast_scroll_speed`, `default_view_visible`.

   **Filtrage des séries** : seules les séries avec `status == "playing"` ou `status == "finished"` sont incluses. Les séries en `"stopped"` (en cours de création, affectation de joueurs, configuration) sont des données d'organisation qui n'ont pas à être publiées. L'organisateur travaille dessus avant de les lancer.

   ```cpp
   QJsonObject Tournament::toJsonForPublish()
   {
       QJsonArray seriesArray;
       for (int i = 0; i < serieCount(); i++) {
           auto s = getSerie(i);
           // Exclure les séries en préparation (stopped)
           if (s->get_status() == "playing" || s->get_status() == "finished") {
               seriesArray.append(s->toJson());
           }
       }

       // Tables : inclure seulement celles assignées à une série visible
       QJsonArray tablesArray;
       for (int i = 0; i < tableCount(); i++) {
           auto t = getTable(i);
           // Vérifier si la table est liée à une série playing/finished
           // (optionnel : inclure toutes les tables ou filtrer)
           tablesArray.append(t->toJson());
       }

       return {
           { "uuid", get_uuid() },
           { "name", get_name() },
           { "date", get_date() },
           { "status", get_status() },
           { "series", seriesArray },
           { "tables", tablesArray }
       };
   }
   ```

   Le JSON publié contient : `uuid`, `name`, `date`, `status`, `series` (filtrées), `tables`.

   Ajouter la déclaration dans `Tournament.h` :
   ```cpp
   QJsonObject toJsonForPublish();
   ```

3. **Dans `fromJson()`** — lire les deux champs (avec valeur par défaut vide si absents, pour la migration des anciens .tnm) :
   ```cpp
   t->update_writeSecret(obj["write_secret"].toString());
   t->update_encryptionKey(obj["encryption_key"].toString());
   ```
   Ajouter après la ligne `t->set_defaultViewVisible(...)`.

---

### TODO B4 : Créer la classe `WebPublisher`

**Fichier à créer : `src/WebPublisher.h`**

```cpp
#ifndef WEBPUBLISHER_H
#define WEBPUBLISHER_H

#include <QObject>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;
class Tournament;

class WebPublisher : public QObject
{
    Q_OBJECT
public:
    static WebPublisher *Instance()
    {
        static WebPublisher m;
        return &m;
    }

    void setTournament(Tournament *t);
    void clearTournament();
    void publish();
    void unpublish();
    void uploadWebappIfNeeded(const QString &version);

    QString workerUrl() const;
    QString adminSecret() const;
    bool enabled() const;

    void setWorkerUrl(const QString &url);
    void setAdminSecret(const QString &secret);
    void setEnabled(bool enabled);

    void loadSettings();
    void saveSettings();

    static QString generateSecret();   // 64 chars hex, 256 bits
    static QByteArray generateKey();    // 32 bytes raw, for AES-256

    bool testConnection();

signals:
    void publishError(const QString &message);
    void publishSuccess();

private:
    WebPublisher();

    void doPublish();
    void doEncryptAndSend(const QByteArray &plaintext);
    void handleReply(QNetworkReply *reply);

    QNetworkAccessManager *m_nam = nullptr;
    QTimer *m_debounceTimer = nullptr;
    Tournament *m_tournament = nullptr;

    QString m_workerUrl;
    QString m_adminSecret;
    bool m_enabled = false;

    QList<QMetaObject::Connection> m_connections;
};

#endif // WEBPUBLISHER_H
```

**Fichier à créer : `src/WebPublisher.cpp`**

Implémentation détaillée :

**Constructeur** :
```cpp
WebPublisher::WebPublisher() : QObject(nullptr)
{
    m_nam = new QNetworkAccessManager(this);

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(2000); // 2 secondes de coalescence
    connect(m_debounceTimer, &QTimer::timeout, this, &WebPublisher::doPublish);

    loadSettings();
}
```

**`loadSettings()` / `saveSettings()`** — utiliser `QSettings` :
```cpp
void WebPublisher::loadSettings() {
    QSettings settings;
    m_workerUrl = settings.value("WebPublish/workerUrl").toString();
    m_adminSecret = settings.value("WebPublish/adminSecret").toString();
    m_enabled = settings.value("WebPublish/enabled", false).toBool();
}

void WebPublisher::saveSettings() {
    QSettings settings;
    settings.setValue("WebPublish/workerUrl", m_workerUrl);
    settings.setValue("WebPublish/adminSecret", m_adminSecret);
    settings.setValue("WebPublish/enabled", m_enabled);
}
```

**`setTournament(Tournament *t)`** :
```cpp
void WebPublisher::setTournament(Tournament *t) {
    clearTournament();
    m_tournament = t;
    if (!t || !m_enabled) return;

    // Connexion aux signaux — MÊME signaux que ceux qui déclenchent saveToDisk dans Tournament.cpp
    // On se connecte à seriesStatusChanged car il agrège tous les statusChanged des séries
    m_connections << connect(t, &Tournament::seriesStatusChanged, this, &WebPublisher::publish);
    m_connections << connect(t, &Tournament::defaultViewVisibleChanged, this, &WebPublisher::publish);

    // Pour matchesUpdated et viewVisibleChanged des séries existantes
    for (int i = 0; i < t->serieCount(); i++) {
        auto s = t->getSerie(i);
        m_connections << connect(s, &TSerie::matchesUpdated, this, &WebPublisher::publish);
        m_connections << connect(s, &TSerie::viewVisibleChanged, this, &WebPublisher::publish);
    }

    // Pour les tables existantes
    for (int i = 0; i < t->tableCount(); i++) {
        auto table = t->getTable(i);
        m_connections << connect(table, &TTable::matchChanged, this, &WebPublisher::publish);
    }

    // Publication initiale
    publish();
}
```

**`clearTournament()`** :
```cpp
void WebPublisher::clearTournament() {
    for (auto &conn : m_connections)
        disconnect(conn);
    m_connections.clear();
    m_tournament = nullptr;
    m_debounceTimer->stop();
}
```

**`publish()`** — juste démarre le debounce timer :
```cpp
void WebPublisher::publish() {
    if (!m_enabled || !m_tournament || m_workerUrl.isEmpty()) return;
    m_debounceTimer->start(); // (re)start — coalescence 2s
}
```

**`doPublish()`** — sérialise, chiffre, envoie :
```cpp
void WebPublisher::doPublish() {
    if (!m_tournament) return;

    QJsonObject json = m_tournament->toJsonForPublish();
    QByteArray plaintext = QJsonDocument(json).toJson(QJsonDocument::Compact);

    doEncryptAndSend(plaintext);
}
```

**`doEncryptAndSend()`** — chiffrement AES-256-GCM via OpenSSL (lié par Qt) :
```cpp
#include <openssl/evp.h>
#include <openssl/rand.h>

void WebPublisher::doEncryptAndSend(const QByteArray &plaintext) {
    // Récupérer la clé de chiffrement (stockée en base64url dans le tournament)
    QByteArray key = QByteArray::fromBase64(
        m_tournament->get_encryptionKey().toLatin1(),
        QByteArray::Base64UrlEncoding
    );
    if (key.size() != 32) {
        qWarning() << "WebPublisher: invalid encryption key size";
        return;
    }

    // Générer IV aléatoire (12 bytes pour AES-GCM)
    unsigned char iv[12];
    RAND_bytes(iv, 12);

    // Chiffrer
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr);
    EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                       reinterpret_cast<const unsigned char*>(key.constData()), iv);

    QByteArray ciphertext(plaintext.size() + 16, 0); // +16 for potential padding
    int outLen = 0;
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()), &outLen,
                      reinterpret_cast<const unsigned char*>(plaintext.constData()), plaintext.size());
    int totalLen = outLen;

    int finalLen = 0;
    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()) + totalLen, &finalLen);
    totalLen += finalLen;

    // Récupérer le tag GCM (16 bytes)
    unsigned char tag[16];
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
    EVP_CIPHER_CTX_free(ctx);

    // Assembler: IV (12) + ciphertext + tag (16)
    QByteArray payload;
    payload.append(reinterpret_cast<const char*>(iv), 12);
    payload.append(ciphertext.constData(), totalLen);
    payload.append(reinterpret_cast<const char*>(tag), 16);

    // Envoyer via HTTP PUT
    QString url = m_workerUrl + "/api/tournament/" + m_tournament->get_uuid();
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("X-Write-Secret", m_tournament->get_writeSecret().toUtf8());
    request.setRawHeader("X-Pongify-Version", pongify_version);

    QNetworkReply *reply = m_nam->put(request, payload);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
        reply->deleteLater();
    });
}
```

**`handleReply()`** :
```cpp
void WebPublisher::handleReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "WebPublisher: HTTP error:" << reply->errorString();
        emit publishError(reply->errorString());
    } else {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 200 && status < 300) {
            emit publishSuccess();
        } else {
            qWarning() << "WebPublisher: HTTP" << status << reply->readAll();
            emit publishError(QString("HTTP %1").arg(status));
        }
    }
}
```

**`unpublish()`** :
```cpp
void WebPublisher::unpublish() {
    if (!m_tournament || m_workerUrl.isEmpty()) return;

    QString url = m_workerUrl + "/api/tournament/" + m_tournament->get_uuid();
    QNetworkRequest request(QUrl(url));
    request.setRawHeader("X-Write-Secret", m_tournament->get_writeSecret().toUtf8());

    m_nam->deleteResource(request);
    clearTournament();
}
```

**`uploadWebappIfNeeded()`** :
```cpp
void WebPublisher::uploadWebappIfNeeded(const QString &version) {
    if (m_workerUrl.isEmpty() || m_adminSecret.isEmpty()) return;

    // Vérifier si cette version existe déjà
    QString checkUrl = m_workerUrl + "/api/webapp/" + version + "/check";
    QNetworkReply *reply = m_nam->get(QNetworkRequest(QUrl(checkUrl)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, version]() {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        reply->deleteLater();

        if (status == 200) return; // Déjà uploadée

        // Uploader les fichiers webapp depuis les ressources Qt
        QStringList files = {"index.html", "app.js", "app.css"};
        for (const auto &filename : files) {
            QFile res(":/webapp/" + filename);
            if (!res.open(QIODevice::ReadOnly)) {
                qWarning() << "WebPublisher: cannot open resource" << filename;
                continue;
            }
            QByteArray content = res.readAll();
            res.close();

            QString putUrl = m_workerUrl + "/api/webapp/" + version + "/" + filename;
            QNetworkRequest req(QUrl(putUrl));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
            req.setRawHeader("X-Admin-Secret", m_adminSecret.toUtf8());
            m_nam->put(req, content);
        }
    });
}
```

**`generateSecret()`** et **`generateKey()`** (méthodes statiques) :
```cpp
QString WebPublisher::generateSecret() {
    // 256 bits = 32 bytes → 64 chars hex
    return QUuid::createUuid().toString(QUuid::Id128)
         + QUuid::createUuid().toString(QUuid::Id128);
}

QByteArray WebPublisher::generateKey() {
    // 256 bits = 32 bytes raw
    QByteArray key(32, 0);
    RAND_bytes(reinterpret_cast<unsigned char*>(key.data()), 32);
    return key;
}
```

**`testConnection()`** :
```cpp
bool WebPublisher::testConnection() {
    if (m_workerUrl.isEmpty()) return false;

    QNetworkRequest req(QUrl(m_workerUrl + "/api/health"));
    QNetworkReply *reply = m_nam->get(req);

    // Bloquer brièvement (acceptable dans un dialog)
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5000, &loop, &QEventLoop::quit); // timeout 5s
    loop.exec();

    bool ok = reply->error() == QNetworkReply::NoError;
    reply->deleteLater();
    return ok;
}
```

**Includes nécessaires pour `WebPublisher.cpp`** :
```cpp
#include "WebPublisher.h"
#include "Tournament.h"
#include "TSerie.h"
#include "TTable.h"
#include "version.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUuid>
#include <QFile>
#include <QEventLoop>
#include <openssl/evp.h>
#include <openssl/rand.h>
```

---

### TODO B5 : Créer le QR Code Provider

**Fichier à créer : `src/QrCodeProvider.h`**

Header-only. Génère les QR codes **à la demande** à partir du paramètre `id` dans la source d'image QML. Chaque vue broadcast passe un deep link différent (ex: `image://qrcode/serie/0/round/2`), et le provider construit l'URL complète avec la clé de chiffrement.

```cpp
#ifndef QRCODEPROVIDER_H
#define QRCODEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include "qrcodegen.hpp"

class QrCodeProvider : public QQuickImageProvider
{
public:
    QrCodeProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {}

    /// Appelé une fois quand la publication démarre. Vide le cache.
    void setBaseInfo(const QString &baseUrl, const QString &key)
    {
        QMutexLocker locker(&mutex);
        m_baseUrl = baseUrl;   // ex: "https://worker.workers.dev/t/<uuid>"
        m_key = key;           // ex: base64url AES key
        m_cache.clear();
    }

    void clear()
    {
        QMutexLocker locker(&mutex);
        m_baseUrl.clear();
        m_key.clear();
        m_cache.clear();
    }

    /// id format (venant du QML Image.source) :
    ///   "home"                    → deep link "/"
    ///   "serie/INDEX"             → deep link "/serie/INDEX"
    ///   "serie/INDEX/round/ROUND" → deep link "/serie/INDEX?round=ROUND"
    ///   "podium/INDEX"            → deep link "/podium/INDEX"
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        Q_UNUSED(requestedSize)
        QMutexLocker locker(&mutex);

        if (m_cache.contains(id)) {
            if (size) *size = m_cache[id].size();
            return m_cache[id];
        }

        // Construire le deep link depuis l'id
        QString deepLink;
        if (id == "home")
            deepLink = "/";
        else if (id.startsWith("serie/") && id.contains("/round/")) {
            auto parts = id.split("/round/");
            deepLink = "/" + parts[0] + "?round=" + parts[1];
        } else
            deepLink = "/" + id;   // "serie/0" → "/serie/0", "podium/1" → "/podium/1"

        // URL complète : baseUrl + #K=key + deepLink
        QString fullUrl = m_baseUrl + "#K=" + m_key + deepLink;

        QImage img = generateQr(fullUrl);
        m_cache[id] = img;

        if (size) *size = img.size();
        return img;
    }

private:
    QImage generateQr(const QString &url) const
    {
        using namespace qrcodegen;
        QrCode qr = QrCode::encodeText(url.toUtf8().constData(), QrCode::Ecc::MEDIUM);

        int sz = qr.getSize();
        int border = 4;
        int scale = 8; // pixels par module
        int imgSize = (sz + border * 2) * scale;

        QImage image(imgSize, imgSize, QImage::Format_RGB32);
        image.fill(Qt::white);

        for (int y = 0; y < sz; y++) {
            for (int x = 0; x < sz; x++) {
                if (qr.getModule(x, y)) {
                    for (int dy = 0; dy < scale; dy++) {
                        for (int dx = 0; dx < scale; dx++) {
                            image.setPixel(
                                (x + border) * scale + dx,
                                (y + border) * scale + dy,
                                qRgb(0, 0, 0)
                            );
                        }
                    }
                }
            }
        }
        return image;
    }

    QString m_baseUrl;
    QString m_key;
    QMap<QString, QImage> m_cache;
    QMutex mutex;
};

#endif // QRCODEPROVIDER_H
```

---

### TODO B6 : Créer le dialog de configuration web publish

**Fichier à créer : `src/DialogWebPublish.h`**

```cpp
#ifndef DIALOGWEBPUBLISH_H
#define DIALOGWEBPUBLISH_H

#include <QDialog>

namespace Ui { class DialogWebPublish; }

class DialogWebPublish : public QDialog
{
    Q_OBJECT
public:
    explicit DialogWebPublish(QWidget *parent = nullptr);
    ~DialogWebPublish();

    QString getWorkerUrl() const;
    QString getAdminSecret() const;
    bool getEnabled() const;

private slots:
    void testConnection();

private:
    Ui::DialogWebPublish *ui;
};

#endif // DIALOGWEBPUBLISH_H
```

**Fichier à créer : `src/DialogWebPublish.cpp`**

```cpp
#include "DialogWebPublish.h"
#include "ui_DialogWebPublish.h"
#include "WebPublisher.h"

DialogWebPublish::DialogWebPublish(QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogWebPublish)
{
    ui->setupUi(this);
    setWindowTitle(tr("Configuration publication web"));

    // Charger les valeurs actuelles
    auto wp = WebPublisher::Instance();
    ui->editWorkerUrl->setText(wp->workerUrl());
    ui->editAdminSecret->setText(wp->adminSecret());
    ui->editAdminSecret->setEchoMode(QLineEdit::Password);
    ui->checkEnabled->setChecked(wp->enabled());
    ui->labelStatus->clear();

    connect(ui->btnTest, &QPushButton::clicked, this, &DialogWebPublish::testConnection);

    connect(this, &QDialog::accepted, this, [this]() {
        auto wp = WebPublisher::Instance();
        wp->setWorkerUrl(ui->editWorkerUrl->text().trimmed());
        wp->setAdminSecret(ui->editAdminSecret->text().trimmed());
        wp->setEnabled(ui->checkEnabled->isChecked());
        wp->saveSettings();
    });
}

DialogWebPublish::~DialogWebPublish()
{
    delete ui;
}

void DialogWebPublish::testConnection()
{
    ui->labelStatus->setText(tr("Test en cours..."));

    auto wp = WebPublisher::Instance();
    // Mettre à jour temporairement l'URL pour le test
    QString savedUrl = wp->workerUrl();
    wp->setWorkerUrl(ui->editWorkerUrl->text().trimmed());

    bool ok = wp->testConnection();

    wp->setWorkerUrl(savedUrl); // Restaurer

    if (ok)
        ui->labelStatus->setText(tr("✓ Connexion réussie"));
    else
        ui->labelStatus->setText(tr("✗ Erreur de connexion"));
}
```

**Fichier à créer : `src/DialogWebPublish.ui`**

Layout du dialog Qt Designer :
- QVBoxLayout principal
  - QGroupBox "Configuration du serveur"
    - QFormLayout :
      - Label "URL du Worker :" + QLineEdit `editWorkerUrl` (placeholderText: "https://pongify-live.xxx.workers.dev")
      - Label "Secret admin :" + QLineEdit `editAdminSecret` (echoMode: Password)
      - QCheckBox `checkEnabled` "Activer la publication web"
    - QHBoxLayout :
      - QPushButton `btnTest` "Tester la connexion"
      - QLabel `labelStatus` (texte vide par défaut)
  - QGroupBox "Première utilisation ?"
    - QLabel (wordWrap: true) : "1. Créez un compte gratuit sur cloudflare.com\n2. Créez un token API (Workers Scripts + KV Storage : Edit)\n3. Lancez pongify-setup.exe et suivez les instructions\n4. Collez l'URL et le secret ci-dessus"
  - QDialogButtonBox (Ok + Cancel)

---

### TODO B7 : Créer l'overlay QR Code et l'intégrer dans chaque vue broadcast

**Concept** : au lieu d'une vue broadcast dédiée au QR code, chaque vue broadcast existante (DefaultView, SerieBracketView, RoundRobinView, HallOfFameView) affiche un **QR code contextuel** en superposition dans un coin. Le QR code pointe vers la page webapp correspondante (deep link).

#### B7a : Modifier `BroadcastModel.h` — ajouter `viewSerieIndex`

**Fichier à modifier : `src/BroadcastModel.h`**

Ajouter une propriété pour l'index de la série dans le tournoi (nécessaire pour construire le deep link) :

```cpp
class BroadcastView: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QObject *, viewSerie)
    QML_READONLY_PROPERTY(QString, viewUrl)
    QML_READONLY_PROPERTY(QString, name)
    QML_READONLY_PROPERTY(int, viewSerieIndex)  // ← NOUVEAU : index de la série dans le tournoi
    QML_WRITABLE_PROPERTY(bool, viewVisible)
    QML_WRITABLE_PROPERTY(int, viewTime)
    // ...
};
```

#### B7b : Modifier `BroadcastWindow.h` — ajouter les propriétés web publish

**Fichier à modifier : `src/BroadcastWindow.h`**

Ajouter les propriétés pour que le QML des vues broadcast puisse savoir si la publication est active :

```cpp
QML_READONLY_PROPERTY(bool, webPublishEnabled)
```

#### B7c : Modifier `BroadcastWindow.cpp` — passer les infos au constructeur

**Fichier à modifier : `src/BroadcastWindow.cpp`**

1. Modifier le constructeur pour recevoir l'état de la publication :
   ```cpp
   BroadcastWindow::BroadcastWindow(QScreen *scr, bool fullscreen, Tournament *t,
                                     bool webPublishEnabled, QWidget *parent)
   ```
   Dans le corps : `update_webPublishEnabled(webPublishEnabled);`

2. Dans `reloadViews()`, passer l'index de la série :
   ```cpp
   for (int i = 0; i < currentTournament->serieCount(); i++)
   {
       auto s = currentTournament->getSerie(i);
       // ... dans chaque branche if/else if :
       v->update_viewSerieIndex(i);  // ← NOUVEAU
       // ...
   }
   ```

#### B7d : Créer le composant overlay QR Code

**Fichier à créer : `qml/broadcast/BroadcastQrOverlay.qml`**

Composant réutilisable affiché en superposition dans le coin bas-droit de chaque vue broadcast :

```qml
import QtQuick

Item {
    id: root

    /// Deep link path passé par la vue parente.
    /// Format : "home" | "serie/INDEX" | "serie/INDEX/round/ROUND" | "podium/INDEX"
    property string deepLink: "home"

    visible: broadcastWindow.webPublishEnabled

    width: 240
    height: 280

    anchors {
        bottom: parent.bottom
        right: parent.right
        margins: 20
    }

    Rectangle {
        anchors.fill: parent
        color: "#1a1a2e"
        radius: 12
        opacity: 0.92
        border.color: "#4fc1e9"
        border.width: 1

        Column {
            anchors.centerIn: parent
            spacing: 10

            Image {
                id: qrImage
                source: root.visible ? "image://qrcode/" + root.deepLink : ""
                width: 180
                height: 180
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
                cache: false
            }

            Text {
                text: "📱 Scannez-moi !"
                color: "#4fc1e9"
                font.pixelSize: 18
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
```

#### B7e : Intégrer l'overlay dans chaque vue broadcast

**Fichier à modifier : `qml/broadcast/DefaultView.qml`**

Ajouter à la fin, avant le `}` de fermeture du Rectangle racine :

```qml
import "."  // ou ajuster le chemin si nécessaire

// ... contenu existant ...

BroadcastQrOverlay {
    deepLink: "home"
}
```

DefaultView n'a pas de `viewSerie` et pas de `viewSerieIndex`. Le deep link `"home"` mène à `HomePage.vue` qui affiche les infos du tournoi (`currentTournament.infoText`, `currentTournament.date`, `currentTournament.name`).

**Fichier à modifier : `qml/broadcast/SerieBracketView.qml`**

Ajouter à la fin, dans le Rectangle racine (au même niveau que `title` et `Flickable`) :

```qml
BroadcastQrOverlay {
    deepLink: "serie/" + viewSerieIndex + "/round/" + (viewSerie ? viewSerie.currentRound : 0)
}
```

Le QR code affiche un deep link vers la page série avec le round actuel pré-sélectionné.

**Fichier à modifier : `qml/broadcast/RoundRobinView.qml`**

```qml
BroadcastQrOverlay {
    deepLink: "serie/" + viewSerieIndex
}
```

**Fichier à modifier : `qml/broadcast/HallOfFameView.qml`**

```qml
BroadcastQrOverlay {
    deepLink: "podium/" + viewSerieIndex
}
```

---

### TODO B8 : Modifier `res.qrc`

**Fichier à modifier : `res.qrc`**

Ajouter l'entrée suivante dans le bloc `<qresource prefix="/">` :

```xml
<file>qml/broadcast/BroadcastQrOverlay.qml</file>
```

Et ajouter un nouveau bloc pour les fichiers webapp (qui seront uploadés sur Cloudflare) :

```xml
<qresource prefix="/webapp">
    <file alias="index.html">web/frontend/dist/index.html</file>
    <file alias="app.js">web/frontend/dist/app.js</file>
    <file alias="app.css">web/frontend/dist/app.css</file>
</qresource>
```

Note : les noms exacts des fichiers dans `dist/` dépendront du build Vite. Si Vite génère des noms avec hash (ex: `app-abc123.js`), configurer Vite pour des noms fixes dans `vite.config.js` :
```javascript
build: {
  rollupOptions: {
    output: {
      entryFileNames: 'app.js',
      assetFileNames: 'app.[ext]'
    }
  }
}
```

---

### TODO B9 : Intégrer dans MainWindow

**Fichier à modifier : `src/MainWindow.h`**

Ajouter les includes et déclarations forward :
```cpp
class WebPublisher;
class QrCodeProvider;
```

Ajouter les propriétés QML (dans la section Q_OBJECT, après les propriétés existantes) :
```cpp
QML_READONLY_PROPERTY(bool, webPublishEnabled)
QML_READONLY_PROPERTY(QString, webPublishUrl)
```

Ajouter les membres privés (dans la section `private:`) :
```cpp
QrCodeProvider *qrCodeProvider = nullptr;
```

Ajouter les méthodes Q_INVOKABLE :
```cpp
Q_INVOKABLE void webPublishConfig();
Q_INVOKABLE void webPublishToggle();
```

**Fichier à modifier : `src/MainWindow.cpp`**

1. **Ajouter les includes** en haut du fichier :
   ```cpp
   #include "WebPublisher.h"
   #include "QrCodeProvider.h"
   #include "DialogWebPublish.h"
   ```

2. **Dans le constructeur `MainWindow::MainWindow()`** — après l'initialisation du `previewProvider`, ajouter :
   ```cpp
   qrCodeProvider = new QrCodeProvider();
   update_webPublishEnabled(false);
   update_webPublishUrl("");
   ```

3. **Dans `loadQmlApp()`** — après la ligne `view->engine()->addImageProvider("broadcastpreview", previewProvider);`, ajouter :
   ```cpp
   view->engine()->addImageProvider("qrcode", qrCodeProvider);
   ```

4. **Ajouter les méthodes** `webPublishConfig()` et `webPublishToggle()` :

   ```cpp
   void MainWindow::webPublishConfig()
   {
       DialogWebPublish d(this);
       if (d.exec() == QDialog::Accepted)
       {
           // Si activé et tournoi ouvert, démarrer la publication
           if (WebPublisher::Instance()->enabled() && currentTournament)
               webPublishToggle();
       }
   }

   void MainWindow::webPublishToggle()
   {
       if (!currentTournament) return;

       auto wp = WebPublisher::Instance();
       if (!wp->enabled() || wp->workerUrl().isEmpty()) {
           webPublishConfig();
           return;
       }

       if (get_webPublishEnabled()) {
           // Arrêter la publication
           wp->unpublish();
           update_webPublishEnabled(false);
           update_webPublishUrl("");
           return;
       }

       // Démarrer la publication
       // Générer writeSecret si absent
       if (currentTournament->get_writeSecret().isEmpty()) {
           currentTournament->update_writeSecret(WebPublisher::generateSecret());
       }

       // Générer encryptionKey si absente
       if (currentTournament->get_encryptionKey().isEmpty()) {
           QByteArray key = WebPublisher::generateKey();
           QString keyB64 = key.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
           currentTournament->update_encryptionKey(keyB64);
       }

       // Sauvegarder immédiatement (writeSecret + encryptionKey dans le .tnm)
       TStorage::Instance()->saveToDisk(currentTournament);

       // Configurer le QR code provider avec l'URL de base et la clé
       QString baseUrl = wp->workerUrl() + "/t/" + currentTournament->get_uuid();
       qrCodeProvider->setBaseInfo(baseUrl, currentTournament->get_encryptionKey());

       // L'URL publique complète (pour affichage dans les logs/config)
       QString publicUrl = baseUrl + "#K=" + currentTournament->get_encryptionKey();
       update_webPublishUrl(publicUrl);
       update_webPublishEnabled(true);

       // Le QR code est désormais généré à la demande par QrCodeProvider::requestImage()
       // Chaque vue broadcast affiche son propre overlay QR avec un deep link contextuel

       // Uploader la webapp si nécessaire
       wp->uploadWebappIfNeeded(pongify_version);

       // Connecter et démarrer la publication
       wp->setTournament(currentTournament);
   }
   ```

5. **Dans le bloc `tournamentOpenedChanged`** — dans le lambda existant qui met à jour le QML context, ajouter à la fin (quand `en == false`, donc quand on ferme un tournoi) :
   ```cpp
   if (!en) {
       WebPublisher::Instance()->clearTournament();
       qrCodeProvider->clear();
       update_webPublishEnabled(false);
       update_webPublishUrl("");
   }
   ```

6. **Ajouter le menu** — deux options :
   - Il faut ajouter dans le `.ui` (via Qt Designer) un nouveau menu "Outils" s'il n'existe pas, ou ajouter les actions dans le code.
   - Comme le menu est défini dans `MainWindow.ui`, il faut soit modifier le `.ui`, soit ajouter les actions par code dans le constructeur.
   - **Approche recommandée** : ajouter par code dans le constructeur, après `ui->setupUi(this)` :

   ```cpp
   // Menu Outils (créer s'il n'existe pas)
   QMenu *toolsMenu = ui->menubar->addMenu(tr("&Outils"));
   QAction *actionWebConfig = toolsMenu->addAction(tr("Configuration publication web..."));
   connect(actionWebConfig, &QAction::triggered, this, &MainWindow::webPublishConfig);

   QAction *actionWebToggle = toolsMenu->addAction(tr("Publier en ligne"));
   actionWebToggle->setCheckable(true);
   connect(actionWebToggle, &QAction::triggered, this, &MainWindow::webPublishToggle);
   connect(this, &MainWindow::webPublishEnabledChanged, actionWebToggle, &QAction::setChecked);
   ```

---

### TODO B10 : Modifier `BroadcastWindow` pour les QR codes contextuels

**Plus besoin d'ajouter une vue QR code dédiée** — chaque vue broadcast affiche son propre overlay. Il faut cependant :

1. Passer `webPublishEnabled` à BroadcastWindow
2. Assigner `viewSerieIndex` à chaque vue

**Fichier à modifier : `src/BroadcastWindow.h`**

Ajouter l'include et la propriété (cf. B7b) :
```cpp
QML_READONLY_PROPERTY(bool, webPublishEnabled)
```

**Fichier à modifier : `src/BroadcastWindow.cpp`**

1. Modifier le constructeur pour recevoir l'état de publication :
   ```cpp
   BroadcastWindow::BroadcastWindow(QScreen *scr, bool fullscreen, Tournament *t,
                                     bool webPublishEnabled, QWidget *parent)
   ```
   Dans le corps : `update_webPublishEnabled(webPublishEnabled);`

2. Enregistrer le `qrcode` image provider dans `loadQmlApp()`, après `view->rootContext()` :
   ```cpp
   // Réutiliser le QrCodeProvider du MainWindow (il a déjà baseUrl + key)
   auto mw = qobject_cast<MainWindow*>(parent);
   if (mw)
       view->engine()->addImageProvider("qrcode", mw->qrCodeProvider);
   ```
   Note : `QQuickView::engine()->addImageProvider()` ne prend pas ownership, pas de double-free.

3. Dans `reloadViews()`, assigner `viewSerieIndex` à chaque vue :
   ```cpp
   for (int i = 0; i < currentTournament->serieCount(); i++)
   {
       auto s = currentTournament->getSerie(i);

       if (s->get_status() == "playing" && s->get_tournamentType() == "single")
       {
           auto v = new BroadcastView(this);
           v->update_viewUrl("qrc:/qml/broadcast/SerieBracketView.qml");
           v->update_viewSerie(s);
           v->update_viewSerieIndex(i);  // ← NOUVEAU
           // ... reste identique
       }
       else if (s->get_status() == "playing" && s->get_tournamentType() == "roundrobin")
       {
           auto v = new BroadcastView(this);
           v->update_viewUrl("qrc:/qml/broadcast/RoundRobinView.qml");
           v->update_viewSerie(s);
           v->update_viewSerieIndex(i);  // ← NOUVEAU
           // ... reste identique
       }
       else if (s->get_status() == "finished")
       {
           auto v = new BroadcastView(this);
           v->update_viewUrl("qrc:/qml/broadcast/HallOfFameView.qml");
           v->update_viewSerie(s);
           v->update_viewSerieIndex(i);  // ← NOUVEAU
           // ... reste identique
       }
   }
   ```

4. Dans `MainWindow::broadcastStart()`, passer le flag :
   ```cpp
   broadcastWindow = new BroadcastWindow(screen, fullscreen, currentTournament,
                                          get_webPublishEnabled(), this);
   ```

---

### TODO B11 : Connecter la publication web aux changements de séries/tables dans Tournament.cpp

**Fichier à modifier : `src/Tournament.cpp`**

Dans `addSerie()`, après les `connect()` existants pour `saveToDisk`, ajouter :
```cpp
if (WebPublisher::Instance()->enabled()) {
    connect(s, &TSerie::matchesUpdated, WebPublisher::Instance(), &WebPublisher::publish);
    connect(s, &TSerie::viewVisibleChanged, WebPublisher::Instance(), &WebPublisher::publish);
    connect(s, &TSerie::statusChanged, WebPublisher::Instance(), &WebPublisher::publish);
}
```

Note : cette approche est alternative à celle de `WebPublisher::setTournament()`. Il faut choisir UNE des deux approches. L'approche recommandée est de garder toute la logique dans `WebPublisher::setTournament()` et de NE PAS modifier `Tournament.cpp` pour les connexions de publication. L'avantage est que le WebPublisher s'occupe de ses propres connexions et peut les nettoyer proprement.

**Décision : NE PAS modifier `Tournament.cpp` pour les connect() — WebPublisher::setTournament() s'en occupe.**

Cependant, pour les séries et tables AJOUTÉES APRÈS le setTournament initial, il faut un mécanisme. La solution est de connecter `Tournament::seriesStatusChanged` (déjà émis par addSerie/removeSerie) dans WebPublisher, qui déclenche un publish. Pour les matchesUpdated des nouvelles séries, il faudra que WebPublisher se reconnecte dynamiquement.

**Solution pragmatique** : dans `WebPublisher::setTournament()`, connecter aussi `seriesStatusChanged` → qui appelle une méthode `reconnectSignals()` pour se reconnecter aux éventuelles nouvelles séries.

---

## PHASE C — Webapp Mobile (Vue.js SPA)

### TODO C1 : Initialiser le projet web/frontend

**Fichiers à créer :**

Répertoire `web/frontend/` avec la structure suivante :

```
web/frontend/
├── package.json
├── vite.config.js
├── index.html
├── postcss.config.js
├── tailwind.config.js
└── src/
    ├── main.js
    ├── App.vue
    ├── router.js
    ├── crypto.js
    ├── composables/
    │   └── useTournament.js
    ├── views/
    │   ├── HomePage.vue
    │   ├── SeriePage.vue
    │   ├── PodiumPage.vue
    │   └── SearchPage.vue
    └── components/
        ├── MatchCard.vue
        ├── BracketColumn.vue
        ├── RankingTable.vue
        ├── PlayerBadge.vue
        ├── BottomNav.vue
        └── StatusBadge.vue
```

**`package.json`** :
```json
{
  "name": "pongify-live",
  "version": "1.0.0",
  "private": true,
  "scripts": {
    "dev": "vite",
    "build": "vite build",
    "preview": "vite preview"
  },
  "dependencies": {
    "vue": "^3.4",
    "vue-router": "^4.3"
  },
  "devDependencies": {
    "@vitejs/plugin-vue": "^5.0",
    "vite": "^5.0",
    "tailwindcss": "^3.4",
    "postcss": "^8.4",
    "autoprefixer": "^10.4"
  }
}
```

**`vite.config.js`** :
```javascript
import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'

export default defineConfig({
  plugins: [vue()],
  build: {
    rollupOptions: {
      output: {
        entryFileNames: 'app.js',
        chunkFileNames: 'app-[name].js',
        assetFileNames: 'app.[ext]'
      }
    }
  }
})
```

**`tailwind.config.js`** :
```javascript
export default {
  content: ['./index.html', './src/**/*.{vue,js}'],
  theme: {
    extend: {
      colors: {
        'pongify': {
          teal: '#4fc1e9',
          gold: '#eed67d',
          dark: '#1a1a2e',
          'dark-lighter': '#2a2a3e',
          green: '#489258',
        }
      }
    }
  }
}
```

**`postcss.config.js`** :
```javascript
export default {
  plugins: {
    tailwindcss: {},
    autoprefixer: {},
  }
}
```

**`web/frontend/src/style.css`** :
```css
@tailwind base;
@tailwind components;
@tailwind utilities;

/* === Composants spécifiques (bracket, round tabs) === */
@layer components {
  .round-tabs {
    @apply flex gap-1 overflow-x-auto py-2;
    -webkit-overflow-scrolling: touch;
  }
  .round-tab {
    @apply px-4 py-1.5 rounded-full text-sm whitespace-nowrap
           border border-gray-700 bg-transparent text-gray-400 cursor-pointer;
  }
  .round-tab.active {
    @apply bg-pongify-teal border-pongify-teal text-pongify-dark;
  }
  .match-card {
    @apply bg-pongify-dark-lighter rounded p-2 px-3;
  }
  .match-card .winner {
    @apply text-pongify-green font-bold;
  }
  .match-card .loser {
    @apply text-gray-500;
  }
  .match-card .score {
    font-variant-numeric: tabular-nums;
  }
}

/* === Bracket full view (tablette/desktop) === */
@media (min-width: 768px) {
  .bracket-grid {
    display: grid;
    grid-auto-flow: column;
    grid-auto-columns: 220px;
    gap: 16px;
    overflow-x: auto;
    padding: 16px 0;
  }
  .bracket-column {
    @apply flex flex-col justify-around gap-2;
  }
}
```

Tailwind CSS est **100% open source** (licence MIT). Seul Tailwind UI (composants payants) est commercial — on ne l'utilise pas. Le tree-shaking de Tailwind en mode build produit un CSS minuscule (~5-10 KB) ne contenant que les classes réellement utilisées.

---

### TODO C2 : Implémenter le module de déchiffrement

**Fichier à créer : `web/frontend/src/crypto.js`**

```javascript
/**
 * Extrait la clé AES du fragment URL (#K=...)
 * @returns {Uint8Array|null} La clé en bytes, ou null si absente
 */
export function getKeyFromFragment() {
  const hash = window.location.hash; // "#K=xxxxxxx"
  if (!hash || !hash.includes('K=')) return null;

  const keyB64 = hash.split('K=')[1];
  if (!keyB64) return null;

  return base64urlDecode(keyB64);
}

/**
 * Déchiffre un blob AES-256-GCM
 * @param {ArrayBuffer} encrypted - IV (12 bytes) + ciphertext + tag (16 bytes)
 * @param {Uint8Array} keyBytes - Clé AES 256 bits (32 bytes)
 * @returns {Promise<object>} Le JSON déchiffré
 */
export async function decryptTournament(encrypted, keyBytes) {
  // Importer la clé
  const cryptoKey = await crypto.subtle.importKey(
    'raw',
    keyBytes,
    { name: 'AES-GCM' },
    false,
    ['decrypt']
  );

  // Extraire IV (12 premiers bytes) et ciphertext+tag (le reste)
  const iv = encrypted.slice(0, 12);
  const ciphertextAndTag = encrypted.slice(12);

  // Déchiffrer (Web Crypto attend ciphertext+tag concaténés pour AES-GCM)
  const plaintext = await crypto.subtle.decrypt(
    { name: 'AES-GCM', iv: new Uint8Array(iv) },
    cryptoKey,
    ciphertextAndTag
  );

  // Parser le JSON
  const json = new TextDecoder().decode(plaintext);
  return JSON.parse(json);
}

/**
 * Décode une chaîne base64url en Uint8Array
 */
function base64urlDecode(str) {
  // Remplacer les caractères base64url par base64 standard
  let base64 = str.replace(/-/g, '+').replace(/_/g, '/');
  // Ajouter le padding si nécessaire
  while (base64.length % 4) base64 += '=';

  const binary = atob(base64);
  const bytes = new Uint8Array(binary.length);
  for (let i = 0; i < binary.length; i++) {
    bytes[i] = binary.charCodeAt(i);
  }
  return bytes;
}
```

---

### TODO C3 : Implémenter le composable `useTournament`

**Fichier à créer : `web/frontend/src/composables/useTournament.js`**

Ce composable implémente le **polling adaptatif multi-niveaux** :

| Méthode | Déclencheur |
|---------|-------------|
| `setPollContext(context)` | Appelé par les vues quand elles montent/changent. `context` = `'playing'` \| `'home'` \| `'stopped'` \| `'finished'` |
| `document.visibilitychange` | Coupe le polling quand l'écran est éteint / onglet masqué, reprend au réveil |

```javascript
import { ref, onMounted, onUnmounted, readonly } from 'vue'
import { getKeyFromFragment, decryptTournament } from '../crypto.js'

// Intervalles de polling selon le contexte de la page affichée
const POLL_INTERVALS = {
  playing: 5000,    // 5s — scores en temps réel
  home: 15000,      // 15s — détection changement de statut
  stopped: 30000,   // 30s — rien ne bouge
  finished: 0       // 0 = polling coupé, les données ne changeront plus
}

export function useTournament(uuid) {
  const tournament = ref(null)
  const isLoading = ref(true)
  const error = ref(null)
  const lastUpdated = ref(null)
  const secondsAgo = ref(0)
  const pollContext = ref('home')

  let pollTimer = null
  let secondsInterval = null
  let lastKnownVersion = 0
  let isVisible = true
  const keyBytes = getKeyFromFragment()

  const apiBase = window.location.origin

  // --- Polling adaptatif ---

  function setPollContext(context) {
    if (pollContext.value === context) return
    pollContext.value = context
    restartPoll()
  }

  function restartPoll() {
    clearTimeout(pollTimer)
    pollTimer = null

    if (!isVisible) return // Écran éteint → pas de poll

    const interval = POLL_INTERVALS[pollContext.value] || POLL_INTERVALS.home
    if (interval === 0) return // finished → pas de poll

    schedulePoll(interval)
  }

  function schedulePoll(interval) {
    pollTimer = setTimeout(async () => {
      await fetchVersion()
      // Re-scheduler seulement si encore visible et pas finished
      if (isVisible && POLL_INTERVALS[pollContext.value] > 0) {
        schedulePoll(POLL_INTERVALS[pollContext.value])
      }
    }, interval)
  }

  // --- Visibility API : couper le polling quand le téléphone est en veille ---

  function handleVisibilityChange() {
    isVisible = document.visibilityState === 'visible'
    if (isVisible) {
      // Réveil : fetch immédiat + reprendre le polling
      fetchVersion()
      restartPoll()
    } else {
      // Écran éteint / onglet masqué : couper le polling
      clearTimeout(pollTimer)
      pollTimer = null
    }
  }

  // --- Fetch ---

  async function fetchVersion() {
    try {
      const res = await fetch(`${apiBase}/api/tournament/${uuid}/version`)
      if (!res.ok) {
        if (res.status === 404) {
          error.value = 'Tournoi non trouvé ou expiré'
          tournament.value = null
        }
        return
      }
      const data = await res.json()
      if (data.updatedAt !== lastKnownVersion) {
        lastKnownVersion = data.updatedAt
        await fetchFullData()
      }
      error.value = null
    } catch (e) {
      error.value = 'Connexion perdue'
    }
  }

  async function fetchFullData() {
    try {
      const res = await fetch(`${apiBase}/api/tournament/${uuid}`)
      if (!res.ok) throw new Error(`HTTP ${res.status}`)

      const encrypted = await res.arrayBuffer()

      if (!keyBytes) {
        error.value = 'Clé de déchiffrement manquante dans l\'URL'
        return
      }

      const data = await decryptTournament(encrypted, keyBytes)
      tournament.value = data
      lastUpdated.value = new Date()
      secondsAgo.value = 0
      isLoading.value = false
      error.value = null
    } catch (e) {
      if (e.name === 'OperationError') {
        error.value = 'Impossible de déchiffrer les données (clé invalide ?)'
      } else {
        error.value = 'Erreur de chargement'
      }
      console.error('Fetch error:', e)
    }
  }

  // --- Lifecycle ---

  onMounted(() => {
    document.addEventListener('visibilitychange', handleVisibilityChange)

    // Fetch initial immédiat
    fetchVersion()
    restartPoll()

    secondsInterval = setInterval(() => {
      if (lastUpdated.value) {
        secondsAgo.value = Math.floor((Date.now() - lastUpdated.value.getTime()) / 1000)
      }
    }, 1000)
  })

  onUnmounted(() => {
    document.removeEventListener('visibilitychange', handleVisibilityChange)
    clearTimeout(pollTimer)
    clearInterval(secondsInterval)
  })

  return {
    tournament: readonly(tournament),
    isLoading: readonly(isLoading),
    error: readonly(error),
    lastUpdated: readonly(lastUpdated),
    secondsAgo: readonly(secondsAgo),
    setPollContext
  }
}
```

**Utilisation dans les vues** — chaque vue appelle `setPollContext` au montage :

```javascript
// Dans SeriePage.vue
import { onMounted, inject } from 'vue'
const { setPollContext } = inject('tournament')

onMounted(() => {
  // serie.status vient des props/tournament data
  setPollContext(serie.value.status) // 'playing', 'stopped', ou 'finished'
})

// Dans HomePage.vue
onMounted(() => setPollContext('home'))

// Dans PodiumPage.vue
onMounted(() => setPollContext('finished'))
```

---

### TODO C4 : Implémenter le router et le layout principal

**Fichier à créer : `web/frontend/src/router.js`**

```javascript
import { createRouter, createWebHashHistory } from 'vue-router'
import HomePage from './views/HomePage.vue'
import SeriePage from './views/SeriePage.vue'
import PodiumPage from './views/PodiumPage.vue'
import SearchPage from './views/SearchPage.vue'

const routes = [
  { path: '/', name: 'home', component: HomePage },
  { path: '/serie/:index', name: 'serie', component: SeriePage, props: true },
  { path: '/podium/:index', name: 'podium', component: PodiumPage, props: true },
  { path: '/search', name: 'search', component: SearchPage },
]

export const router = createRouter({
  history: createWebHashHistory(),
  routes
})
```

**Gestion du hash : clé de chiffrement + deep link + routeur**

Le QR code contextuel encode une URL de la forme :
```
https://worker.workers.dev/t/<uuid>#K=<clé>/<deep-link>
```

Exemples :
- `#K=abc123/`               → HomePage
- `#K=abc123/serie/0?round=2` → SeriePage index 0, round 2
- `#K=abc123/podium/1`        → PodiumPage index 1

Au chargement, `main.js` sépare la clé AES du deep link dans le fragment, stocke la clé en `sessionStorage`, puis initialise le routeur Vue hash mode avec la route extraite.

**Fichier à créer : `web/frontend/src/main.js`**

```javascript
import { createApp } from 'vue'
import App from './App.vue'
import { router } from './router.js'
import './style.css'

// Extraire clé AES + deep link depuis le fragment #K=<key>/<route>
const hash = window.location.hash
if (hash && hash.includes('K=')) {
  const afterK = hash.split('K=')[1]   // "abc123/serie/0?round=2"
  const slashIndex = afterK.indexOf('/')
  if (slashIndex >= 0) {
    const key = afterK.substring(0, slashIndex)     // "abc123"
    const route = afterK.substring(slashIndex)       // "/serie/0?round=2"
    sessionStorage.setItem('pongify_key', key)
    // Remplacer le hash par la route pour Vue Router
    history.replaceState(null, '', window.location.pathname + '#' + route)
  } else {
    // Pas de deep link, juste la clé → homepage
    sessionStorage.setItem('pongify_key', afterK)
    history.replaceState(null, '', window.location.pathname + '#/')
  }
}

createApp(App).use(router).mount('#app')
```

Mettre à jour `crypto.js` pour lire depuis `sessionStorage` :
```javascript
export function getKeyFromFragment() {
  const stored = sessionStorage.getItem('pongify_key')
  if (stored) return base64urlDecode(stored)
  // Fallback: lire depuis le hash actuel (cas non-deep-link)
  const hash = window.location.hash
  if (hash && hash.includes('K=')) {
    return base64urlDecode(hash.split('K=')[1])
  }
  return null
}
```

**Fichier à créer : `web/frontend/src/App.vue`**

```vue
<template>
  <div class="min-h-screen bg-pongify-dark text-white flex flex-col">
    <!-- Header -->
    <header class="bg-pongify-dark border-b border-gray-700 px-4 py-3 flex items-center justify-between sticky top-0 z-10">
      <div class="flex items-center gap-2">
        <span class="text-pongify-teal font-bold text-lg">🏓 Pongify Live</span>
        <span v-if="tournament" class="text-gray-400 text-sm truncate max-w-[180px]">
          {{ tournament.name }}
        </span>
      </div>
      <div class="flex items-center gap-2 text-xs text-gray-500">
        <span v-if="!error" class="inline-block w-2 h-2 rounded-full bg-pongify-green animate-pulse"></span>
        <span v-if="!error">{{ secondsAgo }}s</span>
        <span v-else class="text-red-400">{{ error }}</span>
      </div>
    </header>

    <!-- Content -->
    <main class="flex-1 overflow-auto p-4">
      <div v-if="isLoading" class="flex items-center justify-center h-64">
        <span class="text-gray-400">Chargement...</span>
      </div>
      <router-view v-else :tournament="tournament" />
    </main>

    <!-- Bottom Nav -->
    <BottomNav v-if="tournament" />
  </div>
</template>

<script setup>
import { provide } from 'vue'
import { useTournament } from './composables/useTournament.js'
import BottomNav from './components/BottomNav.vue'

// Extraire l'UUID depuis le pathname: /t/:uuid
const pathParts = window.location.pathname.split('/')
const uuid = pathParts[2] || ''

const { tournament, isLoading, error, secondsAgo, setPollContext } = useTournament(uuid)

// Fournir setPollContext aux vues enfant via inject
provide('tournament', { setPollContext })
</script>
```

---

### TODO C5 : Implémenter les vues principales

**Fichier à créer : `web/frontend/src/views/HomePage.vue`**

Page d'accueil du tournoi — c'est la page cible du QR code de la **DefaultView** broadcast.

Affiche en priorité les **informations du tournoi**, puis la liste des séries :

1. **Infos tournoi** (section principale, en haut) :
   - `tournament.name` — nom du tournoi, gros titre
   - `tournament.date` — date formatée en français (ex: "samedi 15 mars 2025")
   - `tournament.infoText` — texte riche (HTML) affiché avec `v-html`. Ce champ contient les infos saisies par l'organisateur (lieu, horaires, règlement, etc.)

2. **Liste des séries** (en dessous) : affiche le nom, la date, et la liste des séries avec leur statut. Chaque série est cliquable pour naviguer vers la vue détaillée.

Données à afficher depuis `tournament.series[]` :
- `name` : nom de la série
- `status` : "stopped" | "playing" | "finished"
- `tournamentType` : "single" | "roundrobin"
- `isDouble` : boolean
- `isHandicap` : boolean

Icônes de statut : ⏸ stopped, ▶ playing, 🏆 finished

**IMPORTANT** : Si l'utilisateur arrive via un QR code deep link (ex: scanné depuis SerieBracketView), le routeur navigue directement vers `/serie/:index` ou `/podium/:index`. `HomePage.vue` n'est affichée que si le deep link est `/` (DefaultView).

**Fichier à créer : `web/frontend/src/views/SeriePage.vue`**

Vue détaillée d'une série. Reçoit `index` en prop (du router). Si le query param `round` est présent (deep link depuis SerieBracketView), le round correspondant est pré-sélectionné au montage.

**Si `tournamentType == "single"` (élimination directe)** :

Affichage mobile-first **par round** : un seul round visible à la fois, navigation par tabs horizontales scrollables.

```
  ┌─────────────────────────────────────────────────────┐
  │  [ 1/8 ]  [ 1/4 ]  [●1/2 ]  [ Finale ]              │  ← round-tabs (scrollable)
  ├─────────────────────────────────────────────────────┤
  │                                                     │
  │  ┌─────────────────────────────────────┐            │
  │  │  Dupont J. (1150)        3          │            │
  │  │  Martin P. (980)         1     ✓    │  ← MatchCard
  │  └─────────────────────────────────────┘            │
  │                                                     │
  │  ┌─────────────────────────────────────┐            │
  │  │  Bernard L. (1320)       3          │            │
  │  │  Petit A. (1100)         2     ✓    │            │
  │  └─────────────────────────────────────┘            │
  │                                                     │
  │  ...                                                │
  └─────────────────────────────────────────────────────┘
```

- **Mobile** (< 768px) : tabs `round-tabs` + liste verticale de `MatchCard`
- **Tablette/Desktop** (≥ 768px) : grille CSS Grid multi-colonnes (`bracket-grid` + `bracket-column`), tous les rounds visibles côte à côte avec connecteurs visuels en CSS (bordures `::before`/`::after`)
- **Pur CSS** : zéro librairie de bracket, juste flexbox + grid + pseudo-éléments pour les lignes de connexion
- Le round actif est pré-sélectionné au round le plus avancé qui contient des matchs joués

Implémentation des tabs dans `SeriePage.vue` :
```vue
<div class="round-tabs">
  <button v-for="(round, i) in serie.rounds" :key="i"
    :class="['round-tab', { active: activeRound === i }]"
    @click="activeRound = i">
    {{ roundLabel(i, serie.rounds.length) }}
  </button>
</div>

<div class="flex flex-col gap-2">
  <MatchCard v-for="(match, j) in serie.rounds[activeRound]" :key="j"
    :match="match" :players="serie.players" :is-double="serie.isDouble" />
</div>
```

Fonction `roundLabel(index, totalRounds)` :
- `totalRounds - index == 1` → "Finale"
- `totalRounds - index == 2` → "1/2"
- `totalRounds - index == 3` → "1/4"
- `totalRounds - index == 4` → "1/8"
- sinon → "Tour " + (index + 1)

**Si `tournamentType == "roundrobin"` (poules)** : afficher `RankingTable.vue`
  - Grille des résultats
  - Classement calculé (même algorithme que TSerie.cpp `calculateRRWinners`)

Données du bracket depuis `tournament.series[index].rounds[][]` :
- Chaque match : `{ player1, player2, playerScore1, playerScore2, playerWinner1, playerWinner2, isBye }`
- Les joueurs sont référencés par licence → lookup dans `tournament.series[index].players[]`

**Fichier à créer : `web/frontend/src/views/PodiumPage.vue`**

Affiche le podium d'une série terminée.
- Données depuis `tournament.series[index].podium[]`
  - Pour single : liste de licences en ordre de classement
  - Pour RR : liste d'objets `{ player, score, setWin, setLoose, winCount }`
- Affichage : médailles 🥇🥈🥉, nom des joueurs, stats

**Fichier à créer : `web/frontend/src/views/SearchPage.vue`**

Recherche de joueur par nom.
- Input texte avec recherche instantanée (filter)
- Parcourir tous les `players[]` de toutes les séries
- Dédupliquer par licence
- Pour chaque joueur trouvé : afficher toutes ses séries et matchs
- Chaque match montre : adversaire, score, résultat (V/D)

---

### TODO C6 : Implémenter les composants réutilisables

**Fichier à créer : `web/frontend/src/components/MatchCard.vue`**

Composant pour afficher un match unique.
- Props : `match` (objet match), `players` (tableau de joueurs pour le lookup par licence), `isDouble` (boolean)
- Affiche : nom joueur 1 vs nom joueur 2, scores, indicateur de vainqueur
- Si `isBye` : afficher "Bye" pour le joueur absent
- Si doubles : afficher les deux noms de chaque côté
- Style cohérent avec le broadcast Pongify (fond sombre, texte clair)

**Fichier à créer : `web/frontend/src/components/BracketColumn.vue`**

Colonne d'un round dans un bracket d'élimination directe. Utilisé uniquement en **mode desktop** (≥ 768px) pour le bracket complet multi-colonnes.
- Props : `round` (tableau de matchs), `roundIndex`, `totalRounds`, `players`, `isDouble`
- Affiche les `MatchCard` empilés verticalement avec espacement croissant par round (les matchs des rounds avancés sont centrés entre leurs "parents")
- Connecteurs visuels en CSS pur : bordures `::after` sur chaque `MatchCard` pour dessiner les lignes reliant les vainqueurs au match suivant
- Sur mobile (< 768px) ce composant n'est pas utilisé — c'est la navigation par tabs dans `SeriePage.vue` qui prend le relais

**Fichier à créer : `web/frontend/src/components/RankingTable.vue`**

Tableau de classement round-robin.
- Props : `serie` (objet série complet)
- Calculer les scores en JavaScript (reproduire l'algo de `TSerie.cpp calculateRRWinners`) :
  - 2 pts pour victoire en 2 sets
  - 1 pt pour victoire en 3+ sets
  - 0 pt pour défaite
  - Tri : winCount desc → score desc → setWin desc → setLoose asc

**Fichier à créer : `web/frontend/src/components/PlayerBadge.vue`**

Badge compact pour afficher un joueur.
- Props : `player` (objet joueur), `showClub` (boolean)
- Affiche : prénom, nom, [classement], (club si demandé)
- Si doubles : "Prénom1 Nom1 / Prénom2 Nom2"

**Fichier à créer : `web/frontend/src/components/BottomNav.vue`**

Navigation bottom tabs mobile.
- 4 tabs : Accueil | Séries | Podium | Recherche
- Icônes + labels
- Active tab highlighted en teal
- Vue Router navigation

**Fichier à créer : `web/frontend/src/components/StatusBadge.vue`**

Badge de statut de série.
- Props : `status` ("stopped" | "playing" | "finished")
- Couleurs : gris/vert/gold
- Texte : "Non démarré" / "En cours" / "Terminé"

---

### TODO C7 : Build et intégration dans les ressources Qt

Après l'implémentation de la webapp :

```bash
cd web/frontend
npm install
npm run build
```

Les fichiers générés dans `web/frontend/dist/` seront référencés par `res.qrc` et embarqués dans le binaire Pongify.

---

## PHASE D — Tests et validation

### TODO D1 : Tests du Worker (manuels via curl)

```bash
# Variables
WORKER_URL="https://pongify-live.xxx.workers.dev"
UUID="test-$(uuidgen)"
SECRET="test-secret-12345678901234567890"
ADMIN_SECRET="xxx" # celui généré par pongify-setup

# 1. Health check
curl -s "$WORKER_URL/api/health"
# → {"ok":true}

# 2. PUT un tournoi (blob fictif pour le test)
echo -n "test-encrypted-data-placeholder" | \
  curl -s -X PUT -H "X-Write-Secret: $SECRET" \
  -H "X-Pongify-Version: 2.1" \
  -H "Content-Type: application/octet-stream" \
  --data-binary @- "$WORKER_URL/api/tournament/$UUID"
# → 201

# 3. GET le tournoi
curl -s -o /dev/null -w "%{http_code}" "$WORKER_URL/api/tournament/$UUID"
# → 200

# 4. GET la version
curl -s "$WORKER_URL/api/tournament/$UUID/version"
# → {"updatedAt":1709640000}

# 5. PUT avec mauvais secret → 403
curl -s -X PUT -H "X-Write-Secret: wrong" \
  -d "data" "$WORKER_URL/api/tournament/$UUID"
# → 403

# 6. DELETE
curl -s -X DELETE -H "X-Write-Secret: $SECRET" "$WORKER_URL/api/tournament/$UUID"
# → 200

# 7. Upload webapp
echo "<html>test</html>" | \
  curl -s -X PUT -H "X-Admin-Secret: $ADMIN_SECRET" \
  --data-binary @- "$WORKER_URL/api/webapp/2.1/index.html"
# → 200

# 8. Vérifier webapp
curl -s -o /dev/null -w "%{http_code}" "$WORKER_URL/api/webapp/2.1/check"
# → 200
```

### TODO D2 : Tests end-to-end

1. Compiler et lancer `pongify-setup.exe` → obtenir URL + admin secret
2. Configurer dans Pongify (Outils → Configuration publication web)
3. Tester la connexion ✓
4. Créer un tournoi, ajouter des joueurs, démarrer une série
5. Clic "Publier en ligne" → QR code apparaît dans le broadcast
6. Scanner le QR code sur un téléphone en **4G** (pas WiFi !)
7. Vérifier que la webapp s'ouvre et affiche le tournoi
8. Modifier un score dans Pongify → vérifier mise à jour mobile **< 10 secondes**
9. Ouvrir la page recherche → chercher un joueur par nom
10. Terminer la série → vérifier que le podium apparaît
11. Cliquer "Arrêter la publication" → le endpoint retourne 404
12. Ouvrir 50 onglets simultanés pour test de charge (pas de throttle)
13. Vérifier les logs Worker dans le dashboard Cloudflare

### TODO D3 : Tests de sécurité

1. Essayer de PUT avec un secret différent → 403
2. Essayer de PUT un body > 512KB → 413
3. Essayer de PUT deux fois en < 1 seconde → 429 (rate limit)
4. Accéder au blob sans la clé (retirer #K= de l'URL) → erreur de déchiffrement affichée proprement dans la webapp
5. Vérifier que le Worker ne log pas les données du body (zero-knowledge)

---

## Résumé des fichiers

### Fichiers à créer

| Fichier | Phase | Description |
|---------|-------|-------------|
| `pongify_setup/go.mod` | A | Module Go |
| `pongify_setup/main.go` | A | Outil setup Cloudflare (~200 lignes) |
| `pongify_setup/worker.js` | A | Code Cloudflare Worker (~150 lignes) |
| `3rd_party/qrcodegen/qrcodegen.hpp` | B | Lib QR code nayuki (copier depuis GitHub) |
| `3rd_party/qrcodegen/qrcodegen.cpp` | B | Lib QR code nayuki (copier depuis GitHub) |
| `src/WebPublisher.h` | B | Publication HTTP chiffrée vers Worker |
| `src/WebPublisher.cpp` | B | Implémentation WebPublisher |
| `src/QrCodeProvider.h` | B | QQuickImageProvider pour QR codes contextuels (à la demande) |
| `src/DialogWebPublish.h` | B | Dialog de configuration |
| `src/DialogWebPublish.cpp` | B | Implémentation dialog |
| `src/DialogWebPublish.ui` | B | Layout Qt Designer |
| `qml/broadcast/BroadcastQrOverlay.qml` | B | Overlay QR code réutilisable (coin bas-droit de chaque vue) |
| `web/frontend/package.json` | C | Config npm |
| `web/frontend/vite.config.js` | C | Config Vite |
| `web/frontend/index.html` | C | Point d'entrée HTML |
| `web/frontend/src/main.js` | C | Bootstrap Vue app (extraction clé + deep link depuis hash) |
| `web/frontend/src/App.vue` | C | Layout racine |
| `web/frontend/src/router.js` | C | Vue Router config |
| `web/frontend/src/crypto.js` | C | Déchiffrement AES-256-GCM |
| `web/frontend/tailwind.config.js` | C | Config Tailwind (couleurs pongify-*) |
| `web/frontend/postcss.config.js` | C | Config PostCSS (tailwindcss + autoprefixer) |
| `web/frontend/src/style.css` | C | Directives Tailwind + composants custom (round-tabs, match-card, bracket) |
| `web/frontend/src/composables/useTournament.js` | C | Polling adaptatif + visibility API + state réactif |
| `web/frontend/src/views/HomePage.vue` | C | Page accueil tournoi (infos: nom, date, infoText + liste séries) |
| `web/frontend/src/views/SeriePage.vue` | C | Vue bracket / round-robin (deep link avec round pré-sélectionné) |
| `web/frontend/src/views/PodiumPage.vue` | C | Vue podium |
| `web/frontend/src/views/SearchPage.vue` | C | Recherche joueur |
| `web/frontend/src/components/MatchCard.vue` | C | Composant match |
| `web/frontend/src/components/BracketColumn.vue` | C | Colonne bracket |
| `web/frontend/src/components/RankingTable.vue` | C | Tableau RR |
| `web/frontend/src/components/PlayerBadge.vue` | C | Badge joueur |
| `web/frontend/src/components/BottomNav.vue` | C | Navigation mobile |
| `web/frontend/src/components/StatusBadge.vue` | C | Badge statut |

### Fichiers à modifier

| Fichier | Phase | Modification |
|---------|-------|-------------|
| `pongify.pro` | B | `QT += network`, nouveaux SOURCES/HEADERS/FORMS, INCLUDEPATH qrcodegen |
| `res.qrc` | B | Ajouter BroadcastQrOverlay.qml + webapp/ resources |
| `src/Tournament.h` | B | Ajouter writeSecret + encryptionKey properties |
| `src/Tournament.cpp` | B | toJson: sérialiser writeSecret + encryptionKey. Nouveau toJsonForPublish(). fromJson: lire les nouveaux champs |
| `src/BroadcastModel.h` | B | Ajouter propriété `viewSerieIndex` (int) |
| `src/MainWindow.h` | B | Membres QrCodeProvider, propriétés webPublish*, méthodes webPublishConfig/Toggle |
| `src/MainWindow.cpp` | B | Constructeur + loadQmlApp + menu + méthodes webPublish |
| `src/BroadcastWindow.h` | B | Ajouter propriété `webPublishEnabled`, modifier constructeur |
| `src/BroadcastWindow.cpp` | B | Constructeur reçoit webPublishEnabled, reloadViews() assigne viewSerieIndex, loadQmlApp() enregistre qrcode provider |
| `qml/broadcast/DefaultView.qml` | B | Ajouter BroadcastQrOverlay (deepLink: "home") |
| `qml/broadcast/SerieBracketView.qml` | B | Ajouter BroadcastQrOverlay (deepLink: "serie/INDEX/round/ROUND") |
| `qml/broadcast/RoundRobinView.qml` | B | Ajouter BroadcastQrOverlay (deepLink: "serie/INDEX") |
| `qml/broadcast/HallOfFameView.qml` | B | Ajouter BroadcastQrOverlay (deepLink: "podium/INDEX") |

---

## Ordre d'implémentation recommandé

Les phases A, B et C sont partiellement parallélisables :

```
Phase A (Cloudflare Worker + Go setup)
  A1 → A2 (séquentiel)

Phase B (Intégration Pongify C++)
  B1 (qrcodegen)  ─┐
  B2 (pongify.pro) ─┤
  B3 (Tournament)  ─┼─► B4 (WebPublisher) ─► B9 (MainWindow) ─► B10 (BroadcastWindow)
  B5 (QrCodeProv)  ─┤     B7 (QrOverlay + vues broadcast) ─────┘       ↓
  B6 (Dialog)      ─┤                                                  B11
  B8 (res.qrc)     ─┘

Phase C (Webapp Vue.js)
  C1 (init projet) → C2 (crypto) → C3 (useTournament) → C4 (router/App) → C5 (vues) → C6 (composants) → C7 (build)

Phase D (Tests)
  D1 + D2 + D3 (après A + B + C)
```

**Phases A et C sont indépendantes de B et peuvent être développées en parallèle.**
