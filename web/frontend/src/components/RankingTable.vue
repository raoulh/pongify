<template>
  <div class="overflow-x-auto">
    <table class="w-full text-sm">
      <thead>
        <tr class="text-gray-500 border-b border-gray-200">
          <th class="py-2 text-left w-8">#</th>
          <th class="py-2 text-left">Joueur</th>
          <th class="py-2 text-center w-10">V</th>
          <th class="py-2 text-center w-10">Pts</th>
          <th class="py-2 text-center w-10">S+</th>
          <th class="py-2 text-center w-10">S-</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="(entry, rank) in rankings" :key="entry.license"
            class="border-b border-gray-100">
          <td class="py-2">
            <span v-if="rank < 3" class="text-lg">{{ medals[rank] }}</span>
            <span v-else class="text-gray-400">{{ rank + 1 }}</span>
          </td>
          <td class="py-2">
            <PlayerBadge :player="playersMap[entry.license]" :is-double="serie.double" :clickable="true" />
          </td>
          <td class="py-2 text-center font-medium">{{ entry.winCount }}</td>
          <td class="py-2 text-center font-bold text-pongify-teal">{{ entry.score }}</td>
          <td class="py-2 text-center text-pongify-green">{{ entry.setWin }}</td>
          <td class="py-2 text-center text-red-400">{{ entry.setLoose }}</td>
        </tr>
      </tbody>
    </table>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import PlayerBadge from './PlayerBadge.vue'

const props = defineProps({ serie: Object })
const medals = ['🥇', '🥈', '🥉']

const playersMap = computed(() => {
  const map = {}
  for (const p of (props.serie?.players || [])) map[p.license] = p
  return map
})

const rankings = computed(() => {
  if (!props.serie?.rounds) return []

  const scores = {}

  // Initialize all players with zero scores
  for (const p of (props.serie.players || [])) {
    scores[p.license] = { license: p.license, score: 0, setWin: 0, setLoose: 0, winCount: 0 }
  }

  // Iterate all rounds and matches
  for (let i = 0; i < props.serie.rounds.length; i++) {
    const round = props.serie.rounds[i]
    for (let j = 0; j < round.length; j++) {
      const m = round[j]
      if (!m.player1 || !m.player2 || m.isBye) continue
      if (!m.playerWinner1 && !m.playerWinner2) continue

      const s1 = scores[m.player1] || { license: m.player1, score: 0, setWin: 0, setLoose: 0, winCount: 0 }
      const s2 = scores[m.player2] || { license: m.player2, score: 0, setWin: 0, setLoose: 0, winCount: 0 }

      if (m.playerWinner1) {
        s1.score += m.playerScore2 === 0 ? 2 : 1
        s1.winCount++
      }
      if (m.playerWinner2) {
        s2.score += m.playerScore1 === 0 ? 2 : 1
        s2.winCount++
      }

      s1.setWin += m.playerScore1
      s1.setLoose += m.playerScore2
      s2.setWin += m.playerScore2
      s2.setLoose += m.playerScore1

      scores[m.player1] = s1
      scores[m.player2] = s2
    }
  }

  const list = Object.values(scores)
  const isFinished = props.serie.status === 'finished'

  list.sort((a, b) => {
    if (a.winCount !== b.winCount) return b.winCount - a.winCount
    if (a.score !== b.score) return b.score - a.score
    if (a.setWin !== b.setWin) return b.setWin - a.setWin
    if (a.setLoose !== b.setLoose) return a.setLoose - b.setLoose

    // Direct confrontation (only when serie is finished)
    if (isFinished) {
      for (const round of props.serie.rounds) {
        for (const m of round) {
          if (m.player1 === a.license && m.player2 === b.license) return m.playerWinner1 ? -1 : 1
          if (m.player1 === b.license && m.player2 === a.license) return m.playerWinner2 ? -1 : 1
        }
      }
    }
    return 0
  })

  return list
})
</script>
