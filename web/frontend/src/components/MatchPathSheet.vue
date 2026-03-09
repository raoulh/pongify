<template>
  <Teleport to="body">
    <Transition name="sheet">
      <div v-if="visible" class="sheet-backdrop" @click.self="$emit('close')">
        <div class="sheet-panel" ref="panelRef">
          <!-- Header -->
          <div class="flex items-center justify-between px-4 py-3 border-b border-gray-200">
            <h2 class="text-base font-bold text-gray-800">Parcours du match</h2>
            <button class="p-1 rounded-full hover:bg-gray-100 text-gray-500" @click="$emit('close')">
              <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" class="w-5 h-5">
                <path d="M6.28 5.22a.75.75 0 00-1.06 1.06L8.94 10l-3.72 3.72a.75.75 0 101.06 1.06L10 11.06l3.72 3.72a.75.75 0 101.06-1.06L11.06 10l3.72-3.72a.75.75 0 00-1.06-1.06L10 8.94 6.28 5.22z" />
              </svg>
            </button>
          </div>

          <!-- Path content -->
          <div class="sheet-content" ref="contentRef">
            <div class="path-timeline">
              <template v-for="(step, idx) in path" :key="idx">
                <!-- Round label -->
                <div class="path-round-label">
                  <span :class="step.isSelected ? 'text-pongify-teal font-bold' : 'text-gray-400'">
                    {{ step.label }}
                  </span>
                </div>

                <!-- Match card -->
                <div :class="['path-match', { 'path-match-selected': step.isSelected }]"
                     :ref="el => { if (step.isSelected) selectedEl = el }">
                  <MatchCard :match="step.match" :players="players"
                             :is-double="isDouble" :is-handicap="isHandicap" :handicap-table="handicapTable" />
                </div>

                <!-- Connector line -->
                <div v-if="idx < path.length - 1" class="path-connector">
                  <div class="path-line"></div>
                  <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" class="path-arrow">
                    <path fill-rule="evenodd" d="M10 3a.75.75 0 01.75.75v10.638l3.96-4.158a.75.75 0 111.08 1.04l-5.25 5.5a.75.75 0 01-1.08 0l-5.25-5.5a.75.75 0 111.08-1.04l3.96 4.158V3.75A.75.75 0 0110 3z" clip-rule="evenodd" />
                  </svg>
                </div>
              </template>
            </div>
          </div>
        </div>
      </div>
    </Transition>
  </Teleport>
</template>

<script setup>
import { computed, ref, nextTick, watch } from 'vue'
import MatchCard from './MatchCard.vue'

const props = defineProps({
  visible: { type: Boolean, default: false },
  serie: Object,
  players: Object,
  roundIndex: { type: Number, required: true },
  matchIndex: { type: Number, required: true },
  isDouble: Boolean,
  isHandicap: Boolean,
  handicapTable: { type: Array, default: undefined }
})

defineEmits(['close'])

const panelRef = ref(null)
const contentRef = ref(null)
const selectedEl = ref(null)

function roundLabel(ri, total) {
  const fromEnd = total - ri
  if (fromEnd === 1) return 'Finale'
  if (fromEnd === 2) return '1/2 finale'
  if (fromEnd === 3) return '1/4 de finale'
  if (fromEnd === 4) return '1/8 de finale'
  return `Tour ${ri + 1}`
}

const path = computed(() => {
  if (!props.serie?.rounds) return []
  const rounds = props.serie.rounds
  const total = rounds.length
  const steps = []

  // Trace upward (ancestors) — from first round to selected
  const ancestors = []
  let ri = props.roundIndex
  let mi = props.matchIndex
  while (ri > 0) {
    ri--
    const srcA = mi * 2
    const srcB = mi * 2 + 1
    // Add both source matches as a branching point, but follow the one leading to selected
    // We trace the direct lineage: just go up
    // Actually for the "path" view we show the direct chain, not the tree.
    // We pick the source match that leads to our position
    // Both srcA and srcB feed into mi, so we show both at each level
    // Let's show only the single chain going up — the match that feeds into player slot
    // Since we can't know which source fed the winner, show both sources at each ancestor level
    ancestors.unshift({ ri, srcA, srcB })
    mi = srcA // trace upward through first source (we'll show both)
  }

  // Build steps: ancestors first
  for (const anc of ancestors) {
    const matchA = rounds[anc.ri]?.[anc.srcA]
    const matchB = rounds[anc.ri]?.[anc.srcB]
    if (matchA) {
      steps.push({
        roundIndex: anc.ri,
        matchIndex: anc.srcA,
        match: matchA,
        label: `${roundLabel(anc.ri, total)} — Match #${anc.srcA + 1}`,
        isSelected: false,
        isBranch: true,
        branchSide: 'A'
      })
    }
    if (matchB && anc.srcB < (rounds[anc.ri]?.length || 0)) {
      steps.push({
        roundIndex: anc.ri,
        matchIndex: anc.srcB,
        match: matchB,
        label: `${roundLabel(anc.ri, total)} — Match #${anc.srcB + 1}`,
        isSelected: false,
        isBranch: true,
        branchSide: 'B'
      })
    }
  }

  // Selected match
  const selMatch = rounds[props.roundIndex]?.[props.matchIndex]
  if (selMatch) {
    steps.push({
      roundIndex: props.roundIndex,
      matchIndex: props.matchIndex,
      match: selMatch,
      label: `${roundLabel(props.roundIndex, total)} — Match #${props.matchIndex + 1}`,
      isSelected: true
    })
  }

  // Trace downward (descendants) to finale
  ri = props.roundIndex
  mi = props.matchIndex
  while (ri < total - 1) {
    const nextMi = Math.floor(mi / 2)
    ri++
    mi = nextMi
    const m = rounds[ri]?.[mi]
    if (m) {
      steps.push({
        roundIndex: ri,
        matchIndex: mi,
        match: m,
        label: `${roundLabel(ri, total)} — Match #${mi + 1}`,
        isSelected: false
      })
    }
  }

  return steps
})

// Scroll selected match into view when sheet opens
watch(() => props.visible, async (v) => {
  if (v) {
    await nextTick()
    await nextTick()
    if (selectedEl.value) {
      selectedEl.value.scrollIntoView({ behavior: 'smooth', block: 'center' })
    }
  }
})
</script>
