import { createApp } from 'vue'
import App from './App.vue'
import { createAppRouter } from './router.js'
import './style.css'

// Extract UUID from pathname: /t/<uuid>/
const pathSegments = window.location.pathname.split('/')
const tournamentUuid = pathSegments[2] || ''
console.log('[Pongify] UUID from path:', tournamentUuid)

// Extract AES key + deep link from fragment: #K=<key>/<route>
const hash = window.location.hash
if (hash && hash.includes('K=')) {
  const afterK = hash.split('K=')[1]
  const slashIndex = afterK.indexOf('/')
  if (slashIndex >= 0) {
    const key = afterK.substring(0, slashIndex)
    const route = afterK.substring(slashIndex)
    console.log('[Pongify] Key extracted from URL, length:', key.length, 'route:', route)
    localStorage.setItem('pongify_key_' + tournamentUuid, key)
    history.replaceState(null, '', window.location.pathname + '#' + route)
  } else {
    console.log('[Pongify] Key extracted from URL (no route), length:', afterK.length)
    localStorage.setItem('pongify_key_' + tournamentUuid, afterK)
    history.replaceState(null, '', window.location.pathname + '#/')
  }
} else {
  const existingKey = localStorage.getItem('pongify_key_' + tournamentUuid)
  console.log('[Pongify] No key in URL hash.', existingKey ? 'Using cached key, length: ' + existingKey.length : 'No cached key in localStorage!')
}

// Create router AFTER hash cleanup so createWebHashHistory() sees the clean hash
const router = createAppRouter()
createApp(App).use(router).mount('#app')
