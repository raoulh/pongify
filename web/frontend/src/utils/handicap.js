/**
 * Computes handicap points for a match between two players.
 *
 * @param {string} ranking1 - Player 1 ranking (numeric string or "NC")
 * @param {string} ranking2 - Player 2 ranking (numeric string or "NC")
 * @param {number[]} [handicapTable=[0,1,2,3,4]] - Table mapping écart index to points
 * @returns {{ handicap1: number, handicap2: number }}
 */
export function computeHandicap(ranking1, ranking2, handicapTable) {
  const table = Array.isArray(handicapTable) && handicapTable.length > 0
    ? handicapTable
    : [0, 1, 2, 3, 4]

  const r1 = ranking1 === 'NC' ? 95 : parseInt(ranking1, 10)
  const r2 = ranking2 === 'NC' ? 95 : parseInt(ranking2, 10)

  if (isNaN(r1) || isNaN(r2)) return { handicap1: 0, handicap2: 0 }

  const ecart = Math.floor(Math.abs(r1 - r2) / 5)
  const points = table[Math.min(ecart, table.length - 1)]

  if (r1 > r2) {
    return { handicap1: points, handicap2: 0 }
  }
  if (r2 > r1) {
    return { handicap1: 0, handicap2: points }
  }
  return { handicap1: 0, handicap2: 0 }
}
