import { ref, readonly, computed, onMounted, onUnmounted } from 'vue'
import { getKeyFromFragment, decryptTournament } from '../crypto.js'

const POLL_INTERVALS = {
  playing: 10000,
  home: 30000,
  stopped: 60000,
  finished: 0
}

export function useTournament(uuid) {
  const tournament = ref(null)
  const isLoading = ref(true)
  const error = ref(null)
  const lastUpdated = ref(null)
  const pollContext = ref('home')
  const secondsAgo = ref(null)

  let pollTimer = null
  let tickTimer = null
  let lastKnownVersion = 0
  let isVisible = true
  const keyBytes = getKeyFromFragment(uuid)
  const keyMissing = !keyBytes
  const apiBase = window.location.origin

  function formatAgo(seconds) {
    if (seconds < 60) return `il y a ${seconds}s`
    const minutes = Math.floor(seconds / 60)
    if (minutes < 60) return `il y a ${minutes}min`
    const hours = Math.floor(minutes / 60)
    return `il y a ${hours}h${minutes % 60 > 0 ? String(minutes % 60).padStart(2, '0') : ''}`
  }

  const lastUpdateDisplay = computed(() => {
    if (secondsAgo.value === null) return ''
    return formatAgo(secondsAgo.value)
  })

  function startTick() {
    if (tickTimer) return
    tickTimer = setInterval(() => {
      if (lastUpdated.value) {
        secondsAgo.value = Math.floor((Date.now() - lastUpdated.value.getTime()) / 1000)
      }
    }, 1000)
  }

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

  function handlePageShow(e) {
    if (e.persisted) {
      fetchVersion()
      restartPoll()
    }
  }

  function handleOnline() {
    fetchVersion()
    restartPoll()
  }

  function handleFocus() {
    if (document.visibilityState === 'visible') {
      fetchVersion()
      restartPoll()
    }
  }

  async function fetchVersion() {
    try {
      const res = await fetch(`${apiBase}/api/tournament/${uuid}/version`)
      if (!res.ok) {
        if (res.status === 404) {
          error.value = 'Tournoi non trouvé ou expiré'
          isLoading.value = false
          tournament.value = null
        }
        return
      }
      const data = await res.json()
      if (data.updatedAt !== lastKnownVersion) {
        lastKnownVersion = data.updatedAt
        await fetchFullData()
      }
      // Don't clear error here if it was a key/decryption error — only fetchFullData can clear it
      if (error.value && !error.value.includes('déchiffr') && !error.value.includes('clé') && !error.value.includes('QR Code')) {
        error.value = null
      }
    } catch (e) {
      error.value = 'Connexion perdue'
    }
  }

  async function fetchFullData() {
    try {
      console.log('[Pongify] fetchFullData: uuid =', uuid, 'keyBytes =', keyBytes ? keyBytes.length + ' bytes' : 'null')
      const res = await fetch(`${apiBase}/api/tournament/${uuid}`)
      if (!res.ok) throw new Error(`HTTP ${res.status}`)

      const encrypted = await res.arrayBuffer()
      console.log('[Pongify] fetchFullData: received', encrypted.byteLength, 'bytes')

      if (!keyBytes) {
        error.value = 'Clé de déchiffrement manquante dans l\'URL'
        isLoading.value = false
        return
      }

      const data = await decryptTournament(encrypted, keyBytes)
      tournament.value = data
      lastUpdated.value = new Date(lastKnownVersion)
      secondsAgo.value = Math.floor((Date.now() - lastKnownVersion) / 1000)
      isLoading.value = false
      error.value = null
    } catch (e) {
      isLoading.value = false
      if (e.name === 'OperationError') {
        error.value = 'Impossible de déchiffrer les données. La clé dans l\'URL ne correspond pas à celle utilisée par le tournoi. Essayez de rescanner le QR code ou de rouvrir le lien original.'
      } else {
        error.value = 'Erreur de chargement: ' + (e.message || e.name)
      }
      console.error('[Pongify] fetchFullData error:', e.name, e.message, e)
    }
  }

  function retryLoad() {
    error.value = null
    isLoading.value = true
    lastKnownVersion = 0
    fetchVersion()
  }

  onMounted(() => {
    if (keyMissing) {
      console.warn('[Pongify] No encryption key available — skipping all network activity')
      error.value = 'L\'URL du tournoi n\'est plus valable. La clé de déchiffrement est manquante.\nVeuillez rescanner le QR Code ou rouvrir le lien original.'
      isLoading.value = false
      return
    }
    document.addEventListener('visibilitychange', handleVisibilityChange)
    window.addEventListener('pageshow', handlePageShow)
    window.addEventListener('online', handleOnline)
    window.addEventListener('focus', handleFocus)
    fetchVersion()
    restartPoll()
    startTick()
  })

  onUnmounted(() => {
    document.removeEventListener('visibilitychange', handleVisibilityChange)
    window.removeEventListener('pageshow', handlePageShow)
    window.removeEventListener('online', handleOnline)
    window.removeEventListener('focus', handleFocus)
    clearTimeout(pollTimer)
    clearInterval(tickTimer)
  })

  return {
    tournament: readonly(tournament),
    isLoading: readonly(isLoading),
    error: readonly(error),
    lastUpdateDisplay,
    secondsAgo: readonly(secondsAgo),
    setPollContext,
    retryLoad
  }
}
