/*
 * Copyright (c) 2020-2020 Prove & Run S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * Prove & Run S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with Prove & Run S.A.S
 *
 * PROVE & RUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVE & RUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

/*
 * @file
 * @copyright ProvenRun and/or its affiliates. All rights reserved.
 * @date 2020--2022
 * @author Olivier Delande
 *
 * @brief Unidirectional ring buffers.
 */

/*!
 * This library implements a generic ring buffer data structure which allows a
 * producer to asynchronously send messages to a consumer.
 *
 * Introduction
 * ============
 *
 * For a general introduction to ring buffers, see
 * <https://en.wikipedia.org/wiki/Circular_buffer>.
 *
 * This library allows a producer and a consumer to communicate with a ring
 * buffer. It does not deal with the messages themselves. Instead, it tracks the
 * progress of the producer and consumer, and helps them deal with empty and
 * full rings.
 *
 * The library uses shared atomic integers to synchronize accesses by the
 * producer and consumer, which may run concurrently. It is designed to resist
 * against attacks by malicious peers: if the producer or consumer writes
 * ill-formed values to the shared structures, it will only disrupt the
 * communication, not lead its peer to crash.
 *
 * Usage
 * =====
 *
 * The library assumes that the user has set up a zone of memory shared between
 * the producer and consumer in which to place the following structures:
 * - The array of message slots supporting the ring buffer. It must be writable
 *   by the producer and readable by the consumer. The capacity C of the ring
 *   buffer (i.e. the number of slots in the array) must be a power of 2 less
 *   than 2^32.
 *   This library never accesses this array directly. It only deals with message
 *   indices (in [0, C)) that the user can then use to access message slots.
 * - A structure of type ur_shrd_prod_t in which the producer publishes
 *   information for the consumer. It must be writable by the producer and
 *   readable by the consumer.
 * - A structure of type ur_shrd_cons_t in which the consumer publishes
 *   information for the producer. It must be writable by the consumer and
 *   readable by the producer.
 *
 * In addition, the user must allocate two private structures:
 * - A structure of type ur_priv_prod_t private to the producer.
 * - A structure of type ur_priv_cons_t private to the consumer.
 *
 * Notifications
 * -------------
 *
 * When the ring buffer is empty, the consumer may be asleep, waiting for a
 * notification from the producer that there are new messages to consume.
 *
 * When the ring buffer is full, the producer may be asleep, waiting for a
 * notification from the consumer that some message slots have been freed and
 * that production can resume.
 *
 * This library does not perform those notifications itself. It assumes that the
 * user has set up such a mechanism (e.g. using interrupts, POSIX wait
 * conditions, etc.). All that the library does is:
 * - internally, make the peers negociate whether/when they wish to be sent
 *   notifications;
 * - let the user know, through the return values of the ur_prod_commit()
 *   and ur_cons_commit() functions, when it is time to notify the other peer.
 *
 * Initialization
 * --------------
 *
 * Before communication can begin, someone must initialize the ur_shrd_prod_t
 * and ur_shrd_cons_t structures by calling the ur_shrd_prod_init() and
 * ur_shrd_cons_init() functions. It is outside the scope of this library to
 * decide who performs this initialization, but its effects must be visible to
 * both peers before communication can begin.
 *
 * Moreover, each peer must initialize its private structure with the
 * ur_prod_init() or ur_cons_init() function. In addition to a pointer to the
 * private structure to initialize, these functions take pointers to the shared
 * structures, which they conveniently store inside the private structure once
 * and for all, so that the rest of the API only takes a pointer to the private
 * structure. They also take the capacity of the ring buffer, which must be a
 * power of 2 on which both peers agree (e.g. a constant, or a value negociated
 * by a mechanism outside this library).
 *
 * Consumption
 * -----------
 *
 * The ring starts empty, with the consumer waiting for a notification from the
 * producer that production has started.
 *
 * The consumer keeps a private count of the messages that are available for
 * consumption. The ur_cons_unconsumed() function returns that number.
 *
 * If ur_cons_unconsumed() returns a number N > 0, then the consumer can consume
 * N messages, by performing the following steps N times:
 * 1. Get the index (in [0, C)) of the next message to consume by calling
 *    ur_cons_peek().
 * 2. Read the message from the appropriate slot. If the consumer does not trust
 *    the producer, it should read the message with RING_SAFE_READ() to thwart
 *    TOC/TOU attacks.
 * 3. Privately register that the message has been read by calling
 *    ur_cons_advance().
 *
 * If ur_cons_unconsumed() returns 0, then it is time for the consumer to
 * synchronize with the producer and check whether new messages have been
 * produced. To do so, call ur_cons_simple_checkout().
 *
 * If ur_cons_simple_checkout() returns true, then there are messages to
 * consume, and ur_cons_unconsumed() returns a number N > 0. Messages can be
 * consumed as explained above.
 *
 * If ur_cons_simple_checkout() returns false, then there are no new messages to
 * consume. It is time for the consumer to let the producer know that it is
 * going to sleep and that it needs to be woken up by a notification as soon as
 * there are new messages. To do so, call ur_cons_final_checkout(). Because new
 * messages may have been produced just before the call,
 * ur_cons_final_checkout() also checks for new messages, and returns true if
 * there are.
 *
 * If ur_cons_final_checkout() returns true, then there are messages to consume,
 * ur_cons_unconsumed() returns a number N > 0. Messages can be consumed as
 * explained above.
 *
 * If ur_cons_final_checkout() returns false, then the consumer can safely go to
 * sleep; the producer will send a notification when there is a new message.
 *
 * The consumer must also let the producer know when it has consumed something.
 * After one or several cycles of ur_cons_peek() / ur_cons_advance(), the
 * consumer must call ur_cons_commit(). That call makes the consumption visible
 * to the producer. If it returns true, then the user MUST send a notification
 * to the producer.
 *
 * Production
 * ----------
 *
 * Production is similar to consumption, with a few differences.
 *
 * First, the ring starts empty, and the producer knows that it has room for
 * producing C messages, where C is the capacity of the ring buffer.
 *
 * The producer must look for free space much like the consumer looks for new
 * messages:
 * - ur_prod_free() tracks a private count of the free message slots that are
 *   available for production. It is similar to ur_cons_unconsumed().
 * - When ur_prod_free() returns 0, ur_prod_simple_checkout() can be used to
 *   synchronize with the consumer. It is similar to ur_cons_simple_checkout().
 * - When ur_prod_simple_checkout() returns false, it is time to let the
 *   consumer know that the producer is about to go to sleep with a call to
 *   ur_prod_final_checkout(). That function is similar to
 *   ur_cons_final_checkout().
 * - When ur_prod_final_checkout() returns false, the producer can safely go to
 *   sleep.
 *
 * When the producer has determined that it has room to produce a message, it
 * can do so by performing the following steps:
 * 1. Get the index (in [0, C)) of the next message to produce by calling
 *    ur_prod_peek().
 * 2. Write the message to the appropriate slot.
 * 3. Privately register that the message has been written by calling
 *    ur_prod_advance().
 *
 * After it has produced at least one message, the producer must let the
 * consumer know about it by calling ur_prod_commit(). If the call returns true,
 * then the user MUST send a notification to the consumer.
 *
 * Further details
 * ===============
 *
 * A message slot is identified by its "index" in [0, C).
 *
 * Internally, the library tracks production and consumption by assigning
 * increasing "ordinals" to each message. A message with ordinal O is assigned
 * to slot O % C. Capacity, ordinals, and indices are all represented with
 * uint32_t. Because C is a power of 2, when O is incremented and overflows, the
 * corresponding index O % C wraps around from C - 1 to 0 as expected. The
 * distinction between ordinals and indices is crucial to telling an empty ring
 * from a full one. For example, a ring starts empty, and the producer knows
 * that its next message will have ordinal 0. After it has produced C messages,
 * the ring is full, and the next ordinal is C. Although ordinals 0 and C both
 * map to index 0, we need them to be different because they represent different
 * situations.
 */

#![no_std]

pub mod io_ring_buffer;

/**
 * Ring layout
 * ===========
 *
 * Each of the following diagrams presents a consecutive range of ordinals
 * (modulo 2^32) whose length is the capacity of the ring, starting at the
 * leftmost ordinal on the diagram.
 *
 * Shared view
 * -----------
 *
 * Seen from an external observer, the ring is partitioned in two logical
 * ranges, one under the control of each peer. The ranges are delimited by the
 * shared atomic ordinals as depicted by the following diagram.
 *
 * ```text
 * ┌──────────┬──────────┐
 * │ Consumer │ Producer │
 * │  slots   │  slots   │
 * └──────────┴──────────┘
 *  ↑          ↑
 *  shared     shared
 *  cons       prod
 * ```
 *
 * Producer's view
 * ---------------
 *
 * The producer maintains its private bookkeeping information, and sees the ring
 * as follows:
 *
 * ```text
 * ┌─────────────┬───────────────┬──────────┐
 * │ Uncommitted │ Available for │ Consumer │
 * │ production  │  production   │  slots   │
 * └─────────────┴───────────────┴──────────┘
 *  ↑             ↑               ↑
 *  last_         private         prod_end
 *  published_    prod
 *  prod
 * ```
 *
 * Consumer's view
 * ---------------
 *
 * Similarly, for the consumer:
 *
 * ```text
 * ┌─────────────┬────────────┬──────────┐
 * │ Uncommitted │ Unconsumed │ Producer │
 * │ consumption │  messages  │  slots   │
 * └─────────────┴────────────┴──────────┘
 *  ↑             ↑            ↑
 *  last_         private      cons_end
 *  published_    cons
 *  cons
 * ```
 */

pub mod uni_ring {

    use core::sync::atomic::{AtomicU32, Ordering};

    pub struct UrShrdCons {
        /// The ordinal of the next message to consume
        cons: AtomicU32,
        /// When prod strictly passes this value, the producer shall notify the
        /// consumer
        prod_limit: AtomicU32,
    }

    impl UrShrdCons {
        pub fn init(&self) -> () {
            self.cons.store(0, Ordering::Relaxed);
            self.prod_limit.store(0, Ordering::Relaxed);
        }
    }

    pub struct UrShrdProd {
        /// The ordinal of the next message to produce
        prod: AtomicU32,
        /// When cons strictly passes this value, the consumer shall notify the
        /// producer
        cons_limit: AtomicU32,
    }

    impl UrShrdProd {
        pub fn init(&mut self) -> () {
            self.prod.store(0, Ordering::Relaxed);
            self.cons_limit.store(0xFFFFFFFF, Ordering::Relaxed);
        }
    }

    pub struct UrPrivProd<'a> {
        // Invariant: (prod - last_published_prod) mod 2^32 <=
        //   (prod_end - last_published_prod) mod 2^32 <= capacity
        /// A pointer to the information shared by the producer
        shrd_prod: &'a mut UrShrdProd,
        /// A pointer to the information shared by the consumer
        shrd_cons: &'a mut UrShrdCons,
        /// The capacity of the ring
        capacity: u32,
        /// The ordinal of the next message to produce; it will be copied to shrd_prod
        /// when we commit
        prod: u32,
        /// The most recent value of prod that was copied to shrd_prod
        last_published_prod: u32,
        /// An ordinal such that we may consume until prod == prod_end
        prod_end: u32,
    }

    impl<'a> UrPrivProd<'a> {
        pub fn new(
            shrd_prod: &'a mut UrShrdProd,
            shrd_cons: &'a mut UrShrdCons,
            capacity: u32,
        ) -> Self {
            return Self {
                shrd_prod: shrd_prod,
                shrd_cons: shrd_cons,
                capacity: capacity,
                prod: 0,
                last_published_prod: 0,
                prod_end: capacity,
            };
        }

        pub fn free(&self) -> u32 {
            /*
            if 0 == (self.prod_end - self.prod) {
                pnc::println!("  capacity : {}", self.capacity);
                pnc::println!("  prod : {}", self.prod);
                pnc::println!("  last_published_prod : {}", self.last_published_prod);
                pnc::println!("  prod_end : {}", self.prod_end);
                pnc::println!(
                    "  shrd_prod.prod : {}",
                    self.shrd_prod.prod.load(Ordering::SeqCst)
                );
                pnc::println!(
                    "  shrd_prod.cons_limit : {}",
                    self.shrd_prod.cons_limit.load(Ordering::SeqCst)
                );
                pnc::println!(
                    "  shrd_cons.cons : {}",
                    self.shrd_cons.cons.load(Ordering::SeqCst)
                );
                pnc::println!(
                    "  shrd_cons.prod_limit : {}",
                    self.shrd_cons.prod_limit.load(Ordering::SeqCst)
                );
            }
            */
            return self.prod_end - self.prod;
        }

        pub fn peek(&self) -> u32 {
            return self.prod % self.capacity;
        }

        pub fn advance(&mut self) -> () {
            self.prod += 1;
        }

        pub fn simple_checkout(&mut self) -> bool {
            return self.checkout_aux(self.shrd_cons.cons.load(Ordering::Acquire));
        }

        pub fn final_checkout(&mut self) -> bool {
            self.shrd_prod
                .cons_limit
                .store(self.prod_end - self.capacity, Ordering::SeqCst);
            let cons = self.shrd_cons.cons.load(Ordering::SeqCst);
            return self.checkout_aux(cons);
        }

        pub fn checkout_aux(&mut self, cons: u32) -> bool {
            let capacity = self.capacity;

            // If the consumer pretends it has gone backwards since we last checked, or
            // beyond the published prod, then something is wrong
            if (cons + capacity - self.prod_end)
                <= (self.last_published_prod + capacity - self.prod_end)
            {
                self.prod_end = cons + capacity;
                // Let us show that the invariants are preserved. Let pe = old value of
                // prod_end, pe' = new value of prod_end, lpp = last_published_prod, C =
                // capacity, c = cons, md(x) = x mod 2^32.
                // We have md(pe' - lpp) = md(md(c + C - pe) + md(pe - lpp)).
                // md(c + C - pe) <= md(lpp + C - pe) thanks to the test.
                // md(lpp + C - pe) = md(C - md(pe - lpp)) = C - md(pe - lpp) by invariant.
                // Therefore md(c + C - pe) + md(pe - lpp) <= C,
                // md(pe' - lpp) = md(c + C - pe) + md(pe - lpp), and:
                // * md(pe' - lpp) >= md(pe - lpp) >= md(prod - lpp) by invariant,
                // * md(pe' - lpp) <= C.
            }
            return self.prod_end != self.prod;
        }

        pub fn commit(&mut self) -> bool {
            let last_published_prod = self.last_published_prod;
            let prod = self.prod;
            if prod == last_published_prod {
                return false;
            }
            // Make sure that the load from prod_limit is not reordered before the store
            // to prod
            self.shrd_prod.prod.store(prod, Ordering::SeqCst);
            let prod_limit = self.shrd_cons.prod_limit.load(Ordering::SeqCst);
            self.last_published_prod = prod;
            return prod.wrapping_sub(last_published_prod)
                > prod_limit.wrapping_sub(last_published_prod);
        }
    }

    pub struct UrPrivCons<'a> {
        shrd_prod: &'a mut UrShrdProd,
        /// A pointer to the information shared by the consumer
        shrd_cons: &'a mut UrShrdCons,
        /// The capacity of the ring
        capacity: u32,
        /// The ordinal of the next message to consume
        cons: u32,
        /// The most recent value of cons that was copied to shrd_cons
        last_published_cons: u32,
        /// An ordinal such that we may consume until cons == cons_end
        cons_end: u32,
    }

    impl<'a> UrPrivCons<'a> {
        pub fn new(
            shrd_prod: &'a mut UrShrdProd,
            shrd_cons: &'a mut UrShrdCons,
            capacity: u32,
        ) -> Self {
            return UrPrivCons {
                shrd_prod: shrd_prod,
                shrd_cons: shrd_cons,
                capacity: capacity,
                cons: 0,
                last_published_cons: 0,
                cons_end: 0,
            };
        }

        pub fn checkout_aux(&mut self, prod: u32) -> bool {
            if (prod - self.cons_end) <= (self.last_published_cons + self.capacity - self.cons_end)
            {
                self.cons_end = prod;
                // Let us show that the invariants are preserved. Let ce = old value of
                // cons_end, ce' = new value of cons_end, lpc = last_published_cons, C =
                // capacity, p = prod, md(x) = x mod 2^32.
                // We have md(ce' - lpc) = md(md(p - ce) + md(ce - lpc)).
                // md(p - ce) <= md(lpc + C - ce) thanks to the test.
                // md(lpc + C - ce) = md(C - md(ce - lpc)) = C - md(ce - lpc) by invariant.
                // Therefore md(p - ce) + md(ce - lpc) <= C,
                // md(ce' - lpc) = md(p - ce) + md(ce - lpc), and:
                // * md(ce' - lpc) >= md(ce - lpc) >= md(cons - lpc) by invariant,
                // * md(ce' - lpc) <= C.
            }
            return self.cons_end != self.cons;
        }

        pub fn simple_checkout(&mut self) -> bool {
            let prod = self.shrd_prod.prod.load(Ordering::Acquire);
            return self.checkout_aux(prod);
        }

        pub fn final_checkout(&mut self) -> bool {
            // Let the producer know that it needs to notify us when it produces more.
            // Then check out, in case the producer produced something before we asked for
            // a notification
            self.shrd_cons
                .prod_limit
                .store(self.cons_end, Ordering::SeqCst);
            let prod = self.shrd_prod.prod.load(Ordering::SeqCst);
            return self.checkout_aux(prod);
        }

        pub fn commit(&mut self) -> bool {
            let last_published_cons = self.last_published_cons;
            let cons = self.cons;
            if cons == last_published_cons {
                return false;
            }
            self.last_published_cons = cons;
            // Make sure that the load from cons_limit is not reordered before the store to cons
            // TODO why call it store/release
            self.shrd_cons.cons.store(cons, Ordering::SeqCst); // TODO double check this
            let cons_limit = self.shrd_prod.cons_limit.load(Ordering::SeqCst); // TODO double check this
            return cons.wrapping_sub(last_published_cons)
                > cons_limit.wrapping_sub(last_published_cons);
        }

        pub fn unconsumed(&self) -> u32 {
            return self.cons_end - self.cons;
        }

        pub fn peek(&self) -> u32 {
            return self.cons % self.capacity;
        }

        pub fn advance(&mut self) -> () {
            self.cons += 1;
        }
    }
}
