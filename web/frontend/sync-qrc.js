// sync-qrc.js — Updates res.qrc with the actual dist/ filenames after Vite build.
// Called automatically by "npm run build" via the postbuild hook.

import { readdirSync, readFileSync, writeFileSync } from 'fs'
import { resolve, dirname } from 'path'
import { fileURLToPath } from 'url'

const __dirname = dirname(fileURLToPath(import.meta.url))
const distDir = resolve(__dirname, 'dist')
const qrcPath = resolve(__dirname, '..', '..', 'res.qrc')

const files = readdirSync(distDir).filter(f => !f.startsWith('.'))

const webappEntries = files
  .map(f => `        <file alias="${f}">web/frontend/dist/${f}</file>`)
  .join('\n')

const newSection = `    <qresource prefix="/webapp">\n${webappEntries}\n    </qresource>`

let qrc = readFileSync(qrcPath, 'utf-8')
qrc = qrc.replace(
  /    <qresource prefix="\/webapp">[\s\S]*?<\/qresource>/,
  newSection
)

writeFileSync(qrcPath, qrc, 'utf-8')
console.log(`[sync-qrc] Updated res.qrc with ${files.length} webapp files: ${files.join(', ')}`)
