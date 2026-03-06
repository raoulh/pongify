<template>
  <div v-if="serie" class="space-y-4">
    <h1 class="text-xl font-bold text-gray-800">🏆 {{ serie.name }}</h1>

    <div v-if="!serie.podium" class="text-gray-400 text-center py-8">
      Podium non encore validé
    </div>

    <!-- Single elimination podium -->
    <div v-else-if="serie.type === 'single'" class="space-y-2">
      <div v-for="(license, rank) in serie.podium" :key="rank"
           class="flex items-center rounded-lg bg-[#eed67d] px-3 py-3 gap-3">
        <span class="w-8 text-center font-bold text-lg text-[#484848] shrink-0">
          <span v-if="rank < 3">{{ medals[rank] }}</span>
          <span v-else>{{ rank + 1 }}</span>
        </span>
        <component :is="playersMap[license] ? 'router-link' : 'span'"
          :to="playersMap[license] ? { name: 'search', query: { q: playerName(playersMap[license]) } } : undefined"
          class="flex-1 font-semibold text-[#484848] truncate min-w-0">
          {{ playerDisplayName(playersMap[license], serie.double) }}
        </component>
        <span class="text-[#484848] font-bold shrink-0">{{ playerRanking(playersMap[license]) }}</span>
      </div>
    </div>

    <!-- Round-robin podium -->
    <div v-else class="overflow-x-auto">
      <table class="w-full text-sm">
        <thead>
          <tr class="text-gray-500 border-b border-gray-200">
            <th class="py-2 text-left">#</th>
            <th class="py-2 text-left">Joueur</th>
            <th class="py-2 text-center">V</th>
            <th class="py-2 text-center">Pts</th>
            <th class="py-2 text-center">S+</th>
            <th class="py-2 text-center">S-</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="(entry, rank) in serie.podium" :key="rank"
              class="border-b border-gray-100" :class="{ 'text-pongify-gold': rank < 3 }">
            <td class="py-2">
              <span v-if="rank < 3" class="text-lg">{{ medals[rank] }}</span>
              <span v-else class="text-gray-400">{{ rank + 1 }}</span>
            </td>
            <td class="py-2">
              <PlayerBadge :player="playersMap[entry.player]" :is-double="serie.double" :show-club="true" :clickable="true" />
            </td>
            <td class="py-2 text-center">{{ entry.winCount }}</td>
            <td class="py-2 text-center font-bold">{{ entry.score }}</td>
            <td class="py-2 text-center text-pongify-green">{{ entry.setWin }}</td>
            <td class="py-2 text-center text-red-400">{{ entry.setLoose }}</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>

  <div v-else class="text-gray-400 text-center py-8">Série introuvable</div>
</template>

<script setup>
import { computed, onMounted, inject } from 'vue'
import PlayerBadge from '../components/PlayerBadge.vue'

const props = defineProps({ tournament: Object, index: [String, Number] })
const { setPollContext } = inject('tournamentCtx')
onMounted(() => setPollContext('finished'))

const medals = ['🥇', '🥈', '🥉']

const serie = computed(() => {
  const idx = parseInt(props.index, 10)
  return props.tournament?.series?.[idx] || null
})

const playersMap = computed(() => {
  const map = {}
  if (serie.value?.players) {
    for (const p of serie.value.players) map[p.license] = p
  }
  return map
})

function playerName(p) {
  if (!p) return ''
  return `${p.firstname} ${p.lastname}`
}

function playerDisplayName(p, isDouble) {
  if (!p) return '—'
  if (isDouble && p.licenseSecond) {
    return `${p.firstname} ${p.lastname} / ${p.firstnameSecond} ${p.lastnameSecond}`
  }
  return `${p.firstname} ${p.lastname}`
}

function playerRanking(p) {
  if (!p) return ''
  return p.ranking || ''
}
</script>
