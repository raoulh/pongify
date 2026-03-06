<template>
  <div class="min-h-screen bg-gray-50 text-gray-800 flex flex-col">
    <header class="bg-pongify-teal px-4 py-2 sticky top-0 z-10 shadow-md">
      <div class="flex items-center justify-between">
        <span class="text-white font-bold text-lg whitespace-nowrap">🏓 Pongify Live</span>
        <div class="flex items-center gap-2 text-xs text-white/80 whitespace-nowrap">
          <template v-if="!error">
            <span class="inline-block w-2 h-2 rounded-full bg-pongify-green animate-pulse"></span>
            <span v-if="lastUpdateTime">{{ lastUpdateTime }}</span>
          </template>
          <span v-else class="text-red-200">{{ error }}</span>
        </div>
      </div>
      <div v-if="tournament" class="text-center text-sm text-white/70 truncate mt-0.5">
        {{ tournament.name }}
      </div>
    </header>

    <main class="flex-1 overflow-auto p-4 pb-20">
      <div v-if="isLoading" class="flex items-center justify-center h-64">
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

const { tournament, isLoading, error, lastUpdateTime, setPollContext } = useTournament(uuid)

provide('tournamentCtx', { setPollContext })
</script>
