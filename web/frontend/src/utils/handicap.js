/**
 * Computes handicap points for a match between two players.
 * Reproduces the QML formula from BracketView.qml.
 *
 * @param {string} ranking1 - Player 1 ranking (numeric string or "NC")
 * @param {string} ranking2 - Player 2 ranking (numeric string or "NC")
 * @returns {{ handicap1: number, handicap2: number }}
 */
export function computeHandicap(ranking1, ranking2) {
  const r1 = ranking1 === 'NC' ? 95 : parseInt(ranking1, 10)
  const r2 = ranking2 === 'NC' ? 95 : parseInt(ranking2, 10)

  if (isNaN(r1) || isNaN(r2)) return { handicap1: 0, handicap2: 0 }

  if (r1 > r2) {
    return { handicap1: Math.min(Math.floor((r1 - r2) / 5), 4), handicap2: 0 }
  }
  if (r2 > r1) {
    return { handicap1: 0, handicap2: Math.min(Math.floor((r2 - r1) / 5), 4) }
  }
  return { handicap1: 0, handicap2: 0 }
}
