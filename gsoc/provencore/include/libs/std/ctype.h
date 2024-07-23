/*
 * Copyright (c) 2018-2023 ProvenRun S.A.S
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
 * @file ctype.h
 * @brief ctype implementation
 * @author Alexandre Berdery
 * @date June 21st, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _KLIB_CTYPE_H_INCLUDED_
#define _KLIB_CTYPE_H_INCLUDED_

/*
 * The functions defined in this header adapt to the current locale.
 * As locales are not supported in the ProvenCore libc implementation,
 * the default C locale always applies.
 */

/**
 * The _tolower() macro is equivalent to tolower(c) except that the argument
 * c must be an upper-case letter.
 */
#define _tolower(c)     ((c) + ('a' - 'A'))

/**
 * The _toupper() macro is equivalent to toupper() except that the argument
 * c must be a lower-case letter.
 */
#define _toupper(c)     ((c) + ('A' - 'a'))

/** Checks for a digit, equivalent to [0-9] */
static inline int isdigit(int c)
{
    return ((c >= '0') && (c <= '9'));
}

/** Checks for a lowercase letter, equivalent to [a-z] */
static inline int islower(int c)
{
    return ((c >= 'a') && (c <= 'z'));
}

/** Checks for an uppercase letter, equivalent to [A-Z] */
static inline int isupper(int c)
{
    return ((c >= 'A') && (c <= 'Z'));
}

/** Checks for an alphabetic character, equivalent to [A-Za-z] */
static inline int isalpha(int c)
{
    return (isupper(c) || islower(c));
}

/** Checks for an alphanumeric character, equivalent to [A-Za-z0-9] */
static inline int isalnum(int c)
{
    return (isalpha(c) || isdigit(c));
}

/** Checks for a hexadecimal digits, equivalent to [0-9A-Fa-f] */
static inline int isxdigit(int c)
{
    return (isdigit(c) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

/** Checks for a blank character, equivalent to [ \t] */
static inline int isblank(int c)
{
    return ((c == ' ') || (c == '\t'));
}

/** Checks for a control character, equivalent to [\x00-\x1F\x7F] */
static inline int iscntrl(int c)
{
    return (((c >= '\0') && (c <= 0x1F)) || (c == 0x7F));
}

/** Checks for any printable character except space, equivalent to [^[\x00-\x20\x7F]] */
static inline int isgraph(int c)
{
    return ((c > ' ') && (c <= '~'));
}

/** Checks for any printable character including space, equivalent to [^[\x00-\x1F\x7F]] */
static inline int isprint(int c)
{
    return !iscntrl(c);
}

/** Checks for any printable character which is not a space or an alphanumeric character */
static inline int ispunct(int c)
{
    return (isgraph(c) && !isalnum(c));
}

/** Checks for white-space characters, equivalent to [ \t\n\v\f\r] */
static inline int isspace(int c)
{
    return ((c == ' ') || ((c >= '\t') && (c <= '\r')));
}

/**
 * Checks whether c is a 7-bit unsigned char value that fits into the ASCII
 * character set, equivalent to [\x00-\x7F]
 */
static inline int isascii(int c)
{
    return ((c >= 0x0) && (c <= 0x7F));
}

/** Converts integer to a 7-bit ASCII character. Returns the value (\p c & 0x7f). */
static inline int toascii(int c)
{
    return c & 0x7f;
}

/** Converts the letter c to lower case, if possible */
static inline int tolower(int c)
{
    if (isupper(c))
        return _tolower(c);
    return c;
}

/** Converts the letter c to upper case, if possible */
static inline int toupper(int c)
{
    if (islower(c))
        return _toupper(c);
    return c;
}

#endif /* _KLIB_CTYPE_H_INCLUDED_ */
