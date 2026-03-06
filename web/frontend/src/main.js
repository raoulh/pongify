import { createApp } from 'vue'
import App from './App.vue'
import { router } from './router.js'
import './style.css'

// Extract AES key + deep link from fragment: #K=<key>/<route>
console.log('[Pongify] main.js starting')
console.log('[Pongify] pathname:', window.location.pathname)
console.log('[Pongify] hash:', window.location.hash)
const hash = window.location.hash
if (hash && hash.includes('K=')) {
  const afterK = hash.split('K=')[1]
  const slashIndex = afterK.indexOf('/')
  if (slashIndex >= 0) {
    const key = afterK.substring(0, slashIndex)
    const route = afterK.substring(slashIndex)
    console.log('[Pongify] key extracted, length:', key.length, 'route:', route)
    sessionStorage.setItem('pongify_key', key)
    history.replaceState(null, '', window.location.pathname + '#' + route)
  } else {
    console.log('[Pongify] key extracted (no route), length:', afterK.length)
    sessionStorage.setItem('pongify_key', afterK)
    history.replaceState(null, '', window.location.pathname + '#/')
  }
} else {
  console.log('[Pongify] no K= in hash, sessionStorage key:', sessionStorage.getItem('pongify_key') ? 'present' : 'MISSING')
}

createApp(App).use(router).mount('#app')
console.log('[Pongify] app mounted')
