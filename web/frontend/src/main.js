import { createApp } from 'vue'
import App from './App.vue'
import { createAppRouter } from './router.js'
import './style.css'

// Extract AES key + deep link from fragment: #K=<key>/<route>
const hash = window.location.hash
if (hash && hash.includes('K=')) {
  const afterK = hash.split('K=')[1]
  const slashIndex = afterK.indexOf('/')
  if (slashIndex >= 0) {
    const key = afterK.substring(0, slashIndex)
    const route = afterK.substring(slashIndex)
    sessionStorage.setItem('pongify_key', key)
    history.replaceState(null, '', window.location.pathname + '#' + route)
  } else {
    sessionStorage.setItem('pongify_key', afterK)
    history.replaceState(null, '', window.location.pathname + '#/')
  }
}

// Create router AFTER hash cleanup so createWebHashHistory() sees the clean hash
const router = createAppRouter()
createApp(App).use(router).mount('#app')
