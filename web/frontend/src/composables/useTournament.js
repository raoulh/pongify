import { ref, readonly, computed, onMounted, onUnmounted } from 'vue'
import { getKeyFromFragment, decryptTournament } from '../crypto.js'

const POLL_INTERVALS = {
  playing: 5000,
  home: 15000,
  stopped: 30000,
  finished: 0
}

export function useTournament(uuid) {
  const tournament = ref(null)
  const isLoading = ref(true)
  const error = ref(null)
  const lastUpdated = ref(null)
  const pollContext = ref('home')

  let pollTimer = null
  let lastKnownVersion = 0
  let isVisible = true
  const keyBytes = getKeyFromFragment()
  const apiBase = window.location.origin

  const lastUpdateTime = computed(() => {
    if (!lastUpdated.value) return ''
    return lastUpdated.value.toLocaleTimeString('fr-FR', { hour: '2-digit', minute: '2-digit' })
  })

  function setPollContext(context) {
    if (pollContext.value === context) return
    pollContext.value = context
    restartPoll()
  }

  function restartPoll() {
    clearTimeout(pollTimer)
    pollTimer = null
    if (!isVisible) return
    const interval = POLL_INTERVALS[pollContext.value] || POLL_INTERVALS.home
    if (interval === 0) return
    schedulePoll(interval)
  }

  function schedulePoll(interval) {
    pollTimer = setTimeout(async () => {
      await fetchVersion()
      if (isVisible && POLL_INTERVALS[pollContext.value] > 0) {
        schedulePoll(POLL_INTERVALS[pollContext.value])
      }
    }, interval)
  }

  function handleVisibilityChange() {
    isVisible = document.visibilityState === 'visible'
    if (isVisible) {
      fetchVersion()
      restartPoll()
    } else {
      clearTimeout(pollTimer)
      pollTimer = null
    }
  }

  async function fetchVersion() {
    try {
      const res = await fetch(`${apiBase}/api/tournament/${uuid}/version`)
      if (!res.ok) {
        if (res.status === 404) {
          error.value = 'Tournoi non trouvé ou expiré'
          tournament.value = null
        }
        return
      }
      const data = await res.json()
      if (data.updatedAt !== lastKnownVersion) {
        lastKnownVersion = data.updatedAt
        await fetchFullData()
      }
      error.value = null
    } catch (e) {
      error.value = 'Connexion perdue'
    }
  }

  async function fetchFullData() {
    try {
      const res = await fetch(`${apiBase}/api/tournament/${uuid}`)
      if (!res.ok) throw new Error(`HTTP ${res.status}`)

      const encrypted = await res.arrayBuffer()

      if (!keyBytes) {
        error.value = 'Clé de déchiffrement manquante dans l\'URL'
        return
      }

      const data = await decryptTournament(encrypted, keyBytes)
      tournament.value = data
      lastUpdated.value = new Date()
      isLoading.value = false
      error.value = null
    } catch (e) {
      if (e.name === 'OperationError') {
        error.value = 'Impossible de déchiffrer (clé invalide ?)'
      } else {
        error.value = 'Erreur de chargement'
      }
      console.error('[Pongify] fetchFullData error:', e.name, e.message)
    }
  }

  onMounted(() => {
    document.addEventListener('visibilitychange', handleVisibilityChange)
    fetchVersion()
    restartPoll()
  })

  onUnmounted(() => {
    document.removeEventListener('visibilitychange', handleVisibilityChange)
    clearTimeout(pollTimer)
  })

  return {
    tournament: readonly(tournament),
    isLoading: readonly(isLoading),
    error: readonly(error),
    lastUpdateTime,
    setPollContext
  }
}
