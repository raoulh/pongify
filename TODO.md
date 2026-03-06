# Pongify Live — Suivi d'implémentation

> Détails complets dans [PONGIFY_LIVE_PLAN.md](PONGIFY_LIVE_PLAN.md)

## Phase A — Infrastructure Cloud (Cloudflare Worker + Go setup)

- [x] **A1** — Créer le code du Cloudflare Worker (`pongify_setup/worker.js`)
- [x] **A2** — Créer l'outil de setup Go (`pongify_setup/main.go`, `go.mod`)

## Phase B — Intégration Pongify (C++)

- [ ] **B1** — Ajouter la librairie QR Code nayuki (`3rd_party/qrcodegen/`)
- [ ] **B2** — Modifier `pongify.pro` (QT += network, SOURCES, HEADERS, INCLUDEPATH)
- [ ] **B3** — Ajouter `writeSecret` + `encryptionKey` au modèle Tournament
- [ ] **B4** — Créer la classe `WebPublisher` (publication HTTP chiffrée AES-256-GCM)
- [ ] **B5** — Créer le `QrCodeProvider` (génération QR à la demande par deep link)
- [ ] **B6** — Créer le dialog `DialogWebPublish` (configuration serveur)
- [ ] **B7** — Créer l'overlay QR Code et l'intégrer dans chaque vue broadcast
  - [ ] B7a — `BroadcastModel.h` : ajouter propriété `viewSerieIndex`
  - [ ] B7b — `BroadcastWindow.h` : ajouter propriété `webPublishEnabled`
  - [ ] B7c — `BroadcastWindow.cpp` : passer `webPublishEnabled` + assigner `viewSerieIndex`
  - [ ] B7d — Créer `qml/broadcast/BroadcastQrOverlay.qml`
  - [ ] B7e — Intégrer l'overlay dans DefaultView, SerieBracketView, RoundRobinView, HallOfFameView
- [ ] **B8** — Modifier `res.qrc` (ajouter BroadcastQrOverlay + webapp resources)
- [ ] **B9** — Intégrer dans MainWindow (menu, QrCodeProvider, méthodes webPublish)
- [ ] **B10** — Modifier BroadcastWindow (constructeur, qrcode provider, viewSerieIndex)
- [ ] **B11** — Connecter la publication aux changements via `WebPublisher::setTournament()`

## Phase C — Webapp Mobile (Vue.js SPA)

- [x] **C1** — Initialiser le projet `web/frontend/` (package.json, Vite, Tailwind, CDN importmap)
- [x] **C2** — Implémenter le module de déchiffrement (`crypto.js`)
- [x] **C3** — Implémenter le composable `useTournament` (polling adaptatif + visibility API)
- [x] **C4** — Implémenter le router et le layout principal (`main.js`, `router.js`, `App.vue`)
- [x] **C5** — Implémenter les vues principales (HomePage, SeriesListPage, SeriePage, PodiumListPage, PodiumPage, TablesPage, SearchPage)
- [x] **C6** — Implémenter les composants réutilisables (MatchCard, BracketColumn, RankingTable, PlayerBadge, BottomNav, StatusBadge)
- [x] **C7** — Implémenter l'utilitaire handicap (`utils/handicap.js`)
- [ ] **C8** — Build et intégration dans les ressources Qt

## Phase D — Tests et validation

- [x] **D1** — Tests du Worker (manuels via curl)
- [ ] **D2** — Tests end-to-end (publication, QR scan, mise à jour live, multi-onglets)
- [ ] **D3** — Tests de sécurité (mauvais secret, body trop gros, rate limit, clé manquante)
