<template>
  <div class="space-y-3">
    <h1 class="text-xl font-bold text-gray-800">📋 Tables</h1>

    <div v-if="!sortedTables.length" class="text-gray-400 text-center py-8">
      Aucune table configurée
    </div>

    <div v-for="t in sortedTables" :key="t.tableNumber"
         class="bg-white rounded-lg p-4 shadow-sm border border-gray-100"
         :class="{ 'ring-2 ring-pongify-gold': t.isFinale }">
      <div class="flex items-center justify-between mb-1">
        <div class="flex items-center gap-2">
          <span class="font-bold text-pongify-teal">Table {{ t.tableNumber }}</span>
          <span v-if="t.roundLabel" class="text-xs px-2 py-0.5 rounded-full font-semibold"
                :class="t.isFinale ? 'bg-pongify-gold text-[#484848]' : 'bg-gray-100 text-gray-500'">{{ t.roundLabel }}</span>
        </div>
        <span v-if="!t.serie" class="text-xs text-gray-400 italic">Libre</span>
        <span v-else class="text-xs text-gray-500">{{ t.serie.name }}</span>
      </div>

      <div v-if="t.match" class="mt-2">
        <MatchCard :match="t.match" :players="tablePlayersMap(t)" :is-double="!!t.serie?.double" :is-handicap="!!t.serie?.handicap" :handicap-table="t.serie?.handicapTable" :hide-score="true" />
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed, onMounted, inject } from 'vue'
import MatchCard from '../components/MatchCard.vue'
import { computeHandicap } from '../utils/handicap.js'

const props = defineProps({ tournament: Object })
const { setPollContext } = inject('tournamentCtx')
onMounted(() => setPollContext('playing'))

const sortedTables = computed(() => {
  if (!props.tournament?.tables) return []

  const seriesMap = {}
  for (const s of (props.tournament.series || [])) {
    seriesMap[s.uid] = s
  }

  const result = props.tournament.tables.map(table => {
    const entry = { tableNumber: table.tableNumber, serie: null, match: null, player1: null, player2: null, handicap1: 0, handicap2: 0, roundLabel: '', isFinale: false }

    if (!table.serieUid) return entry

    const serie = seriesMap[table.serieUid]
    if (!serie) return entry
    entry.serie = serie

    const round = serie.rounds?.[table.round]
    const match = round?.[table.match]
    if (!match) return entry
    entry.match = match

    // Round label
    if (serie.type === 'single') {
      const totalRounds = serie.rounds?.length || 0
      const fromEnd = totalRounds - table.round
      if (fromEnd === 1) { entry.roundLabel = 'Finale'; entry.isFinale = true }
      else if (fromEnd === 2) entry.roundLabel = '1/2'
      else if (fromEnd === 3) entry.roundLabel = '1/4'
      else if (fromEnd === 4) entry.roundLabel = '1/8'
      else entry.roundLabel = `Tour ${table.round + 1}`
    } else {
      entry.roundLabel = `Tour ${table.round + 1}`
    }

    const playersMap = {}
    for (const p of (serie.players || [])) playersMap[p.license] = p

    entry.player1 = playersMap[match.player1] || null
    entry.player2 = playersMap[match.player2] || null

    if (serie.handicap && entry.player1 && entry.player2) {
      const h = computeHandicap(entry.player1.ranking, entry.player2.ranking, serie.handicapTable)
      entry.handicap1 = h.handicap1
      entry.handicap2 = h.handicap2
    }

    return entry
  })

  // Sort: occupied first, then by table number
  return result.sort((a, b) => {
    if (a.match && !b.match) return -1
    if (!a.match && b.match) return 1
    return a.tableNumber - b.tableNumber
  })
})

function tablePlayersMap(t) {
  const map = {}
  if (t.player1 && t.match) map[t.match.player1] = t.player1
  if (t.player2 && t.match) map[t.match.player2] = t.player2
  return map
}
</script>
