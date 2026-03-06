<template>
  <div class="space-y-4">
    <h1 class="text-xl font-bold text-gray-800">🏆 Podiums</h1>

    <div v-if="!finishedSeries.length" class="text-gray-400 text-center py-8">
      Aucune série terminée pour l'instant
    </div>

    <router-link v-for="{ serie, originalIndex } in finishedSeries" :key="serie.uid"
         :to="{ name: 'podium', params: { index: originalIndex } }"
         class="block bg-white rounded-lg p-4 space-y-3 shadow-sm border border-gray-100 hover:ring-1 hover:ring-pongify-teal">
      <div class="flex items-center justify-between">
        <h2 class="font-semibold text-gray-800">{{ serie.name }}</h2>
        <span class="text-gray-400 text-sm">Voir le classement complet ›</span>
      </div>

      <div v-if="serie.podium" class="space-y-1">
        <div v-for="(entry, rank) in topThree(serie)" :key="rank"
             class="flex items-center gap-2 text-sm">
          <span class="text-lg">{{ medals[rank] || '' }}</span>
          <PlayerBadge :player="entry.player" :is-double="serie.double" />
        </div>
      </div>
      <div v-else class="text-sm text-gray-400">Podium non validé</div>
    </router-link>
  </div>
</template>

<script setup>
import { computed, onMounted, inject } from 'vue'
import PlayerBadge from '../components/PlayerBadge.vue'

const props = defineProps({ tournament: Object })
const { setPollContext } = inject('tournamentCtx')
onMounted(() => setPollContext('finished'))

const medals = ['🥇', '🥈', '🥉']

const finishedSeries = computed(() => {
  if (!props.tournament?.series) return []
  return props.tournament.series
    .map((serie, i) => ({ serie, originalIndex: i }))
    .filter(({ serie }) => serie.status === 'finished' && serie.podium)
})

function topThree(serie) {
  const playersMap = {}
  for (const p of (serie.players || [])) playersMap[p.license] = p

  return (serie.podium || []).slice(0, 3).map(entry => {
    if (typeof entry === 'string') {
      return { player: playersMap[entry] || { firstname: entry, lastname: '' } }
    }
    return { player: playersMap[entry.player] || { firstname: entry.player, lastname: '' }, ...entry }
  })
}
</script>
