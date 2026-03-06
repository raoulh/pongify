<template>
  <div class="space-y-3">
    <h1 class="text-xl font-bold text-gray-800">Séries</h1>

    <div v-if="!tournament.series || !tournament.series.length" class="text-gray-400 text-center py-8">
      Aucune série en cours
    </div>

    <router-link v-for="(serie, i) in tournament.series" :key="serie.uid"
                 :to="{ name: 'serie', params: { index: i } }"
                 class="block bg-white rounded-lg p-4 shadow-sm border border-gray-100 hover:ring-1 hover:ring-pongify-teal">
      <div class="flex items-center gap-3">
        <StatusBadge :status="serie.status" />
        <div class="min-w-0 flex-1">
          <div class="font-medium truncate text-gray-800">{{ serie.name }}</div>
          <div class="text-xs text-gray-500 mt-0.5">
            {{ serie.type === 'single' ? '🏓 Élimination directe' : '🔄 Poules' }}
            <span v-if="serie.double"> · Double</span>
            <span v-if="serie.handicap"> · Handicap</span>
            · {{ serie.players?.length || 0 }} joueurs
          </div>
        </div>
        <span class="text-gray-400 text-lg">›</span>
      </div>

      <div v-if="serie.status === 'playing'" class="mt-2 text-xs text-gray-400">
        {{ playedCount(serie) }} / {{ totalMatches(serie) }} matchs joués
      </div>
    </router-link>
  </div>
</template>

<script setup>
import { onMounted, inject } from 'vue'
import StatusBadge from '../components/StatusBadge.vue'

defineProps({ tournament: Object })
const { setPollContext } = inject('tournamentCtx')
onMounted(() => setPollContext('home'))

function playedCount(serie) {
  let count = 0
  for (const round of (serie.rounds || [])) {
    for (const m of round) {
      if (m.playerWinner1 || m.playerWinner2 || m.isBye) count++
    }
  }
  return count
}

function totalMatches(serie) {
  let count = 0
  for (const round of (serie.rounds || [])) count += round.length
  return count
}
</script>
