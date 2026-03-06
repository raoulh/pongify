<template>
  <div class="space-y-6">
    <div class="text-center">
      <h1 class="text-2xl font-bold text-pongify-teal">{{ tournament.name }}</h1>
      <p class="text-gray-500 mt-1">{{ formatDate(tournament.date) }}</p>
    </div>

    <div v-if="tournament.info_text"
         class="prose max-w-none bg-white rounded-lg p-4 text-sm text-gray-600 shadow-sm border border-gray-100"
         v-html="tournament.info_text">
    </div>

    <div v-if="tournament.series && tournament.series.length" class="space-y-2">
      <h2 class="text-lg font-semibold text-gray-700">Séries</h2>
      <router-link v-for="(serie, i) in tournament.series" :key="serie.uid"
                   :to="{ name: 'serie', params: { index: i } }"
                   class="block bg-white rounded-lg p-3 flex items-center gap-3 shadow-sm border border-gray-100 hover:ring-1 hover:ring-pongify-teal">
        <StatusBadge :status="serie.status" />
        <div class="min-w-0 flex-1">
          <div class="font-medium truncate text-gray-800">{{ serie.name }}</div>
          <div class="text-xs text-gray-500">
            {{ serie.type === 'single' ? 'Élimination directe' : 'Poules' }}
            <span v-if="serie.double"> · Double</span>
            <span v-if="serie.handicap"> · Handicap</span>
          </div>
        </div>
        <span class="text-xs text-gray-400">{{ serie.players?.length || 0 }} joueurs</span>
      </router-link>
    </div>
  </div>
</template>

<script setup>
import { onMounted, inject } from 'vue'
import StatusBadge from '../components/StatusBadge.vue'

defineProps({ tournament: Object })
const { setPollContext } = inject('tournamentCtx')
onMounted(() => setPollContext('home'))

function formatDate(dateStr) {
  if (!dateStr) return ''
  const d = new Date(dateStr)
  if (isNaN(d.getTime())) return dateStr
  return d.toLocaleDateString('fr-FR', { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' })
}
</script>
