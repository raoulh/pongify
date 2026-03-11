<template>
  <div v-if="sourceNames.length" class="flex flex-wrap gap-1 mt-1">
    <span class="text-xs text-pongify-teal">
      Qualifiés depuis
    </span>
    <span v-for="name in sourceNames" :key="name"
          class="inline-flex items-center text-xs px-1.5 py-0.5 rounded-full bg-pongify-teal/10 text-pongify-teal font-medium">
      {{ name }}
    </span>
  </div>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
  feedFrom: { type: Array, default: () => [] },
  allSeries: { type: Array, default: () => [] }
})

const sourceNames = computed(() => {
  if (!props.feedFrom || !props.feedFrom.length) return []
  return props.feedFrom
    .map(uid => {
      const s = props.allSeries.find(s => s.uid === uid)
      return s ? s.name : null
    })
    .filter(Boolean)
})
</script>
