<template>
  <div class="min-h-screen bg-gray-50 text-gray-800 flex flex-col">
    <header class="bg-pongify-teal px-4 py-2 sticky top-0 z-10 shadow-md">
      <div class="flex items-center justify-between">
        <span class="text-white font-bold text-lg whitespace-nowrap">🏓 Pongify Live</span>
        <div class="flex items-center gap-2 text-xs text-white/80 whitespace-nowrap">
          <template v-if="!error">
            <span class="inline-block w-2 h-2 rounded-full animate-pulse" :class="secondsAgo !== null && secondsAgo > 3600 ? 'bg-red-400' : secondsAgo !== null && secondsAgo > 900 ? 'bg-yellow-400' : 'bg-pongify-green'"></span>
            <span v-if="lastUpdateDisplay">{{ lastUpdateDisplay }}</span>
          </template>
          <span v-else class="text-red-200 truncate max-w-[200px]">{{ error }}</span>
        </div>
      </div>
      <div v-if="tournament" class="text-center text-sm text-white/70 truncate mt-0.5">
        {{ tournament.name }}
      </div>
    </header>

    <main class="flex-1 overflow-auto p-4 pb-20">
      <div v-if="error && !tournament" class="flex flex-col items-center justify-center h-64 text-center px-4">
        <div class="text-red-500 text-4xl mb-4">⚠️</div>
        <h2 class="text-lg font-semibold text-gray-700 mb-2">Erreur</h2>
        <p class="text-gray-600 text-sm max-w-sm">{{ error }}</p>
        <button @click="retryLoad" class="mt-6 px-4 py-2 bg-pongify-teal text-white rounded-lg text-sm font-medium hover:opacity-90">
          Réessayer
        </button>
      </div>
      <div v-else-if="isLoading" class="flex items-center justify-center h-64">
        <span class="text-gray-400">Chargement...</span>
      </div>
      <router-view v-else :tournament="tournament" />
    </main>

    <BottomNav v-if="tournament" />
  </div>
</template>

<script setup>
import { provide } from 'vue'
import { useTournament } from './composables/useTournament.js'
import BottomNav from './components/BottomNav.vue'

const pathParts = window.location.pathname.split('/')
const uuid = pathParts[2] || ''

const { tournament, isLoading, error, lastUpdateDisplay, secondsAgo, setPollContext, retryLoad } = useTournament(uuid)

provide('tournamentCtx', { setPollContext })
</script>
