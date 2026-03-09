<template>
  <div v-if="serie && serie.status === 'stopped'" class="text-gray-400 text-center py-8">
    Cette série n'a pas encore démarré.
  </div>

  <div v-else-if="serie" class="space-y-4">
    <div class="flex items-center gap-3">
      <StatusBadge :status="serie.status" />
      <h1 class="text-xl font-bold text-gray-800 truncate">{{ serie.name }}</h1>
    </div>

    <!-- Single elimination bracket -->
    <template v-if="serie.type === 'single'">

      <!-- Mobile view mode toggle -->
      <div class="md:hidden flex items-center justify-between">
        <div class="view-toggle">
          <button :class="['view-toggle-btn', { active: mobileViewMode === 'rounds' }]"
                  @click="mobileViewMode = 'rounds'" title="Vue par tour">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" class="w-4 h-4">
              <path fill-rule="evenodd" d="M2 4.75A.75.75 0 012.75 4h14.5a.75.75 0 010 1.5H2.75A.75.75 0 012 4.75zm0 10.5a.75.75 0 01.75-.75h14.5a.75.75 0 010 1.5H2.75a.75.75 0 01-.75-.75zM2 10a.75.75 0 01.75-.75h7.5a.75.75 0 010 1.5h-7.5A.75.75 0 012 10z" clip-rule="evenodd" />
            </svg>
          </button>
          <button :class="['view-toggle-btn', { active: mobileViewMode === 'bracket' }]"
                  @click="mobileViewMode = 'bracket'" title="Vue bracket">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" class="w-4 h-4">
              <path fill-rule="evenodd" d="M4.25 2A2.25 2.25 0 002 4.25v2.5A2.25 2.25 0 004.25 9h2.5A2.25 2.25 0 009 6.75v-2.5A2.25 2.25 0 006.75 2h-2.5zm0 9A2.25 2.25 0 002 13.25v2.5A2.25 2.25 0 004.25 18h2.5A2.25 2.25 0 009 15.75v-2.5A2.25 2.25 0 006.75 11h-2.5zm9-9A2.25 2.25 0 0011 4.25v2.5A2.25 2.25 0 0013.25 9h2.5A2.25 2.25 0 0018 6.75v-2.5A2.25 2.25 0 0015.75 2h-2.5zm0 9A2.25 2.25 0 0011 13.25v2.5A2.25 2.25 0 0013.25 18h2.5A2.25 2.25 0 0018 15.75v-2.5A2.25 2.25 0 0015.75 11h-2.5z" clip-rule="evenodd" />
            </svg>
          </button>
        </div>
        <!-- Compact toggle (only visible in bracket mode) -->
        <div v-if="mobileViewMode === 'bracket'" class="view-toggle">
          <button :class="['view-toggle-btn text-xs px-2', { active: bracketCardSize === 'compact' }]"
                  @click="bracketCardSize = 'compact'">Compact</button>
          <button :class="['view-toggle-btn text-xs px-2', { active: bracketCardSize === 'normal' }]"
                  @click="bracketCardSize = 'normal'">Normal</button>
        </div>
      </div>

      <!-- Mobile: rounds mode (tab navigation) -->
      <div v-if="mobileViewMode === 'rounds'" class="md:hidden">
        <div class="round-tabs">
          <button v-for="(round, i) in serie.rounds" :key="i"
                  :class="['round-tab', { active: activeRound === i, live: liveRound === i && activeRound !== i }]"
                  @click="setActiveRound(i)">
            <span class="live-dot" v-if="liveRound === i"></span>
            {{ roundLabel(i, serie.rounds.length) }}
          </button>
        </div>
        <div class="flex flex-col gap-2 mt-2">
          <MatchCard v-for="(match, j) in serie.rounds[activeRound]" :key="j"
                     :match="match" :players="playersMap" :is-double="serie.double" :is-handicap="serie.handicap" :handicap-table="serie.handicapTable"
                     :round-index="activeRound" :match-index="j" :total-rounds="serie.rounds.length"
                     :show-path-annotation="true"
                     @select="openPathSheet"
                     @navigate="navigateToMatch" />
        </div>
      </div>

      <!-- Mobile: bracket mode (full bracket grid on mobile) -->
      <div v-if="mobileViewMode === 'bracket'" class="md:hidden bracket-scroll">
        <div :class="['bracket-grid bracket-grid-mobile', { 'bracket-grid-compact': bracketCardSize === 'compact' }]">
          <BracketColumn v-for="(round, i) in serie.rounds" :key="i"
                         :round="round" :round-index="i" :total-rounds="serie.rounds.length"
                         :players="playersMap" :is-double="serie.double" :is-handicap="serie.handicap" :handicap-table="serie.handicapTable"
                         :is-live="liveRound === i"
                         :compact="bracketCardSize === 'compact'" />
        </div>
      </div>

      <!-- Desktop: full bracket grid -->
      <div class="hidden md:block">
        <div class="bracket-grid">
          <BracketColumn v-for="(round, i) in serie.rounds" :key="i"
                         :round="round" :round-index="i" :total-rounds="serie.rounds.length"
                         :players="playersMap" :is-double="serie.double" :is-handicap="serie.handicap" :handicap-table="serie.handicapTable"
                         :is-live="liveRound === i" />
        </div>
      </div>
    </template>

    <!-- Round-robin -->
    <template v-else-if="serie.type === 'roundrobin'">
      <RankingTable :serie="serie" />

      <div v-for="(round, ri) in serie.rounds" :key="ri" class="space-y-2">
        <h3 :class="['text-sm font-semibold', liveRound === ri ? 'text-pongify-teal flex items-center gap-1.5' : 'text-gray-500']">
          <span class="live-dot" v-if="liveRound === ri"></span>
          Tour {{ ri + 1 }}
        </h3>
        <MatchCard v-for="(match, j) in round" :key="j"
                   :match="match" :players="playersMap" :is-double="serie.double" :is-handicap="serie.handicap" :handicap-table="serie.handicapTable" />
      </div>
    </template>
  </div>

  <div v-else class="text-gray-400 text-center py-8">Série introuvable</div>

  <!-- Bottom sheet: match path -->
  <MatchPathSheet v-if="serie?.type === 'single'"
                  :visible="selectedMatch !== null"
                  :serie="serie" :players="playersMap"
                  :round-index="selectedMatch?.ri ?? 0" :match-index="selectedMatch?.mi ?? 0"
                  :is-double="serie?.double" :is-handicap="serie?.handicap" :handicap-table="serie?.handicapTable"
                  @close="selectedMatch = null" />
</template>

<script setup>
import { computed, ref, onMounted, inject, watch } from 'vue'
import { useRoute } from 'vue-router'
import StatusBadge from '../components/StatusBadge.vue'
import MatchCard from '../components/MatchCard.vue'
import BracketColumn from '../components/BracketColumn.vue'
import RankingTable from '../components/RankingTable.vue'
import MatchPathSheet from '../components/MatchPathSheet.vue'

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

// Mobile view mode: 'rounds' (tab navigation) or 'bracket' (full bracket grid)
const mobileViewMode = ref('rounds')
const bracketCardSize = ref('compact')

// Bottom sheet state
const selectedMatch = ref(null)

function openPathSheet(ri, mi) {
  selectedMatch.value = { ri, mi }
}

// Navigate to a specific match (from annotation tap)
function navigateToMatch(ri, mi) {
  setActiveRound(ri)
}

// Determine initial round: from query param or most advanced played round
const activeRound = ref(0)
const userSelectedRound = ref(false)

function setActiveRound(i) {
  userSelectedRound.value = true
  activeRound.value = i
}

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

// Identify the round currently being played (has assigned matches with no winner yet)
const liveRound = computed(() => {
  if (!serie.value?.rounds || serie.value.status !== 'playing') return -1
  for (let i = 0; i < serie.value.rounds.length; i++) {
    const hasUnfinished = serie.value.rounds[i].some(m =>
      m.player1 && m.player2 && !m.playerWinner1 && !m.playerWinner2 && !m.isBye
    )
    if (hasUnfinished) return i
  }
  return -1
})

watch(serie, () => {
  if (!userSelectedRound.value) activeRound.value = findBestRound()
}, { immediate: true })
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
