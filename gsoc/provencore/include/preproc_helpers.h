/*
 * Copyright (c) 2016-2023 ProvenRun S.A.S
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
 * @brief Pre-Processing helper macros
 * @author Henri Chataing
 * @date November 30th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _PREPROC_HELPERS_H_INCLUDED_
#define _PREPROC_HELPERS_H_INCLUDED_

/**
 * @brief Expand its argument.
 */
#define __EXPAND(x) x

/**
 * @brief Concatenate its two arguments.
 */
#define __CONCATENATE(x,y) x##y

/**
 * @brief Count the number of variadic parameters, to a maximum of 8
 * Implementation note: dummy parameter is added to support counting of empty
 * __VA_ARGS__.
 */
#define __COUNT_ARGS(...) __COUNT_ARGS1(__dummy, ##__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define __COUNT_ARGS1(__dummy, _1, _2, _3, _4, _5, _6, _7, _8, count, ...) count

/*
 * Allows to get the value of a define as a string:
 *
 * #define FOO BAR
 *
 * int main(void)
 * {
 *      puts("FOO = " __XSTR(FOO);
 *      return 0;
 * }
 *
 * will output "FOO = BAR"
 */
#define __XSTR(tok) __STR(tok)
#define __STR(tok) #tok

/**
 * Returns 1 if the argument is a empty defined token, or 0 otherwise.
 * e.g.
 *  `#define TOKEN0 test`
 *  `#define TOKEN1`
 *
 * __EMPTY(TOKEN0) => 0
 * __EMPTY(TOKEN1) => 1
 * __EMPTY(UNDEFINEDTOKEN) => 0
 */
#define __EMPTY(tok) __EMPTY1(tok)
#define __EMPTY1(...) __EMPTY2(0, ##__VA_ARGS__, 0, 1)
#define __EMPTY2(_0, _1, _2, ...) _2

#define __FOR_EACH_0(what)
#define __FOR_EACH_1(what, x, ...) what(x)
#define __FOR_EACH_2(what, x, ...) what(x) __FOR_EACH_1(what, __VA_ARGS__)
#define __FOR_EACH_3(what, x, ...) what(x) __FOR_EACH_2(what, __VA_ARGS__)
#define __FOR_EACH_4(what, x, ...) what(x) __FOR_EACH_3(what, __VA_ARGS__)
#define __FOR_EACH_5(what, x, ...) what(x) __FOR_EACH_4(what, __VA_ARGS__)
#define __FOR_EACH_6(what, x, ...) what(x) __FOR_EACH_5(what, __VA_ARGS__)
#define __FOR_EACH_7(what, x, ...) what(x) __FOR_EACH_6(what, __VA_ARGS__)
#define __FOR_EACH_8(what, x, ...) what(x) __FOR_EACH_7(what, __VA_ARGS__)

/**
 * @brief Apply the metho \p what to each element in a __VA_ARGS__ list.
 * Only works for lists of up to 8 elements.
 */
#define __FOR_EACH(what, ...) \
    __FOR_EACH1(__COUNT_ARGS(__VA_ARGS__), what, ##__VA_ARGS__)
#define __FOR_EACH1(N, what, ...) \
    __CONCATENATE(__FOR_EACH_, N)(what, ##__VA_ARGS__)

#endif /* _PREPROC_HELPERS_H_INCLUDED_ */
