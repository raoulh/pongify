<template>
  <div :class="['match-card', { 'match-card-compact': compact }]" @click.self="onCardClick">
    <template v-if="match.isBye">
      <!-- Player 1 half (the player who has the bye) -->
      <div class="flex">
        <div class="flex-1 min-w-0 px-2.5 py-1.5 bg-[#484848]">
          <template v-if="isDouble && player1?.licenseSecond">
            <div class="flex items-baseline justify-between gap-2">
              <span class="text-white text-sm truncate">{{ player1.firstname }} <b>{{ player1.lastname }}</b></span>
              <span class="match-rank shrink-0">{{ player1.ranking }}</span>
            </div>
            <div class="flex items-baseline justify-between gap-2">
              <span class="text-white text-sm truncate">{{ player1.firstnameSecond }} <b>{{ player1.lastnameSecond }}</b></span>
              <span class="match-rank shrink-0">{{ player1.rankingSecond }}</span>
            </div>
          </template>
          <template v-else-if="player1">
            <div class="flex items-baseline justify-between gap-2">
              <span class="text-white text-sm truncate">{{ player1.firstname }}</span>
              <span class="match-rank shrink-0">{{ player1.ranking }}</span>
            </div>
            <div class="text-white text-sm font-bold truncate">{{ player1.lastname }}</div>
          </template>
        </div>
        <div class="w-10 flex items-center justify-center shrink-0 bg-[#eed67d]" v-if="!hideScore">
          <span class="font-bold text-base text-black">-</span>
        </div>
      </div>
      <div class="h-px bg-pongify-gold"></div>
      <!-- Player 2 half (empty = exempt) -->
      <div class="flex">
        <div class="flex-1 min-w-0 px-2.5 py-1.5 bg-[#b2b2b2]">
          <div class="text-sm py-0.5 italic text-gray-500">Exempt</div>
          <div class="text-sm py-0.5">&nbsp;</div>
        </div>
        <div class="w-10 flex items-center justify-center shrink-0 bg-[#faf1d0]" v-if="!hideScore">
          <span class="font-bold text-base text-[#8b8b8b]">-</span>
        </div>
      </div>
    </template>
    <template v-else>
    <div class="flex">
      <component :is="player1 ? 'router-link' : 'div'"
        :to="player1 ? { name: 'search', query: { q: searchName(player1) } } : undefined"
        class="flex-1 min-w-0 px-2.5 py-1.5 block"
        :class="p1Bg">
        <template v-if="isDouble && player1?.licenseSecond">
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm truncate">{{ player1.firstname }} <b>{{ player1.lastname }}</b></span>
            <span class="match-rank shrink-0">{{ player1.ranking }}</span>
          </div>
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm truncate">{{ player1.firstnameSecond }} <b>{{ player1.lastnameSecond }}</b></span>
            <span class="match-rank shrink-0">{{ player1.rankingSecond }}</span>
          </div>
        </template>
        <template v-else-if="player1">
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm truncate">{{ player1.firstname }}</span>
            <div class="flex items-center gap-1.5 shrink-0">
              <span v-if="isHandicap && h.handicap1 > 0" class="text-xs font-bold" style="color:#84fa78">+{{ h.handicap1 }}</span>
              <span class="match-rank">{{ player1.ranking }}</span>
            </div>
          </div>
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm font-bold truncate">{{ player1.lastname }}</span>
            <span class="text-xs truncate italic" style="color:#cccccc">{{ player1.club }}</span>
          </div>
        </template>
        <template v-else>
          <div class="text-sm py-1" style="color:#aaa">—</div>
          <div class="text-sm py-1" style="color:#aaa">—</div>
        </template>
      </component>
      <div class="w-10 flex items-center justify-center shrink-0 cursor-pointer" :class="s1Bg" v-if="!hideScore" @click.stop="onCardClick">
        <span class="font-bold text-base" :class="s1Text">{{ fmtScore(match.playerScore1) }}</span>
      </div>
    </div>
    <!-- Gold separator -->
    <div class="h-px bg-pongify-gold" @click.stop="onCardClick"></div>
    <!-- Player 2 half -->
    <div class="flex">
      <component :is="player2 ? 'router-link' : 'div'"
        :to="player2 ? { name: 'search', query: { q: searchName(player2) } } : undefined"
        class="flex-1 min-w-0 px-2.5 py-1.5 block"
        :class="p2Bg">
        <template v-if="isDouble && player2?.licenseSecond">
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm truncate">{{ player2.firstname }} <b>{{ player2.lastname }}</b></span>
            <span class="match-rank shrink-0">{{ player2.ranking }}</span>
          </div>
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm truncate">{{ player2.firstnameSecond }} <b>{{ player2.lastnameSecond }}</b></span>
            <span class="match-rank shrink-0">{{ player2.rankingSecond }}</span>
          </div>
        </template>
        <template v-else-if="player2">
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm truncate">{{ player2.firstname }}</span>
            <div class="flex items-center gap-1.5 shrink-0">
              <span v-if="isHandicap && h.handicap2 > 0" class="text-xs font-bold" style="color:#84fa78">+{{ h.handicap2 }}</span>
              <span class="match-rank">{{ player2.ranking }}</span>
            </div>
          </div>
          <div class="flex items-baseline justify-between gap-2">
            <span class="text-white text-sm font-bold truncate">{{ player2.lastname }}</span>
            <span class="text-xs truncate italic" style="color:#cccccc">{{ player2.club }}</span>
          </div>
        </template>
        <template v-else>
          <div class="text-sm py-1" style="color:#aaa">—</div>
          <div class="text-sm py-1" style="color:#aaa">—</div>
        </template>
      </component>
      <div class="w-10 flex items-center justify-center shrink-0 cursor-pointer" :class="s2Bg" v-if="!hideScore" @click.stop="onCardClick">
        <span class="font-bold text-base" :class="s2Text">{{ fmtScore(match.playerScore2) }}</span>
      </div>
    </div>
    </template>
    <!-- Path annotation -->
    <div v-if="showPathAnnotation && !isLastRound" class="match-path-annotation" @click.stop="$emit('navigate', nextRoundIndex, nextMatchIndex)">
      <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" class="w-3.5 h-3.5 shrink-0"><path fill-rule="evenodd" d="M3 10a.75.75 0 01.75-.75h10.638L10.23 5.29a.75.75 0 111.04-1.08l5.5 5.25a.75.75 0 010 1.08l-5.5 5.25a.75.75 0 11-1.04-1.08l4.158-3.96H3.75A.75.75 0 013 10z" clip-rule="evenodd" /></svg>
      <span>{{ nextRoundLabel }} #{{ nextMatchIndex + 1 }}</span>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import PlayerBadge from './PlayerBadge.vue'
import { computeHandicap } from '../utils/handicap.js'

const props = defineProps({
  match: Object,
  players: Object,
  isDouble: Boolean,
  isHandicap: Boolean,
  handicapTable: { type: Array, default: undefined },
  hideScore: { type: Boolean, default: false },
  roundIndex: { type: Number, default: -1 },
  matchIndex: { type: Number, default: -1 },
  totalRounds: { type: Number, default: 0 },
  showPathAnnotation: { type: Boolean, default: false },
  compact: { type: Boolean, default: false }
})

const emit = defineEmits(['select', 'navigate'])

const isLastRound = computed(() => props.roundIndex >= props.totalRounds - 1)
const nextRoundIndex = computed(() => props.roundIndex + 1)
const nextMatchIndex = computed(() => Math.floor(props.matchIndex / 2))

const nextRoundLabel = computed(() => {
  if (props.totalRounds <= 0 || isLastRound.value) return ''
  const fromEnd = props.totalRounds - nextRoundIndex.value
  if (fromEnd === 1) return 'Finale'
  if (fromEnd === 2) return '1/2'
  if (fromEnd === 3) return '1/4'
  if (fromEnd === 4) return '1/8'
  return `Tour ${nextRoundIndex.value + 1}`
})

function onCardClick() {
  if (props.roundIndex >= 0) emit('select', props.roundIndex, props.matchIndex)
}

const player1 = computed(() => props.players?.[props.match?.player1] || null)
const player2 = computed(() => props.players?.[props.match?.player2] || null)

const h = computed(() => {
  if (!props.isHandicap || !player1.value || !player2.value) return { handicap1: 0, handicap2: 0 }
  return computeHandicap(player1.value.ranking, player2.value.ranking, props.handicapTable)
})

const p1Bg = computed(() => (!player1.value || props.match.playerWinner2) ? 'bg-[#b2b2b2]' : 'bg-[#484848]')
const p2Bg = computed(() => (!player2.value || props.match.playerWinner1) ? 'bg-[#b2b2b2]' : 'bg-[#484848]')
const s1Bg = computed(() => (!player1.value || props.match.playerWinner2) ? 'bg-[#faf1d0]' : 'bg-[#eed67d]')
const s2Bg = computed(() => (!player2.value || props.match.playerWinner1) ? 'bg-[#faf1d0]' : 'bg-[#eed67d]')
const s1Text = computed(() => props.match.playerWinner2 ? 'text-[#8b8b8b]' : 'text-black')
const s2Text = computed(() => props.match.playerWinner1 ? 'text-[#8b8b8b]' : 'text-black')

function fmtScore(s) {
  if (s === undefined || s === null || s === '' || s === -1 || s === '-1') return '-'
  return String(s)
}

function searchName(player) {
  if (!player) return ''
  return `${player.firstname} ${player.lastname}`
}
</script>
