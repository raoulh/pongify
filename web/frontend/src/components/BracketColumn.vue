<template>
  <div class="bracket-col-wrapper">
    <div :class="['text-xs font-semibold text-center mb-1', isLive ? 'text-pongify-teal flex items-center justify-center gap-1.5' : 'text-gray-400']">
      <span class="live-dot" v-if="isLive"></span>
      {{ label }}
    </div>
    <div class="bracket-column">
      <MatchCard v-for="(match, j) in round" :key="j"
                 :match="match" :players="players" :is-double="isDouble" :is-handicap="isHandicap" :handicap-table="handicapTable" />
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import MatchCard from './MatchCard.vue'

const props = defineProps({
  round: Array,
  roundIndex: Number,
  totalRounds: Number,
  players: Object,
  isDouble: Boolean,
  isHandicap: Boolean,
  handicapTable: { type: Array, default: undefined },
  isLive: { type: Boolean, default: false }
})

const label = computed(() => {
  const fromEnd = props.totalRounds - props.roundIndex
  if (fromEnd === 1) return 'Finale'
  if (fromEnd === 2) return '1/2'
  if (fromEnd === 3) return '1/4'
  if (fromEnd === 4) return '1/8'
  return `Tour ${props.roundIndex + 1}`
})
</script>
