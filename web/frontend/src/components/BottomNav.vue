<template>
  <nav class="fixed bottom-0 left-0 right-0 bg-pongify-teal z-10 shadow-[0_-2px_8px_rgba(0,0,0,0.1)]">
    <div class="flex justify-around">
      <router-link v-for="tab in tabs" :key="tab.to" :to="tab.to"
                   class="flex flex-col items-center py-2 px-3 text-xs rounded-lg"
                   :class="isActive(tab.to) ? 'text-white bg-white/20' : 'text-white/75'"
                   @click.native="onTabClick(tab, $event)">
        <span class="text-lg">{{ tab.icon }}</span>
        <span>{{ tab.label }}</span>
      </router-link>
    </div>
  </nav>
</template>

<script setup>
import { useRoute, useRouter } from 'vue-router'

const route = useRoute()
const router = useRouter()

function onTabClick(tab, event) {
  if (tab.to === '/search' && route.path === '/search') {
    event.preventDefault()
    router.replace({ path: '/search' })
  }
}

const tabs = [
  { to: '/', icon: '🏠', label: 'Accueil' },
  { to: '/series', icon: '🏓', label: 'Séries' },
  { to: '/podium', icon: '🏆', label: 'Podium' },
  { to: '/tables', icon: '📋', label: 'Tables' },
  { to: '/search', icon: '🔍', label: 'Recherche' },
]

function isActive(to) {
  if (to === '/') return route.path === '/'
  return route.path.startsWith(to)
}
</script>
