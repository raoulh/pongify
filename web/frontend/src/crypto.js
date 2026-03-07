/**
 * Reads the AES key from sessionStorage (set by main.js at startup).
 * Falls back to parsing the current URL hash.
 * @param {string} uuid - Tournament UUID for scoped storage lookup
 * @returns {Uint8Array|null}
 */
export function getKeyFromFragment(uuid) {
  const storageKey = 'pongify_key_' + uuid
  const stored = sessionStorage.getItem(storageKey)
  if (stored) {
    const bytes = base64urlDecode(stored)
    console.log('[Pongify] Key from sessionStorage: base64url length:', stored.length, '→', bytes.length, 'bytes')
    if (bytes.length !== 32) {
      console.warn('[Pongify] Key size mismatch! Expected 32 bytes, got', bytes.length)
    }
    return bytes
  }

  const hash = window.location.hash
  if (hash && hash.includes('K=')) {
    const keyPart = hash.split('K=')[1]
    if (keyPart) {
      const end = keyPart.indexOf('/')
      const raw = end >= 0 ? keyPart.substring(0, end) : keyPart
      const bytes = base64urlDecode(raw)
      console.log('[Pongify] Key from hash fallback: base64url length:', raw.length, '→', bytes.length, 'bytes')
      return bytes
    }
  }
  console.warn('[Pongify] No encryption key found for uuid:', uuid)
  return null
}

/**
 * Decrypts an AES-256-GCM blob.
 * @param {ArrayBuffer} encrypted - IV (12 bytes) + ciphertext + tag (16 bytes)
 * @param {Uint8Array} keyBytes - 32-byte AES key
 * @returns {Promise<object>} Parsed JSON
 */
export async function decryptTournament(encrypted, keyBytes) {
  console.log('[Pongify] Decrypting:', encrypted.byteLength, 'bytes, key:', keyBytes.length, 'bytes')

  const cryptoKey = await crypto.subtle.importKey(
    'raw', keyBytes, { name: 'AES-GCM' }, false, ['decrypt']
  )

  const iv = encrypted.slice(0, 12)
  const ciphertextAndTag = encrypted.slice(12)
  console.log('[Pongify] IV:', new Uint8Array(iv).length, 'bytes, ciphertext+tag:', ciphertextAndTag.byteLength, 'bytes')

  const plaintext = await crypto.subtle.decrypt(
    { name: 'AES-GCM', iv: new Uint8Array(iv) },
    cryptoKey,
    ciphertextAndTag
  )

  const text = new TextDecoder().decode(plaintext)
  console.log('[Pongify] Decrypted OK, plaintext size:', text.length)
  return JSON.parse(text)
}

/**
 * Decodes a base64url string to Uint8Array.
 * @param {string} str
 * @returns {Uint8Array}
 */
function base64urlDecode(str) {
  let base64 = str.replace(/-/g, '+').replace(/_/g, '/')
  while (base64.length % 4) base64 += '='
  const binary = atob(base64)
  const bytes = new Uint8Array(binary.length)
  for (let i = 0; i < binary.length; i++) {
    bytes[i] = binary.charCodeAt(i)
  }
  return bytes
}
