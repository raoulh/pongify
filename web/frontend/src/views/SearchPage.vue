<template>
  <div class="space-y-4">
    <h1 class="text-xl font-bold text-gray-800">🔍 Recherche</h1>

    <input v-model="query" type="text" placeholder="Nom du joueur..."
           class="w-full bg-white border border-gray-200 rounded-lg px-4 py-2.5 text-gray-800 placeholder-gray-400 focus:outline-none focus:ring-1 focus:ring-pongify-teal">

    <!-- Top 5 players when search is empty -->
    <div v-if="query.length < 2 && topPlayers.length" class="space-y-3">
      <div class="text-xs font-semibold text-gray-500 uppercase tracking-wide">🏅 Top 5 joueurs</div>
      <div v-for="(entry, idx) in topPlayers" :key="entry.license"
           class="bg-white rounded-lg px-4 py-3 shadow-sm border border-gray-100 cursor-pointer hover:border-pongify-teal/40 transition-colors"
           @click="searchPlayer(entry.player)">
        <div class="flex items-center gap-3">
          <span class="text-lg font-bold w-6 text-center" :class="idx === 0 ? 'text-yellow-500' : idx === 1 ? 'text-gray-400' : idx === 2 ? 'text-amber-600' : 'text-gray-300'">{{ idx + 1 }}</span>
          <div class="flex-1 min-w-0">
            <div class="font-semibold text-pongify-teal text-sm truncate">{{ entry.player.firstname }} {{ entry.player.lastname }}</div>
            <div class="text-xs text-gray-400">{{ entry.player.club }}</div>
          </div>
          <div class="text-right text-xs">
            <span class="font-bold" :class="winRate(entry.stats) >= 50 ? 'text-pongify-green' : 'text-red-400'">{{ winRate(entry.stats) }}%</span>
            <div class="text-gray-400">{{ entry.stats.wins }}V {{ entry.stats.losses }}D</div>
          </div>
        </div>
      </div>
    </div>

    <div v-if="query.length >= 2 && !filteredPlayers.length" class="text-gray-400 text-center py-4">
      Aucun joueur trouvé
    </div>

    <div v-for="entry in filteredPlayers" :key="entry.license" class="bg-white rounded-lg p-4 space-y-3 shadow-sm border border-gray-100">
      <div class="font-semibold text-pongify-teal text-base">
        <PlayerBadge :player="entry.player" :is-double="false" :show-club="true" />
      </div>

      <!-- Stats summary -->
      <div class="flex items-center gap-3 text-xs bg-gray-50 rounded-lg px-3 py-2">
        <div class="flex items-center gap-1">
          <span class="font-bold text-pongify-green text-sm">{{ entry.stats.wins }}</span>
          <span class="text-gray-500">V</span>
        </div>
        <div class="flex items-center gap-1">
          <span class="font-bold text-red-400 text-sm">{{ entry.stats.losses }}</span>
          <span class="text-gray-500">D</span>
        </div>
        <span class="text-gray-300">|</span>
        <div class="flex items-center gap-1">
          <span class="text-pongify-green font-semibold">{{ entry.stats.setsWon }}</span>
          <span class="text-gray-400">/</span>
          <span class="text-red-400 font-semibold">{{ entry.stats.setsLost }}</span>
          <span class="text-gray-500">sets</span>
        </div>
        <span class="text-gray-300">|</span>
        <span class="font-bold" :class="winRate(entry.stats) >= 50 ? 'text-pongify-green' : 'text-red-400'">
          {{ winRate(entry.stats) }}%
        </span>
      </div>

      <!-- Upcoming matches -->
      <div v-if="entry.upcoming.length">
        <div class="text-xs font-semibold text-gray-500 mb-2 uppercase tracking-wide">Prochains matchs</div>
        <div v-for="(um, i) in entry.upcoming" :key="'u'+i"
             class="text-sm text-gray-600 py-2 border-l-2 border-pongify-teal pl-3 mb-1">
          <div class="flex items-center gap-2 flex-wrap">
            <span class="text-pongify-teal font-medium">{{ um.serieName }}</span>
            <template v-if="um.partner">
              <span class="text-gray-400">avec</span>
              <PlayerBadge :player="um.partner" :is-double="false" :clickable="true" />
            </template>
          </div>
          <div class="flex items-center gap-2 mt-0.5 flex-wrap">
            <span class="text-gray-400">vs</span>
            <PlayerBadge :player="um.opponent" :is-double="false" :clickable="true" />
            <template v-if="um.opponent2">
              <span class="text-gray-400">/</span>
              <PlayerBadge :player="um.opponent2" :is-double="false" :clickable="true" />
            </template>
            <span v-if="um.tableNumber" class="text-xs text-white bg-pongify-teal rounded px-1.5 py-0.5 ml-auto">Table {{ um.tableNumber }}</span>
          </div>
        </div>
      </div>

      <!-- Results -->
      <div v-if="entry.results.length">
        <div class="text-xs font-semibold text-gray-500 mb-2 uppercase tracking-wide">Résultats</div>
        <div v-for="(r, i) in entry.results" :key="'r'+i"
             class="text-sm py-2 border-l-2 pl-3 mb-1"
             :class="r.won ? 'border-green-400 bg-green-50/50' : 'border-red-300 bg-red-50/50'"
             style="border-radius: 0 6px 6px 0;">
          <div class="flex items-center gap-2 flex-wrap">
            <span class="font-bold text-xs px-1.5 py-0.5 rounded" :class="r.won ? 'bg-green-100 text-green-700' : 'bg-red-100 text-red-500'">{{ r.won ? 'V' : 'D' }}</span>
            <span class="text-gray-500 text-xs">{{ r.serieName }}</span>
            <template v-if="r.partner">
              <span class="text-gray-400 text-xs">avec</span>
              <PlayerBadge :player="r.partner" :is-double="false" :clickable="true" />
            </template>
            <span class="font-mono font-bold ml-auto" :class="r.won ? 'text-green-600' : 'text-red-400'">{{ r.score }}</span>
          </div>
          <div class="flex items-center gap-2 mt-0.5 flex-wrap">
            <span class="text-gray-400 text-xs">vs</span>
            <PlayerBadge :player="r.opponent" :is-double="false" :clickable="true" />
            <template v-if="r.opponent2">
              <span class="text-gray-400">/</span>
              <PlayerBadge :player="r.opponent2" :is-double="false" :clickable="true" />
            </template>
          </div>
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
import { ref, computed, onMounted, inject, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import PlayerBadge from '../components/PlayerBadge.vue'

const props = defineProps({ tournament: Object })
const { setPollContext } = inject('tournamentCtx')
const route = useRoute()
const router = useRouter()

onMounted(() => setPollContext('home'))

const query = ref(route.query.q || '')

watch(() => route.query.q, (newQ) => {
  query.value = newQ || ''
})
const medals = ['🥇', '🥈', '🥉']

function normalize(str) {
  return (str || '').normalize('NFD').replace(/[\u0300-\u036f]/g, '').toLowerCase()
}

function winRate(stats) {
  const total = stats.wins + stats.losses
  if (total === 0) return 0
  return Math.round(stats.wins / total * 100)
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

      // For doubles, also create an entry for the second player
      let secondEntry = null
      if (serie.double && player.licenseSecond) {
        if (!map.has(player.licenseSecond)) {
          map.set(player.licenseSecond, {
            license: player.licenseSecond,
            player: {
              firstname: player.firstnameSecond,
              lastname: player.lastnameSecond,
              license: player.licenseSecond,
              ranking: player.rankingSecond,
              club: player.clubSecond || player.club
            },
            stats: { wins: 0, losses: 0, setsWon: 0, setsLost: 0 },
            upcoming: [],
            results: [],
            podiums: []
          })
        }
        secondEntry = map.get(player.licenseSecond)
      }

      // Scan matches
      for (let ri = 0; ri < (serie.rounds || []).length; ri++) {
        for (let mi = 0; mi < serie.rounds[ri].length; mi++) {
          const m = serie.rounds[ri][mi]
          if (m.isBye) continue
          const isP1 = m.player1 === player.license
          const isP2 = m.player2 === player.license
          if (!isP1 && !isP2) continue

          const opponentLicense = isP1 ? m.player2 : m.player1
          const opponentPair = playersMap[opponentLicense] || null
          const won = isP1 ? m.playerWinner1 : m.playerWinner2
          const lost = isP1 ? m.playerWinner2 : m.playerWinner1
          const myScore = isP1 ? m.playerScore1 : m.playerScore2
          const oppScore = isP1 ? m.playerScore2 : m.playerScore1

          // For doubles, resolve opponent2 and partner
          let opponent = opponentPair
          let opponent2 = null
          let partner = null
          if (serie.double && opponentPair && opponentPair.licenseSecond) {
            opponent = { firstname: opponentPair.firstname, lastname: opponentPair.lastname, license: opponentPair.license, ranking: opponentPair.ranking, club: opponentPair.club }
            opponent2 = { firstname: opponentPair.firstnameSecond, lastname: opponentPair.lastnameSecond, license: opponentPair.licenseSecond, ranking: opponentPair.rankingSecond, club: opponentPair.clubSecond || opponentPair.club }
          }
          if (serie.double && player.licenseSecond) {
            partner = { firstname: player.firstnameSecond, lastname: player.lastnameSecond, license: player.licenseSecond, ranking: player.rankingSecond, club: player.clubSecond || player.club }
          }
          // For secondEntry, the partner is the first player
          let partnerForSecond = null
          if (secondEntry) {
            partnerForSecond = { firstname: player.firstname, lastname: player.lastname, license: player.license, ranking: player.ranking, club: player.club }
          }

          if (won || lost) {
            entry.stats.setsWon += myScore
            entry.stats.setsLost += oppScore
            if (won) entry.stats.wins++
            else entry.stats.losses++

            entry.results.push({
              serieName: serie.name,
              opponent,
              opponent2,
              partner,
              won: !!won,
              score: `${myScore}-${oppScore}`
            })
            if (secondEntry) {
              secondEntry.stats.setsWon += myScore
              secondEntry.stats.setsLost += oppScore
              if (won) secondEntry.stats.wins++
              else secondEntry.stats.losses++
              secondEntry.results.push({
                serieName: serie.name,
                opponent,
                opponent2,
                partner: partnerForSecond,
                won: !!won,
                score: `${myScore}-${oppScore}`
              })
            }
          } else if (opponent) {
            const tableNum = tablesLookup[`${serie.uid}:${ri}:${mi}`] || null
            entry.upcoming.push({
              serieName: serie.name,
              opponent,
              opponent2,
              partner,
              tableNumber: tableNum
            })
            if (secondEntry) {
              secondEntry.upcoming.push({
                serieName: serie.name,
                opponent,
                opponent2,
                partner: partnerForSecond,
                tableNumber: tableNum
              })
            }
          }
        }
      }

      // Podium
      if (serie.status === 'finished' && serie.podium) {
        if (serie.type === 'single') {
          const rank = serie.podium.indexOf(player.license)
          if (rank >= 0) {
            const podium = {
              serieName: serie.name,
              rank,
              medal: rank < 3 ? medals[rank] : '',
              rankLabel: rank < 3 ? ['1er', '2ème', '3ème'][rank] : `${rank + 1}ème`
            }
            entry.podiums.push(podium)
            if (secondEntry) secondEntry.podiums.push(podium)
          }
        } else {
          const podEntry = serie.podium.find(e => e.player === player.license)
          if (podEntry) {
            const rank = serie.podium.indexOf(podEntry)
            const podium = {
              serieName: serie.name,
              rank,
              medal: rank < 3 ? medals[rank] : '',
              rankLabel: rank < 3 ? ['1er', '2ème', '3ème'][rank] : `${rank + 1}ème`
            }
            entry.podiums.push(podium)
            if (secondEntry) secondEntry.podiums.push(podium)
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

const topPlayers = computed(() => {
  return allPlayersData.value
    .filter(e => (e.stats.wins + e.stats.losses) > 0)
    .sort((a, b) => {
      if (b.stats.wins !== a.stats.wins) return b.stats.wins - a.stats.wins
      return winRate(b.stats) - winRate(a.stats)
    })
    .slice(0, 5)
})

function searchPlayer(player) {
  const name = `${player.lastname}`
  query.value = name
  router.replace({ path: '/search', query: { q: name } })
}
</script>
