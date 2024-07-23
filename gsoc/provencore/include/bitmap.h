/*
 * Copyright (c) 2015-2023 ProvenRun S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * ProvenRun S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with ProvenRun S.A.S
 *
 * PROVENRUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVENRUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */
/**
 * @file
 * @brief Helper functions and macro for bitmap management
 * @author Vincent Siles
 * @date March 26th, 2015 (creation)
 * @copyright (c) 2015-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _BITMAP_H_INCLUDED_
#define _BITMAP_H_INCLUDED_

#include <limits.h>
#ifdef __cplusplus
#include <stdbool.h>
#endif
#include <stddef.h>
#include <stdint.h>

typedef uint8_t bitchunk_t;

/* Constants and macros for bit map manipulation. */
#define BITCHUNK_BITS   (sizeof(bitchunk_t) * CHAR_BIT)
#define BITMAP_CHUNKS(nr_bits) (((nr_bits)+BITCHUNK_BITS-1)/BITCHUNK_BITS)

static inline bitchunk_t *MAP_CHUNK(bitchunk_t *map, size_t bit)
{
    // #define MAP_CHUNK(map,bit) ((map)[(bit)/BITCHUNK_BITS])
    return &map[bit / (BITCHUNK_BITS)];
}

/* Same as MAP_CHUNK, but with constant bit map. */
static inline bitchunk_t const *MAP_CHUNK_CONST(bitchunk_t const *map, size_t bit)
{
    return &map[bit / (BITCHUNK_BITS)];
}

static inline size_t CHUNK_OFFSET(size_t bit)
{
    //#define CHUNK_OFFSET(bit) ((bit)%BITCHUNK_BITS)
    return bit % (BITCHUNK_BITS);
}

static inline _Bool GET_BIT(bitchunk_t const *map, size_t bit)
{
    //#define GET_BIT(map,bit) ( MAP_CHUNK((map),(bit)) & (UINT8_C(1) << CHUNK_OFFSET(bit) ))
    bitchunk_t chunk = *MAP_CHUNK_CONST(map, bit);
    return (chunk & (UINT8_C(1) << CHUNK_OFFSET(bit))) != 0;
}

static inline void SET_BIT(bitchunk_t *map, size_t bit)
{
    //#define SET_BIT(map,bit) ( MAP_CHUNK((map),(bit)) |= (UINT8_C(1) << CHUNK_OFFSET(bit) ))
    bitchunk_t *chunk = MAP_CHUNK(map, bit);
    *chunk |= UINT8_C(1) << CHUNK_OFFSET(bit);
}

static inline void UNSET_BIT(bitchunk_t *map, size_t bit)
{
    //#define UNSET_BIT(map,bit) ( MAP_CHUNK((map),(bit)) &= ~(UINT8_C(1) << CHUNK_OFFSET(bit) ))
    bitchunk_t *chunk = MAP_CHUNK(map, bit);
    *chunk &= ~(UINT8_C(1) << CHUNK_OFFSET(bit));
}

#endif /* _BITMAP_H_INCLUDED_ */
