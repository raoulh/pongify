<template>
  <div v-if="serie" class="space-y-4">
    <div class="flex items-center gap-3">
      <StatusBadge :status="serie.status" />
      <h1 class="text-xl font-bold text-gray-800 truncate">{{ serie.name }}</h1>
    </div>

    <!-- Single elimination bracket -->
    <template v-if="serie.type === 'single'">
      <!-- Mobile: round tabs -->
      <div class="md:hidden">
        <div class="round-tabs">
          <button v-for="(round, i) in serie.rounds" :key="i"
                  :class="['round-tab', { active: activeRound === i }]"
                  @click="activeRound = i">
            {{ roundLabel(i, serie.rounds.length) }}
          </button>
        </div>
        <div class="flex flex-col gap-2 mt-2">
          <MatchCard v-for="(match, j) in serie.rounds[activeRound]" :key="j"
                     :match="match" :players="playersMap" :is-double="serie.double" :is-handicap="serie.handicap" />
        </div>
      </div>

      <!-- Desktop: full bracket grid -->
      <div class="hidden md:block">
        <div class="bracket-grid">
          <BracketColumn v-for="(round, i) in serie.rounds" :key="i"
                         :round="round" :round-index="i" :total-rounds="serie.rounds.length"
                         :players="playersMap" :is-double="serie.double" :is-handicap="serie.handicap" />
        </div>
      </div>
    </template>

    <!-- Round-robin -->
    <template v-else-if="serie.type === 'roundrobin'">
      <RankingTable :serie="serie" />

      <div v-for="(round, ri) in serie.rounds" :key="ri" class="space-y-2">
        <h3 class="text-sm font-semibold text-gray-500">Tour {{ ri + 1 }}</h3>
        <MatchCard v-for="(match, j) in round" :key="j"
                   :match="match" :players="playersMap" :is-double="serie.double" :is-handicap="serie.handicap" />
      </div>
    </template>
  </div>

  <div v-else class="text-gray-400 text-center py-8">Série introuvable</div>
</template>

<script setup>
import { computed, ref, onMounted, inject, watch } from 'vue'
import { useRoute } from 'vue-router'
import StatusBadge from '../components/StatusBadge.vue'
import MatchCard from '../components/MatchCard.vue'
import BracketColumn from '../components/BracketColumn.vue'
import RankingTable from '../components/RankingTable.vue'

const props = defineProps({ tournament: Object, index: [String, Number] })
const route = useRoute()
const { setPollContext } = inject('tournamentCtx')

const serie = computed(() => {
  const idx = parseInt(props.index, 10)
  return props.tournament?.series?.[idx] || null
})

const playersMap = computed(() => {
  const map = {}
  if (serie.value?.players) {
    for (const p of serie.value.players) {
      map[p.license] = p
    }
  }
  return map
})

// Determine initial round: from query param or most advanced played round
const activeRound = ref(0)

function findBestRound() {
  const qr = parseInt(route.query.round, 10)
  if (!isNaN(qr) && serie.value?.rounds?.[qr]) return qr
  if (!serie.value?.rounds) return 0
  let best = 0
  for (let i = 0; i < serie.value.rounds.length; i++) {
    for (const m of serie.value.rounds[i]) {
      if (m.playerWinner1 || m.playerWinner2) { best = i; break }
    }
  }
  return best
}

watch(serie, () => { activeRound.value = findBestRound() }, { immediate: true })
onMounted(() => {
  if (serie.value) setPollContext(serie.value.status)
})
watch(() => serie.value?.status, (s) => { if (s) setPollContext(s) })

function roundLabel(index, total) {
  const fromEnd = total - index
  if (fromEnd === 1) return 'Finale'
  if (fromEnd === 2) return '1/2'
  if (fromEnd === 3) return '1/4'
  if (fromEnd === 4) return '1/8'
  return `Tour ${index + 1}`
}
</script>
