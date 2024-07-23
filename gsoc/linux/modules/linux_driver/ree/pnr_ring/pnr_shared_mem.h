/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2023, ProvenRun S.A.S
 */
_Pragma("once")

/**
 * @file
 * @copyright ProvenRun and/or its affiliates. All rights reserved.
 * @date 2022
 * @author Olivier Delande
 *
 * @brief Low-level utilities for communication in shared memory.
 */

/**
 * @brief Qualifies its Lvalue argument as volatile (and const), which makes it
 * safe to evaluate to avoid TOCTOU attacks.
 * @param v the Lvalue to read
 * @return the same Lvalue, qualified with volatile and const
 */
#define PNR_SAFE_READ(v) (*(__typeof__(v) const volatile *)&(v))

/**
 * @brief Forces the compiler to forget the contents of memory.
 *
 * This function can be used as an alternative to PNR_SAFE_READ to avoid TOCTOU
 * attacks. A sequence of calls to PNR_SAFE_READ is costly, and can be replaced
 * by a sequence of regular reads followed by a call to pnr_safe_read_barrier.
 */
static inline void pnr_safe_read_barrier(void)
{
	__asm__ volatile ("" : : : "memory");
}

/*
 * Atomics
 *
 * We can't use C11 atomics in the Linux kernel, because it has incompatible
 * definitions of some of the symbols. This section introduces appropriate
 * abstractions.
 *
 * PNR_ATOMIC(t)
 *   Turns a type t into an atomic version of it. It must be lock-free.
 * PNR_ATOMIC_INIT(p, v)
 *   Non-atomically initializes the atomic object *p to v.
 * PNR_ATOMIC_LOAD_<o>(p)
 *   Loads the atomic object *p with memory ordering <o>.
 * PNR_ATOMIC_STORE_<o>(p, v)
 *   Stores v to the atomic object *p with memory ordering <o>.
 * PNR_ATOMIC_EXCHANGE_<o>(p, v)
 *   Atomically loads the atomic object *p and stores v with memory ordering
 *   <o>. Returns the loaded value.
 * PNR_ATOMIC_COMPARE_EXCHANGE_<s>_<so>_<fo>(p, e, d)
 *   Atomically compares the value of the atomic object *p to the expected value
 *   *e and, if equal, replaces it with the desired value d. On success, memory
 *   ordering is affected according to <so>, and the macro evaluates to true. On
 *   failure, *e is overwritten with the read value, memory ordering is affected
 *   according to <fo>, and the macro evaluates to false. If <s> is STRONG, then
 *   there are no spurious failures: on failure, the old and new values of *e
 *   are different.
 * PNR_ATOMIC_FETCH_OR_<o>(p, v)
 *   Atomically applies *p |= v with memory ordering <o>, and returns *p's old
 *   value.
 * PNR_STORE_<so>_THEN_LOAD_<lo>(f, v, s)
 *   Stores v to the atomic object *f with memory ordering <so>, "then" loads
 *   the atomic object *s with memory ordering <lo>. Returns the loaded value.
 *   "then" means that we prevent the two operations from being reordered. See
 *   below for use cases.
 *
 * Synchronization with multiple atomics
 *
 * In many communication protocols, an agent signals a production by writing to
 * a 'prod' shared atomic integer, and a consuming agent reads it. These stores
 * and loads must respectively have release and acquire memory ordering
 * constraints for the messages to be effectively transferred.
 *
 * This simple situation is complicated by notification suppression mechanisms.
 * "After" it has updated prod, the producer must load an associated shared
 * atomic integer 'prod_evt' to see whether it must notify the consumer.
 * Similarly, when it considers sleeping because no messages are available for
 * consumption, the consumer must store to prod_evt "before" it loads prod to
 * see if it may have missed a notification. Both accesses to prod_evt may use
 * relaxed memory ordering. However, for a commit and a checkout to properly
 * synchronize, we need one of the following to be true:
 * - the store to prod synchronizes with the load from prod;
 * - the load from prod_evt reads from the store to prod_evt.
 *
 * Some sequential consistency is needed for that. We expose two macros to do
 * that:
 * - the producer uses PNR_STORE_RELEASE_THEN_LOAD_RELAXED to store to prod,
 *   then load from prod_evt;
 * - the consumer uses PNR_STORE_RELAXED_THEN_LOAD_ACQUIRE to store to prod_evt,
 *   then load from prod.
 *
 * Sometimes prod_evt is also used for transferring ownership (e.g. it can be
 * the consumer index). In those cases loads of prod_evt must use acquire memory
 * ordering, and stores must use release memory ordering. We provide the
 * stronger PNR_STORE_RELEASE_THEN_LOAD_ACQUIRE for those cases.
 */

/**
 * @brief Performs an atomic thread fence with sequentially consistent memory
 * ordering.
 */
static inline void pnr_atomic_thread_fence_seq_cst(void);

#if defined(__KERNEL__)

/*
 * In Linux's memory model, memory ordering constraints degenerate to compiler-
 * only barriers when CONFIG_SMP is disabled. We do not want that, because we
 * must support situations where Linux communicates with a system running on
 * another CPU, regardless of whether Linux itself runs on several CPUs. Linux
 * has limited support for that. When we don't have the appropriate support, we
 * check at compile time that CONFIG_SMP is enabled.
 */

#include <asm/barrier.h>
#include <linux/atomic.h>

#define PNR_ATOMIC_CHECK() _Static_assert(IS_ENABLED(CONFIG_SMP),              \
	"Some atomic primitives have no suitable implementation without CONFIG_SMP")

#define PNR_ATOMIC(t) t

#define PNR_ATOMIC_INIT(p, v) { *(p) = (v); }

#define PNR_ATOMIC_LOAD_RELAXED(p) READ_ONCE(*(p))
#define PNR_ATOMIC_LOAD_ACQUIRE(p) virt_load_acquire(p)
#define PNR_ATOMIC_LOAD_SEQ_CST(p) ({ \
	virt_mb();                          \
	virt_load_acquire(p);               \
})

#define PNR_ATOMIC_STORE_RELAXED(p, v) WRITE_ONCE(*(p), (v))
#define PNR_ATOMIC_STORE_RELEASE(p, v) virt_store_release((p), (v))
#define PNR_ATOMIC_STORE_SEQ_CST(p, v) do { \
	virt_store_release((p), (v));             \
	virt_mb();                                \
} while (0)

#define PNR_ATOMIC_EXCHANGE_RELAXED(p, v) \
	({PNR_ATOMIC_CHECK(); xchg_relaxed((p), (v)); })
#define PNR_ATOMIC_EXCHANGE_ACQUIRE(p, v) \
	({PNR_ATOMIC_CHECK(); xchg_acquire((p), (v)); })
#define PNR_ATOMIC_EXCHANGE_RELEASE(p, v) \
	({PNR_ATOMIC_CHECK(); xchg_release((p), (v)); })
#define PNR_ATOMIC_EXCHANGE_SEQ_CST(p, v) \
	({PNR_ATOMIC_CHECK(); xchg((p), (v)); })

// Not all versions of Linux define try_cmpxchg*
#ifdef try_cmpxchg_relaxed
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_RELAXED_RELAXED(p, e, d) \
	({PNR_ATOMIC_CHECK(); try_cmpxchg_relaxed((p), (e), (d)); })
#else
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_RELAXED_RELAXED(p, e, d) ({ \
	__typeof__(e) _e;                                                    \
	__typeof__(*_e) _expected, _found;                                   \
	PNR_ATOMIC_CHECK();                                                  \
	_e = (e);                                                            \
	_expected = *_e;                                                     \
	_found = cmpxchg_relaxed((p), _expected, (d));                       \
	if (unlikely(_found != _expected)) {                                 \
		*_e = _found;                                                      \
	}                                                                    \
	likely(_found == _expected);                                         \
})
#endif

#define PNR_ATOMIC_COMPARE_EXCHANGE_WEAK_RELAXED_RELAXED(p, e, d) \
	PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_RELAXED_RELAXED((p), (e), (d))

// Not all versions of Linux define try_cmpxchg*
#ifdef try_cmpxchg
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_RELAXED(p, e, d) \
	({PNR_ATOMIC_CHECK(); try_cmpxchg((p), (e), (d)); })
#else
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_RELAXED(p, e, d) ({ \
	__typeof__(e) _e;                                                    \
	__typeof__(*_e) _expected, _found;                                   \
	PNR_ATOMIC_CHECK();                                                  \
	_e = (e);                                                            \
	_expected = *_e;                                                     \
	_found = cmpxchg((p), _expected, (d));                               \
	if (unlikely(_found != _expected)) {                                 \
		*_e = _found;                                                      \
	}                                                                    \
	likely(_found == _expected);                                         \
})
#endif

#define PNR_ATOMIC_COMPARE_EXCHANGE_WEAK_SEQ_CST_RELAXED(p, e, d) \
	PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_RELAXED((p), (e), (d))

#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_SEQ_CST(p, e, d) ({      \
	_Bool _res;                                                               \
	virt_mb__before_atomic();                                                 \
	_res = PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_RELAXED_RELAXED((p), (e), (d)); \
	virt_mb__after_atomic();                                                  \
	_res;                                                                     \
})

#define PNR_ATOMIC_COMPARE_EXCHANGE_WEAK_SEQ_CST_SEQ_CST(p, e, d) \
	PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_SEQ_CST((p), (e), (d))

// Unfortunately, Linux only exposes the following RMW operations for a few
// types
#define PNR_ATOMIC_FETCH_OR_RELAXED(p, v) ({                                \
	PNR_ATOMIC_CHECK();                                                       \
	_Generic((p), uint32_t * : atomic_fetch_or_relaxed((v), (atomic_t *)(p))); \
})
#define PNR_ATOMIC_FETCH_OR_ACQUIRE(p, v) ({                                \
	PNR_ATOMIC_CHECK();                                                       \
	_Generic((p), uint32_t * : atomic_fetch_or_acquire((v), (atomic_t *)(p))); \
})
#define PNR_ATOMIC_FETCH_OR_RELEASE(p, v) ({                                \
	PNR_ATOMIC_CHECK();                                                       \
	_Generic((p), uint32_t * : atomic_fetch_or_release((v), (atomic_t *)(p))); \
})
#define PNR_ATOMIC_FETCH_OR_SEQ_CST(p, v) ({                                \
	PNR_ATOMIC_CHECK();                                                       \
	_Generic((p), uint32_t * : atomic_fetch_or((v), (atomic_t *)(p)));         \
})

#define PNR_STORE_RELEASE_THEN_LOAD_RELAXED(fst, val, snd) ({ \
	virt_store_release((fst), (val));                           \
	virt_mb();                                                  \
	READ_ONCE(*(snd));                                          \
})

#define PNR_STORE_RELAXED_THEN_LOAD_ACQUIRE(fst, val, snd) ({ \
	WRITE_ONCE(*(fst), (val));                                  \
	virt_mb();                                                  \
	virt_load_acquire(snd);                                     \
})

#define PNR_STORE_RELEASE_THEN_LOAD_ACQUIRE(fst, val, snd) ({ \
	virt_store_release((fst), (val));                           \
	virt_mb();                                                  \
	virt_load_acquire(snd);                                     \
})

static inline void pnr_atomic_thread_fence_seq_cst(void)
{
	virt_mb();
}

#else

#if defined(__FreeBSD_kernel__)
#include <sys/stdatomic.h>
#else
#include <stdatomic.h>
#endif

#define PNR_ATOMIC(t) _Atomic(t)

#define PNR_ATOMIC_INIT(p, v) atomic_init((p), (v))

#define PNR_ATOMIC_LOAD_RELAXED(p) \
	atomic_load_explicit((p), memory_order_relaxed)
#define PNR_ATOMIC_LOAD_ACQUIRE(p) \
	atomic_load_explicit((p), memory_order_acquire)
#define PNR_ATOMIC_LOAD_SEQ_CST(p) \
	atomic_load_explicit((p), memory_order_seq_cst)

#define PNR_ATOMIC_STORE_RELAXED(p, v) \
	atomic_store_explicit((p), (v), memory_order_relaxed)
#define PNR_ATOMIC_STORE_RELEASE(p, v) \
	atomic_store_explicit((p), (v), memory_order_release)
#define PNR_ATOMIC_STORE_SEQ_CST(p, v) \
	atomic_store_explicit((p), (v), memory_order_seq_cst)

#define PNR_ATOMIC_EXCHANGE_RELAXED(p, v) \
	atomic_exchange_explicit((p), (v), memory_order_relaxed)
#define PNR_ATOMIC_EXCHANGE_ACQUIRE(p, v) \
	atomic_exchange_explicit((p), (v), memory_order_acquire)
#define PNR_ATOMIC_EXCHANGE_RELEASE(p, v) \
	atomic_exchange_explicit((p), (v), memory_order_release)
#define PNR_ATOMIC_EXCHANGE_SEQ_CST(p, v) \
	atomic_exchange_explicit((p), (v), memory_order_seq_cst)

#define PNR_ATOMIC_COMPARE_EXCHANGE_WEAK_RELAXED_RELAXED(p, e, d)            \
	atomic_compare_exchange_weak_explicit((p), (e), (d), memory_order_relaxed, \
	memory_order_relaxed)
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_RELAXED_RELAXED(p, e, d)            \
	atomic_compare_exchange_strong_explicit((p), (e), (d), memory_order_relaxed, \
	memory_order_relaxed)
#define PNR_ATOMIC_COMPARE_EXCHANGE_WEAK_SEQ_CST_RELAXED(p, e, d)            \
	atomic_compare_exchange_weak_explicit((p), (e), (d), memory_order_seq_cst, \
	memory_order_relaxed)
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_RELAXED(p, e, d)            \
	atomic_compare_exchange_strong_explicit((p), (e), (d), memory_order_seq_cst, \
	memory_order_relaxed)
#define PNR_ATOMIC_COMPARE_EXCHANGE_WEAK_SEQ_CST_SEQ_CST(p, e, d)            \
	atomic_compare_exchange_weak_explicit((p), (e), (d), memory_order_seq_cst, \
	memory_order_seq_cst)
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_SEQ_CST(p, e, d)            \
	atomic_compare_exchange_strong_explicit((p), (e), (d), memory_order_seq_cst, \
	memory_order_seq_cst)

#define PNR_ATOMIC_FETCH_OR_RELAXED(p, v) \
	atomic_fetch_or_explicit((p), (v), memory_order_relaxed)
#define PNR_ATOMIC_FETCH_OR_ACQUIRE(p, v) \
	atomic_fetch_or_explicit((p), (v), memory_order_acquire)
#define PNR_ATOMIC_FETCH_OR_RELEASE(p, v) \
	atomic_fetch_or_explicit((p), (v), memory_order_release)
#define PNR_ATOMIC_FETCH_OR_SEQ_CST(p, v) \
	atomic_fetch_or_explicit((p), (v), memory_order_seq_cst)

/*
 * For every PNR_STORE_*_THEN_LOAD_* macro, we have two implementations:
 * 1. Make both accesses with seq_cst memory ordering.
 * 2. Make each access with minimal memory ordering, but insert a seq_cst fence
 *    between them.
 *
 * Implementation 1 is probably more efficient on ARMv8-A (both for Aarch32 and
 * Aarch64) because it only uses LDA(R) and STL(R)--no DMB. Implementation 2 is
 * more efficient on ARMv7-A, because it uses fewer DMBs.
 */

#if defined(__ARM_ARCH_8A__) || defined(__aarch64__)

#define PNR_STORE_RELEASE_THEN_LOAD_RELAXED(fst, val, snd) ({ \
	atomic_store_explicit((fst), (val), memory_order_seq_cst);  \
	atomic_load_explicit((snd), memory_order_seq_cst);          \
})

#define PNR_STORE_RELAXED_THEN_LOAD_ACQUIRE(fst, val, snd) ({ \
	atomic_store_explicit((fst), (val), memory_order_seq_cst);  \
	atomic_load_explicit((snd), memory_order_seq_cst);          \
})

#define PNR_STORE_RELEASE_THEN_LOAD_ACQUIRE(fst, val, snd) ({ \
	atomic_store_explicit((fst), (val), memory_order_seq_cst);  \
	atomic_load_explicit((snd), memory_order_seq_cst);          \
})

#else

#define PNR_STORE_RELEASE_THEN_LOAD_RELAXED(fst, val, snd) ({ \
	atomic_store_explicit((fst), (val), memory_order_release);  \
	atomic_thread_fence(memory_order_seq_cst);                  \
	atomic_load_explicit((snd), memory_order_relaxed);          \
})

#define PNR_STORE_RELAXED_THEN_LOAD_ACQUIRE(fst, val, snd) ({ \
	atomic_store_explicit((fst), (val), memory_order_relaxed);  \
	atomic_thread_fence(memory_order_seq_cst);                  \
	atomic_load_explicit((snd), memory_order_acquire);          \
})

#define PNR_STORE_RELEASE_THEN_LOAD_ACQUIRE(fst, val, snd) ({ \
	atomic_store_explicit((fst), (val), memory_order_release);  \
	atomic_thread_fence(memory_order_seq_cst);                  \
	atomic_load_explicit((snd), memory_order_acquire);          \
})

#endif

static inline void pnr_atomic_thread_fence_seq_cst(void)
{
	atomic_thread_fence(memory_order_seq_cst);
}

#endif

/* Implicitly sequentially consistent variants */

#define PNR_ATOMIC_LOAD(p) PNR_ATOMIC_LOAD_SEQ_CST(p)
#define PNR_ATOMIC_STORE(p, v) PNR_ATOMIC_STORE_SEQ_CST((p), (v))
#define PNR_ATOMIC_EXCHANGE(p, v) PNR_ATOMIC_EXCHANGE_SEQ_CST((p), (v))
#define PNR_ATOMIC_COMPARE_EXCHANGE_WEAK(p, e, d) \
	PNR_ATOMIC_COMPARE_EXCHANGE_WEAK_SEQ_CST_SEQ_CST((p), (e), (d))
#define PNR_ATOMIC_COMPARE_EXCHANGE_STRONG(p, e, d) \
	PNR_ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ_CST_SEQ_CST((p), (e), (d))
#define PNR_ATOMIC_FETCH_OR(p, v) PNR_ATOMIC_FETCH_OR_SEQ_CST((p), (v))
