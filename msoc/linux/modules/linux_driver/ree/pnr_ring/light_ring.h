/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2023, ProvenRun S.A.S
 */
_Pragma("once")

/**
 * @file
 * @copyright ProvenRun and/or its affiliates. All rights reserved.
 * @date 2020--2023
 * @author Olivier Delande
 *
 * @brief Lightweight unidirectional ring buffers.
 *
 * This library implements a generic ring buffer data structure which allows a
 * producer to asynchronously send messages to a consumer.
 *
 * Introduction
 * ============
 *
 * For a general introduction to ring buffers, see
 * https://en.wikipedia.org/wiki/Circular_buffer.
 *
 * This library allows a producer and a consumer to communicate with a ring
 * buffer. It does not deal with the messages themselves. Instead, it tracks the
 * progress of the producer and consumer, and helps them deal with empty rings.
 *
 * The library uses shared atomic integers to synchronize accesses by the
 * producer and consumer, which may run concurrently. It is designed to resist
 * against attacks by malicious peers: if the producer or consumer writes
 * ill-formed values to the shared structures, it will only disrupt the
 * communication, not lead its peer to crash.
 *
 * In contrast to other ring buffer libraries, this library tracks production,
 * but not consumption: it allows the producer to let the consumer know when it
 * has produced something, but has no facilities to let the producer know when
 * the consumer has freed some space for production. It is assumed that the user
 * has some external way to know this. The upshot is that the library has less
 * synchronization overhead (hence the "lightweight" qualifier) that if it has
 * to track production.
 *
 * A typical use case is when a server and client communicate according to the
 * following protocol. The client sends requests to the server and the server
 * later sends responses back to the client, and the number of outstanding
 * requests (i.e. requests for which a response has not yet been sent) must
 * never be greater than a limit N. That is, if there are N outstanding
 * requests, the client must wait for at least one response before it can send
 * another request. In such a situation, a lightweight ring buffer with a
 * capacity >= N can be used for the responses: the server need not track the
 * client's consumption of responses because, according to the protocol, it
 * knows that it will always have room for producing a response when it needs to
 * do so.
 *
 * Usage
 * =====
 *
 * The library assumes that the user has set up a zone of memory shared between
 * the producer and consumer in which to place the following structures:
 * - The array of message slots supporting the ring buffer. It must be writable
 *   by the producer and readable by the consumer. The capacity C of the ring
 *   buffer (i.e. the number of slots in the array) must be in [1, 2^31].
 *   This library never accesses this array directly. It only deals with message
 *   indices (in [0, C)) that the user can then use to access message slots.
 * - A structure of type lr_shrd_prod_t in which the producer publishes
 *   information for the consumer. It must be writable by the producer and
 *   readable by the consumer.
 * - A structure of type lr_shrd_cons_t in which the consumer publishes
 *   information for the producer. It must be writable by the consumer and
 *   readable by the producer.
 *
 * In addition, the user must allocate two private structures:
 * - A structure of type lr_priv_prod_t private to the producer.
 * - A structure of type lr_priv_cons_t private to the consumer.
 *
 * Notifications
 * -------------
 *
 * When the ring buffer is empty, the consumer may be asleep, waiting for a
 * notification from the producer that there are new messages to consume.
 *
 * This library does not perform those notifications itself. It assumes that the
 * user has set up such a mechanism (e.g. using interrupts, POSIX wait
 * conditions, etc.). All that the library does is:
 * - internally, make the consumer negotiate whether/when it wishes to be sent
 *   notifications;
 * - let the user know, through the return values of the lr_prod_commit()
 *   function, when it is time to notify the consumer.
 *
 * Initialization
 * --------------
 *
 * Before communication can begin, someone must initialize the lr_shrd_prod_t
 * and lr_shrd_cons_t structures by calling the lr_shrd_prod_init() and
 * lr_shrd_cons_init() functions. It is outside the scope of this library to
 * decide who performs this initialization, but its effects must be visible to
 * both peers before communication can begin.
 *
 * Moreover, each peer must initialize its private structure with the
 * lr_prod_init() or lr_cons_init() function. In addition to a pointer to the
 * private structure to initialize, these functions take pointers to the shared
 * structures, which they conveniently store inside the private structure once
 * and for all, so that the rest of the API only takes a pointer to the private
 * structure. They also take the capacity of the ring buffer, which must be an
 * integer in [1, 2^31] on which both peers agree (e.g. a constant, or a value
 * negotiated by a mechanism outside this library).
 *
 * Consumption
 * -----------
 *
 * The ring starts empty, with the consumer waiting for a notification from the
 * producer that production has started.
 *
 * The consumer privately tracks which messages are available for consumption.
 * The lr_cons_can_consume() function determines whether there is at least one
 * unconsumed message.
 *
 * While lr_cons_can_consume() returns true, the consumer can consume a message,
 * by performing the following steps:
 * 1. Get the index (in [0, C)) of the next message to consume by calling
 *    lr_cons_peek().
 * 2. Read the message from the appropriate slot. If the consumer does not trust
 *    the producer, it should read the message with PNR_SAFE_READ() to thwart
 *    TOCTOU attacks.
 * 3. Privately register that the message has been read by calling
 *    lr_cons_advance().
 *
 * If lr_cons_can_consume() returns false, then it is time for the consumer to
 * synchronize with the producer and check whether new messages have been
 * produced. To do so, call lr_cons_simple_checkout().
 *
 * If lr_cons_simple_checkout() returns true, then there are messages to
 * consume. Messages can be consumed as explained above.
 *
 * If lr_cons_simple_checkout() returns false, then there are no new messages to
 * consume. It is time for the consumer to let the producer know that it is
 * going to sleep and that it needs to be woken up by a notification as soon as
 * there are new messages. To do so, call lr_cons_final_checkout(). Because new
 * messages may have been produced just before the call,
 * lr_cons_final_checkout() also checks for new messages, and returns true if
 * there are.
 *
 * If lr_cons_final_checkout() returns true, then there are messages to consume.
 * Messages can be consumed as explained above.
 *
 * If lr_cons_final_checkout() returns false, then the consumer can safely go to
 * sleep; the producer will send a notification when there is a new message.
 *
 * Production
 * ----------
 *
 * Production is similar to consumption, with a few differences.
 *
 * First, the ring starts empty, and the producer knows that it has room for
 * producing C messages, where C is the capacity of the ring buffer.
 *
 * The producer must have a way to know at any time, by some external mechanism,
 * whether there are free slots where it can produce messages in the ring
 * buffer.
 *
 * When the producer has determined that it has room to produce a message, it
 * can do so by performing the following steps:
 * 1. Get the index (in [0, C)) of the next message to produce by calling
 *    lr_prod_peek().
 * 2. Write the message to the appropriate slot.
 * 3. Privately register that the message has been written by calling
 *    lr_prod_advance().
 *
 * After it has produced at least one message, the producer must let the
 * consumer know about it by calling lr_prod_commit(). If the call returns true,
 * then the user MUST send a notification to the consumer.
 *
 * Further details
 * ===============
 *
 * A message slot is identified by its "index" in [0, C).
 *
 * Internally, the library tracks production and consumption by assigning
 * "positions" to each message. A position is a pair (w, i), where:
 * - i, in [0, C), is the index in the array;
 * - w, the "wrap counter", is 0 or 1, and changes when a position is
 *   incremented and wraps around the array.
 * A position is represented by the integer w * 2^31 + i.
 * The wrap counter is crucial to telling an empty ring from a full one. For
 * example, a ring starts empty, and the producer knows that its next message
 * will have position (0, 0). After it has produced C messages, the ring is
 * full, and the next position is (1, 0). Although both positions map to index
 * 0, we need them to be different because they represent different situations.
 */

/*
 * Ring layout
 * ===========
 *
 * Each of the following diagrams presents a consecutive range of positions
 * whose length is the capacity of the ring, starting at the leftmost position
 * on the diagram.
 *
 * Shared view
 * -----------
 *
 * Seen from an external observer, the ring is partitioned in two logical
 * ranges, one under the control of each peer. The ranges are delimited by the
 * shared atomic position prod, and another bound maintained outside this
 * library, as depicted by the following diagram.
 *
 * ┌─────────────────────┬────────────────┐
 * │   Consumer slots    │ Producer slots │
 * └─────────────────────┴────────────────┘
 *  ↑                     ↑
 *  bound maintained      shared
 *  outside this library  prod
 *
 * Producer's view
 * ---------------
 *
 * The producer maintains its private bookkeeping information, and sees the ring
 * as follows:
 *
 * ┌─────────────┬───────────────┬──────────┐
 * │ Uncommitted │ Available for │ Consumer │
 * │ production  │  production   │  slots   │
 * └─────────────┴───────────────┴──────────┘
 *  ↑             ↑               ↑
 *  last_         private         bound maintained
 *  published_    prod            outside this library
 *  prod
 *
 * Consumer's view
 * ---------------
 *
 * Similarly, for the consumer:
 *
 * ┌─────────────────────┬────────────┬──────────┐
 * │     Uncommitted     │ Unconsumed │ Producer │
 * │     consumption     │  messages  │  slots   │
 * └─────────────────────┴────────────┴──────────┘
 *  ↑                     ↑            ↑
 *  bound maintained      private      cons_end
 *  outside this library  cons
 */

#include "ring_util.h"
#include "pnr_shared_mem.h"

/**
 * @brief The information shared by the producer.
 */
typedef struct {
	/// The position of the next message to produce
	PNR_ATOMIC(uint32_t) prod;
} lr_shrd_prod_t;

/**
 * @brief The information shared by the consumer.
 */
typedef struct {
	/// When prod strictly passes this value, the producer shall notify the
	/// consumer
	PNR_ATOMIC(uint32_t) prod_evt;
} lr_shrd_cons_t;

/**
 * @brief The information private to the producer.
 */
typedef struct {
	/// A pointer to the information shared by the producer
	lr_shrd_prod_t *shrd_prod;
	/// A pointer to the information shared by the consumer
	lr_shrd_cons_t *shrd_cons;
	/// The capacity of the ring
	uint32_t capacity;
	/// The position of the next message to produce; it will be copied to
	/// shrd_prod when we commit
	uint32_t prod;
	/// The most recent value of prod that was copied to shrd_prod
	uint32_t last_published_prod;
} lr_priv_prod_t;

/**
 * @brief The information private to the consumer.
 */
typedef struct {
	// Invariant: ring_dist(capacity, cons, cons_end) <= capacity
	/// A pointer to the information shared by the producer
	lr_shrd_prod_t *shrd_prod;
	/// A pointer to the information shared by the consumer
	lr_shrd_cons_t *shrd_cons;
	/// The capacity of the ring
	uint32_t capacity;
	/// The position of the next message to consume
	uint32_t cons;
	/// A position such that we may consume until cons == cons_end
	uint32_t cons_end;
} lr_priv_cons_t;

/**
 * @brief Initializes the structure shared by the producer.
 *
 * This function may be called either by the producer or the consumer.
 * Both lr_shrd_prod_init() and lr_shrd_cons_init() must be called before the
 * producer or consumer starts using the ring buffer (except for initializing
 * their private states).
 * @param shrd_prod a pointer to the information shared by the producer
 */
static inline void lr_shrd_prod_init(lr_shrd_prod_t *shrd_prod)
{
	PNR_ATOMIC_INIT(&shrd_prod->prod, 0);
}

/**
 * @brief Initializes the structure shared by the consumer.
 *
 * This function may be called either by the producer or the consumer.
 * Both lr_shrd_prod_init() and lr_shrd_cons_init() must be called before the
 * producer or consumer starts using the ring buffer (except for initializing
 * their private states).
 * @param shrd_cons a pointer to the information shared by the consumer
 */
static inline void lr_shrd_cons_init(lr_shrd_cons_t *shrd_cons)
{
	PNR_ATOMIC_INIT(&shrd_cons->prod_evt, 0);
}

/**
 * @brief Producer only. Initializes the producer's private structure. The
 * shared structures must be initialized before any other function is called.
 * @param priv_prod a pointer to the producer's private structure
 * @param shrd_prod a pointer to the information shared by the producer
 * @param shrd_cons a pointer to the information shared by the consumer
 * @param capacity the capacity of the ring buffer; must be in [1, 2^31]
 */
void lr_prod_init(lr_priv_prod_t *priv_prod, lr_shrd_prod_t *shrd_prod,
	lr_shrd_cons_t *shrd_cons, uint32_t capacity);

/**
 * @brief Producer only. Retrieves the capacity of the ring buffer.
 * @param priv_prod a pointer to the producer's private structure
 * @return the capacity of the ring buffer
 */
static inline uint32_t lr_prod_capacity(lr_priv_prod_t const *priv_prod)
{
	return priv_prod->capacity;
}

/**
 * @brief Producer only. Returns the index of the next slot in which to
 * produce a message.
 * @param priv_prod a pointer to the producer's private structure
 * @return the index of the next slot in which to produce a message
 */
static inline uint32_t lr_prod_peek(lr_priv_prod_t const *priv_prod)
{
	return ring_index(priv_prod->prod);
}

/**
 * @brief Producer only. Registers that a message was produced.
 *
 * The producer is expected to call lr_prod_commit() at some point to make that
 * production visible to the consumer.
 * @param priv_prod a pointer to the producer's private structure
 */
static inline void lr_prod_advance(lr_priv_prod_t *priv_prod)
{
	priv_prod->prod = ring_incr(priv_prod->capacity, priv_prod->prod);
}

/**
 * @brief Producer only. Makes all of the production so far visible to the
 * consumer.
 * @param priv_prod a pointer to the producer's private structure
 * @return whether a notification shall be sent to the consumer
 */
__attribute__ ((warn_unused_result)) bool lr_prod_commit(
	lr_priv_prod_t *priv_prod);

/**
 * @brief Consumer only. Initializes the consumer's private structure. The
 * shared structures must be initialized before any other function is called.
 * @param priv_cons a pointer to the consumer's private structure
 * @param shrd_prod a pointer to the information shared by the producer
 * @param shrd_cons a pointer to the information shared by the consumer
 * @param capacity the capacity of the ring buffer; must be in [1, 2^31]
 */
void lr_cons_init(lr_priv_cons_t *priv_cons, lr_shrd_prod_t *shrd_prod,
	lr_shrd_cons_t *shrd_cons, uint32_t capacity);

/**
 * @brief Consumer only. Retrieves the capacity of the ring buffer.
 * @param priv_cons a pointer to the consumer's private structure
 * @return the capacity of the ring buffer
 */
static inline uint32_t lr_cons_capacity(lr_priv_cons_t const *priv_cons)
{
	return priv_cons->capacity;
}

/**
 * @brief Consumer only. Checks out the unconsumed messages in the ring buffer
 * and updates the private state accordingly.
 *
 * This function does not properly deal with notifications.
 *
 * Indeed, when there are no unconsumed messages in the ring,
 * lr_cons_simple_checkout() does nothing special. This is fine as long as the
 * consumer soon checks out the ring again. However, if the consumer wants to
 * sleep until there are new messages to consume, it must ensure that it will
 * eventually be notified by the producer. That is what lr_cons_final_checkout()
 * does.
 * @param priv_cons a pointer to the consumer's private structure
 * @return whether there are unconsumed messages in the ring
 */
bool lr_cons_simple_checkout(lr_priv_cons_t *priv_cons);

/**
 * @brief Consumer only. Asks for a notification from the producer, then checks
 * out the unconsumed messages in the ring buffer and updates the private state
 * accordingly.
 *
 * This function is typically called after lr_cons_simple_checkout() has
 * returned false, and the consumer wishes to sleep until there are more
 * messages to consume. In the unlikely event that messages were produced since
 * the call to lr_cons_simple_checkout(), this call will return true; otherwise
 * it will return false and the consumer can go to sleep, safe in the knowledge
 * that the producer will notify it when it has produced more.
 * @param priv_cons a pointer to the consumer's private structure
 * @return whether there are unconsumed messages in the ring
 */
bool lr_cons_final_checkout(lr_priv_cons_t *priv_cons);

/**
 * @brief Consumer only. Determines whether there is at least one unconsumed
 * message in the ring buffer.
 * @param priv_cons a pointer to the consumer's private structure
 * @return whether there is at least one unconsumed message
 */
static inline bool lr_cons_can_consume(lr_priv_cons_t const *priv_cons)
{
	return priv_cons->cons_end != priv_cons->cons;
}

/**
 * @brief Consumer only. Returns the index of the next slot from which to
 * consume a message.
 *
 * The message may only be consumed if calls to lr_cons_simple_checkout(),
 * lr_cons_final_checkout(), or lr_cons_can_consume() determine that there is a
 * message to consume.
 * @param priv_cons a pointer to the consumer's private structure
 * @return the index of the next slot from which to consume a message
 */
static inline uint32_t lr_cons_peek(lr_priv_cons_t const *priv_cons)
{
	return ring_index(priv_cons->cons);
}

/**
 * @brief Consumer only. Returns the range of unconsumed messages.
 * @param priv_cons a pointer to the consumer's private structure
 * @return the range
 */
static inline ring_lrange_t lr_cons_peek_all(lr_priv_cons_t const *priv_cons)
{
	return (ring_lrange_t){priv_cons->capacity, priv_cons->cons,
		priv_cons->cons_end};
}

/**
 * @brief Consumer only. Registers that the first unconsumed message was
 * consumed.
 * @param priv_cons a pointer to the consumer's private structure
 */
static inline void lr_cons_advance(lr_priv_cons_t *priv_cons)
{
	priv_cons->cons = ring_incr(priv_cons->capacity, priv_cons->cons);
}
