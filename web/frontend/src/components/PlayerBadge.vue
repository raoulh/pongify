<template>
  <component :is="clickable && player ? 'router-link' : 'span'"
    v-if="player"
    :to="clickable ? { name: 'search', query: { q: fullName } } : undefined"
    class="text-sm"
    :class="[
      clickable ? 'text-pongify-teal hover:underline cursor-pointer' : '',
      isDouble && player.licenseSecond ? 'inline-block' : 'inline'
    ]">
    <template v-if="isDouble && player.licenseSecond">
      <span class="flex items-baseline gap-1">
        <span>{{ player.firstname }} {{ player.lastname }}</span>
        <span class="text-gray-400 text-xs">({{ player.ranking }})</span>
      </span>
      <span class="flex items-baseline gap-1">
        <span>{{ player.firstnameSecond }} {{ player.lastnameSecond }}</span>
        <span class="text-gray-400 text-xs">({{ player.rankingSecond }})</span>
      </span>
    </template>
    <template v-else>
      {{ player.firstname }} {{ player.lastname }}
      <span class="text-gray-400">({{ player.ranking }})</span>
    </template>
    <span v-if="showClub && player.club" class="text-gray-400 text-xs ml-1">{{ player.club }}</span>
  </component>
  <span v-else class="text-gray-400 text-sm italic">—</span>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
  player: Object,
  isDouble: { type: Boolean, default: false },
  showClub: { type: Boolean, default: false },
  clickable: { type: Boolean, default: false }
})

const fullName = computed(() => {
  if (!props.player) return ''
  return `${props.player.firstname} ${props.player.lastname}`
})
</script>
