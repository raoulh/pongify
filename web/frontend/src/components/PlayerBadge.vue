<template>
  <span v-if="player && isDouble && player.licenseSecond" class="inline-block text-sm">
    <span class="flex items-baseline gap-1">
      <component :is="clickable ? 'router-link' : 'span'"
        :to="clickable ? { name: 'search', query: { q: fullName } } : undefined"
        :class="clickable ? 'text-pongify-teal hover:underline cursor-pointer' : ''">
        {{ player.firstname }} {{ player.lastname }}
      </component>
      <span class="text-gray-400 text-xs">({{ player.ranking }})</span>
    </span>
    <span class="flex items-baseline gap-1">
      <component :is="clickable ? 'router-link' : 'span'"
        :to="clickable ? { name: 'search', query: { q: fullNameSecond } } : undefined"
        :class="clickable ? 'text-pongify-teal hover:underline cursor-pointer' : ''">
        {{ player.firstnameSecond }} {{ player.lastnameSecond }}
      </component>
      <span class="text-gray-400 text-xs">({{ player.rankingSecond }})</span>
    </span>
    <span v-if="showClub && player.club" class="text-gray-400 text-xs ml-1">{{ player.club }}</span>
  </span>
  <component v-else-if="player"
    :is="clickable ? 'router-link' : 'span'"
    :to="clickable ? { name: 'search', query: { q: fullName } } : undefined"
    class="text-sm inline"
    :class="clickable ? 'text-pongify-teal hover:underline cursor-pointer' : ''">
    {{ player.firstname }} {{ player.lastname }}
    <span class="text-gray-400">({{ player.ranking }})</span>
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

const fullNameSecond = computed(() => {
  if (!props.player) return ''
  return `${props.player.firstnameSecond} ${props.player.lastnameSecond}`
})
</script>
