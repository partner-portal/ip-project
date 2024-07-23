// SPDX-License-Identifier: GPL-2.0-only
/**
 * @file
 * @copyright ProvenRun and/or its affiliates. All rights reserved.
 * @date 2020--2023
 * @author Olivier Delande
 *
 * @brief Lightweight unidirectional ring buffers.
 */

#include "light_ring.h"

void lr_prod_init(lr_priv_prod_t *priv_prod, lr_shrd_prod_t *shrd_prod,
	lr_shrd_cons_t *shrd_cons, uint32_t capacity)
{
	priv_prod->shrd_prod = shrd_prod;
	priv_prod->shrd_cons = shrd_cons;
	priv_prod->capacity = capacity;
	priv_prod->prod = 0;
	priv_prod->last_published_prod = 0;
}

bool lr_prod_commit(lr_priv_prod_t *priv_prod)
{
	uint32_t last_published_prod = priv_prod->last_published_prod,
		prod = priv_prod->prod, prod_evt;

	if (prod == last_published_prod)
		return false;
	priv_prod->last_published_prod = prod;
	// Make sure that the load from prod_evt is not reordered before the store to
	// prod
	prod_evt = PNR_STORE_RELEASE_THEN_LOAD_RELAXED(&priv_prod->shrd_prod->prod,
		prod, &priv_prod->shrd_cons->prod_evt);
	return ring_valid(priv_prod->capacity, prod_evt) &&
		!ring_in_bounds(priv_prod->capacity, last_published_prod, prod_evt, prod);
}

void lr_cons_init(lr_priv_cons_t *priv_cons, lr_shrd_prod_t *shrd_prod,
	lr_shrd_cons_t *shrd_cons, uint32_t capacity)
{
	priv_cons->shrd_prod = shrd_prod;
	priv_cons->shrd_cons = shrd_cons;
	priv_cons->capacity = capacity;
	priv_cons->cons = 0;
	priv_cons->cons_end = 0;
}

/**
 * @brief An auxiliary function for the consumer's checkouts.
 *
 * After the prod position has been loaded, checks that it is not too far away
 * and, if so, updates the private state accordingly.
 * @param priv_cons a pointer to the consumer's private structure
 * @param prod the prod position
 * @return whether there are unconsumed messages in the ring
 */
static bool lr_cons_checkout_aux(lr_priv_cons_t *priv_cons, uint32_t prod)
{
	uint32_t cons;

	cons = priv_cons->cons;
	// If the producer pretends it has gone backwards since we last checked, or
	// beyond the private cons, then something is wrong
	if (ring_valid(priv_cons->capacity, prod) && ring_in_bounds(
		priv_cons->capacity, priv_cons->cons_end, cons ^ RING_WRAP, prod)) {
		priv_cons->cons_end = prod;
	}
	return priv_cons->cons_end != cons;
}

bool lr_cons_simple_checkout(lr_priv_cons_t *priv_cons)
{
	return lr_cons_checkout_aux(priv_cons, PNR_ATOMIC_LOAD_ACQUIRE(
		&priv_cons->shrd_prod->prod));
}

bool lr_cons_final_checkout(lr_priv_cons_t *priv_cons)
{
	// Let the producer know that it needs to notify us when it produces more.
	// Then check out, in case the producer produced something before we asked for
	// a notification
	uint32_t prod;

	prod = PNR_STORE_RELAXED_THEN_LOAD_ACQUIRE(&priv_cons->shrd_cons->prod_evt,
		priv_cons->cons_end, &priv_cons->shrd_prod->prod);
	return lr_cons_checkout_aux(priv_cons, prod);
}
