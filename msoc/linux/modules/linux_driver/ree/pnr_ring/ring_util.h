/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2023, ProvenRun S.A.S
 */
_Pragma("once")

/**
 * @file
 * @copyright ProvenRun and/or its affiliates. All rights reserved.
 * @date 2020--2022
 * @author Olivier Delande
 *
 * @brief Utilities for ring buffers.
 */

#if defined(__KERNEL__)
// The Linux kernel does not have stdint.h, but it has booleans
#include <linux/types.h>
#elif defined(__FreeBSD_kernel__)
#include <sys/stdint.h>
#else
#include <stdbool.h>
#include <stdint.h>
#endif

// The bit used as wrap counter in ring buffer positions
#if defined(__KERNEL__)
#define RING_WRAP (U32_C(1) << 31)
#else
#define RING_WRAP (UINT32_C(1) << 31)
#endif

/// The maximal supported ring buffer capacity
#define RING_MAX_CAPA RING_WRAP

/**
 * @param v an unsigned 32-bit value
 * @return whether it is a valid ring buffer capacity
 */
static inline bool ring_is_valid_capacity(uint32_t v)
{
	return v > 0 && v <= RING_MAX_CAPA;
}

/**
 * @param p a position in a ring buffer
 * @return the corresponding index
 */
static inline uint32_t ring_index(uint32_t p)
{
	return p & ~RING_WRAP;
}

/**
 * @brief Determines if a value is a valid position in a ring buffer.
 * @param capa the capacity of the ring buffer
 * @param p the value
 * @return whether \p p is valid
 */
static inline bool ring_valid(uint32_t capa, uint32_t p)
{
	return ring_index(p) < capa;
}

/**
 * @brief Computes the distance from one position to another in a ring buffer.
 *
 * Assuming that \p from_p and \p to_p are both valid positions in a ring buffer
 * of valid capacity \p capa, let d be the number of slots it takes to advance
 * from \p from_p to \p to_p.
 * If d <= \p capa, then the function returns d.
 * Otherwise, it returns a number > \p capa.
 * @param capa the capacity of the ring buffer
 * @param from_p the starting position
 * @param to_p the ending position
 * @return the distance (subject to the aforementioned limitations)
 */
static inline uint32_t ring_dist(uint32_t capa, uint32_t from_p,
	uint32_t to_p)
{
	uint32_t d = ring_index(to_p) - ring_index(from_p);

	if (((from_p ^ to_p) & RING_WRAP) != 0)
		d += capa;
	return d;
}

/**
 * @brief Determines whether a position is within an interval.
 *
 * Assuming that \p min, \p max, and \p p are valid positions in a ring buffer
 * of valid capacity \p capa, and that \p max is at most \p capa steps ahead of
 * \p min, determines whether \p p is in [\p min, \p max].
 * @param capa the capacity of the ring buffer
 * @param min the lower bound
 * @param max the upper bound
 * @param p the tested position
 * @return whether \p p is in [\p min, \p max] (subject to the aforementioned
 * limitations)
 */
static inline bool ring_in_bounds(uint32_t capa, uint32_t min, uint32_t max,
	uint32_t p)
{
	return ring_dist(capa, min, p) <= ring_dist(capa, min, max);
}

/**
 * @brief Advances a ring buffer position by one step.
 * @param capa the capacity of the ring buffer
 * @param p the position
 * @return the next position, assuming that \p p is valid
 */
static inline uint32_t ring_incr(uint32_t capa, uint32_t p)
{
	++p;
	if (ring_index(p) == capa)
		p = ~p & RING_WRAP;
	return p;
}

/**
 * @brief Represents a logical (position-based) range of messages in a ring
 * buffer.
 *
 * The length of the range is less than or equal to the capacity of the ring.
 *
 * To iterate on a logical range \c r, do:
 * ~~~{.c}
 * for (uint32_t i = ring_index(r.begin_pos), d = ring_dist(r.capa, r.begin_pos,
 *	 r.end_pos); d > 0; --d) {
 *	 // Access slot [i]
 *	 // ...
 *	 if (++i == r.capa) {
 *		 i = 0;
 *	 }
 * }
 * ~~~
 */
typedef struct {
	/// The capacity of the ring buffer
	uint32_t capa;
	/// The first position of the range
	uint32_t begin_pos;
	/// The position past the end of the range. Equal to begin_pos if the range is
	/// empty
	uint32_t end_pos;
} ring_lrange_t;

/**
 * @brief Represents the two physical (index-based) ranges corresponding to a
 * logical (position-based) range of messages in a ring buffer.
 *
 * If the logical range is empty, so are the two physical ranges.
 *
 * If the logical range does not wrap around the ring buffer, it is represented
 * by the first physical range, and the second range is empty.
 *
 * If the logical range wraps around the ring buffer, the two physical ranges
 * are used.
 *
 * The second physical range always begins at index 0.
 *
 * To iterate on a pair of physical ranges \c r, do:
 * ~~~{.c}
 * for (uint32_t i = r.fst_begin_idx; i < r.fst_end_idx; ++i) {
 *	 // Access slot [i]
 *	 // ...
 * }
 * for (uint32_t i = 0; i < r.snd_end_idx; ++i) {
 *	 // Access slot [i]
 *	 // ...
 * }
 * ~~~
 */
typedef struct {
	/// The index of the first slot of the first physical range
	uint32_t fst_begin_idx;
	/// The index past the end of the first physical range. Equal to fst_begin_idx
	/// if the first range is empty
	uint32_t fst_end_idx;
	/// The index past the end of the second physical range. 0 if the second range
	/// is empty
	uint32_t snd_end_idx;
} ring_pranges_t;

/**
 * @brief Converts a logical range of message positions in a ring buffer to a
 * pair of physical index ranges.
 * @param lrange the logical range
 * @return the corresponding physical ranges
 */
ring_pranges_t ring_pranges(ring_lrange_t lrange);
