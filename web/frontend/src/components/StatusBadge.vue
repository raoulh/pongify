<template>
  <span class="inline-flex items-center gap-1 text-xs px-2 py-0.5 rounded-full"
        :class="classes">
    <span class="w-1.5 h-1.5 rounded-full" :class="dotClass"></span>
    {{ label }}
  </span>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({ status: String })

const label = computed(() => {
  if (props.status === 'playing') return 'En cours'
  if (props.status === 'finished') return 'Terminé'
  return 'Non démarré'
})

const classes = computed(() => {
  if (props.status === 'playing') return 'bg-green-100 text-pongify-green'
  if (props.status === 'finished') return 'bg-amber-50 text-amber-600'
  return 'bg-gray-100 text-gray-500'
})

const dotClass = computed(() => {
  if (props.status === 'playing') return 'bg-pongify-green'
  if (props.status === 'finished') return 'bg-amber-500'
  return 'bg-gray-400'
})
</script>
