<template>
  <div class="space-y-4">
    <h1 class="text-xl font-bold text-gray-800">🔍 Recherche</h1>

    <input v-model="query" type="text" placeholder="Nom du joueur..."
           class="w-full bg-white border border-gray-200 rounded-lg px-4 py-2.5 text-gray-800 placeholder-gray-400 focus:outline-none focus:ring-1 focus:ring-pongify-teal">

    <div v-if="query.length >= 2 && !filteredPlayers.length" class="text-gray-400 text-center py-4">
      Aucun joueur trouvé
    </div>

    <div v-for="entry in filteredPlayers" :key="entry.license" class="bg-white rounded-lg p-4 space-y-3 shadow-sm border border-gray-100">
      <div class="font-semibold text-pongify-teal">
        <PlayerBadge :player="entry.player" :is-double="false" :show-club="true" />
      </div>

      <!-- Stats summary -->
      <div class="flex gap-4 text-xs text-gray-500">
        <span class="text-pongify-green">{{ entry.stats.wins }}V</span>
        <span class="text-red-400">{{ entry.stats.losses }}D</span>
        <span>Sets {{ entry.stats.setsWon }}-{{ entry.stats.setsLost }}</span>
      </div>

      <!-- Upcoming matches -->
      <div v-if="entry.upcoming.length">
        <div class="text-xs font-semibold text-gray-500 mb-1">Prochains matchs</div>
        <div v-for="(um, i) in entry.upcoming" :key="'u'+i" class="text-sm text-gray-600 flex items-center gap-2">
          <span class="text-pongify-teal">{{ um.serieName }}</span>
          <span>vs</span>
          <PlayerBadge :player="um.opponent" :is-double="false" />
          <span v-if="um.tableNumber" class="text-xs text-gray-500 ml-auto">Table {{ um.tableNumber }}</span>
        </div>
      </div>

      <!-- Results -->
      <div v-if="entry.results.length">
        <div class="text-xs font-semibold text-gray-500 mb-1">Résultats</div>
        <div v-for="(r, i) in entry.results" :key="'r'+i"
             class="text-sm flex items-center gap-2 py-0.5"
             :class="r.won ? 'text-pongify-green' : 'text-red-400'">
          <span class="w-4">{{ r.won ? 'V' : 'D' }}</span>
          <span class="text-gray-500 text-xs">{{ r.serieName }}</span>
          <span class="text-gray-600">vs</span>
          <PlayerBadge :player="r.opponent" :is-double="false" />
          <span class="score ml-auto text-gray-500">{{ r.score }}</span>
        </div>
      </div>

      <!-- Podiums -->
      <div v-if="entry.podiums.length">
        <div class="text-xs font-semibold text-gray-500 mb-1">Podiums</div>
        <div v-for="(p, i) in entry.podiums" :key="'p'+i" class="text-sm text-gray-700">
          <span>{{ p.medal }} {{ p.serieName }}</span>
          <span class="text-gray-500"> — {{ p.rankLabel }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, inject } from 'vue'
import { useRoute } from 'vue-router'
import PlayerBadge from '../components/PlayerBadge.vue'

const props = defineProps({ tournament: Object })
const { setPollContext } = inject('tournamentCtx')
const route = useRoute()

onMounted(() => setPollContext('home'))

const query = ref(route.query.q || '')
const medals = ['🥇', '🥈', '🥉']

function normalize(str) {
  return (str || '').normalize('NFD').replace(/[\u0300-\u036f]/g, '').toLowerCase()
}

// Build a map of all unique players across all series
const allPlayersData = computed(() => {
  if (!props.tournament?.series) return []
  const map = new Map()

  // Build tables lookup: serieUid+round+match -> tableNumber
  const tablesLookup = {}
  for (const t of (props.tournament.tables || [])) {
    if (t.serieUid) {
      tablesLookup[`${t.serieUid}:${t.round}:${t.match}`] = t.tableNumber
    }
  }

  for (let si = 0; si < props.tournament.series.length; si++) {
    const serie = props.tournament.series[si]
    const playersMap = {}
    for (const p of (serie.players || [])) playersMap[p.license] = p

    for (const player of (serie.players || [])) {
      if (!map.has(player.license)) {
        map.set(player.license, {
          license: player.license,
          player,
          stats: { wins: 0, losses: 0, setsWon: 0, setsLost: 0 },
          upcoming: [],
          results: [],
          podiums: []
        })
      }
      const entry = map.get(player.license)

      // Scan matches
      for (let ri = 0; ri < (serie.rounds || []).length; ri++) {
        for (let mi = 0; mi < serie.rounds[ri].length; mi++) {
          const m = serie.rounds[ri][mi]
          if (m.isBye) continue
          const isP1 = m.player1 === player.license
          const isP2 = m.player2 === player.license
          if (!isP1 && !isP2) continue

          const opponentLicense = isP1 ? m.player2 : m.player1
          const opponent = playersMap[opponentLicense] || null
          const won = isP1 ? m.playerWinner1 : m.playerWinner2
          const lost = isP1 ? m.playerWinner2 : m.playerWinner1
          const myScore = isP1 ? m.playerScore1 : m.playerScore2
          const oppScore = isP1 ? m.playerScore2 : m.playerScore1

          if (won || lost) {
            entry.stats.setsWon += myScore
            entry.stats.setsLost += oppScore
            if (won) entry.stats.wins++
            else entry.stats.losses++

            entry.results.push({
              serieName: serie.name,
              opponent,
              won: !!won,
              score: `${myScore}-${oppScore}`
            })
          } else if (opponent) {
            const tableNum = tablesLookup[`${serie.uid}:${ri}:${mi}`] || null
            entry.upcoming.push({
              serieName: serie.name,
              opponent,
              tableNumber: tableNum
            })
          }
        }
      }

      // Podium
      if (serie.status === 'finished' && serie.podium) {
        if (serie.type === 'single') {
          const rank = serie.podium.indexOf(player.license)
          if (rank >= 0) {
            entry.podiums.push({
              serieName: serie.name,
              rank,
              medal: rank < 3 ? medals[rank] : '',
              rankLabel: rank < 3 ? ['1er', '2ème', '3ème'][rank] : `${rank + 1}ème`
            })
          }
        } else {
          const podEntry = serie.podium.find(e => e.player === player.license)
          if (podEntry) {
            const rank = serie.podium.indexOf(podEntry)
            entry.podiums.push({
              serieName: serie.name,
              rank,
              medal: rank < 3 ? medals[rank] : '',
              rankLabel: rank < 3 ? ['1er', '2ème', '3ème'][rank] : `${rank + 1}ème`
            })
          }
        }
      }
    }
  }

  return Array.from(map.values())
})

const filteredPlayers = computed(() => {
  if (query.value.length < 2) return []
  const q = normalize(query.value)
  return allPlayersData.value.filter(entry => {
    const p = entry.player
    const searchStr = normalize(`${p.firstname} ${p.lastname} ${p.firstnameSecond || ''} ${p.lastnameSecond || ''}`)
    return searchStr.includes(q)
  })
})
</script>
