// Module for various generally useful minor things.


/*
    Copyright (C) 2004-2017 Daniel Vallstrom. All rights reserved.

    Unless explicitly acquired and licensed from Licensor under a license
    other than the Reciprocal Public License ("RPL"), the contents of this
    file are subject to the RPL Version 1.1, or subsequent versions as
    allowed by the RPL, and You may not copy or use this file in either
    source code or executable form, except in compliance with the terms
    and conditions of the RPL.

    You should be able to find a copy of the RPL (the "License") in a file
    named LICENSE that should come along with this file; if not, write to
    daniel.vallstrom@gmail.com.

    All software distributed under the License is provided in the hope
    that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    PURPOSE. See the License for more details.
*/


/*
  allocStrCopy    -- Returns a new copy of a string.
  max             -- Returns the maximum of 2 numbers.
  min             -- Returns the minimum of 2 numbers.
  randomN         -- Returns a random number.
  readLL          -- Converts a string and handles errors.
  readReal        -- Converts a string and handles errors.
  readSignedReal  -- Converts a string and handles errors.
  readUInt        -- Converts a string and handles errors.
  readUIntBase    -- Converts a string and handles errors.
  readUIntPart    -- Converts a string and handles errors.
  readULLInt      -- Converts a string and handles errors.
  readULLIntBase  -- Converts a string and handles errors.
  sort3FM         -- Sorts 3 numbers using (< o f). A macro.
  sort3M          -- Sorts 3 numbers. A macro.
  swapM           -- Swaps 2 numbers. A macro.
*/

#ifndef common_H
#define common_H


#include <stdbool.h>
#include <assert.h>


#if AssertionLevel >= 2
#define assert2(a)  assert(a)
#else
#define assert2(a)
#endif

#if AssertionLevel >= 3
#define assert3(a)  assert(a)
#else
#define assert3(a)
#endif

#if AssertionLevel >= 4
#define assert4(a)  assert(a)
#else
#define assert4(a)
#endif

#if !defined(NDEBUG)
// Defined like this for extra safety.
#define evalAndAssert(a)                        \
    do                                          \
    {                                           \
        bool tmpEvalAndAssertMacroVar = a;      \
        assert( tmpEvalAndAssertMacroVar );     \
    }                                           \
    while (0)
#else
#define evalAndAssert(a)  a
#endif


#define PI ((double)(3.14159265358979323846))

// A random angle [0..2PI).
#define RandomAngle() ( ( rand() / (double)((unsigned long)RAND_MAX+1) ) *   \
                        (2*PI) )

// A random double [0..1).
#define RandomD() ( rand() / (double)((unsigned long)RAND_MAX+1) )

// Returns a new copy of s, or NULL if the allocation failed.
char * allocStrCopy( char * s );

// Returns the maximum of x and y.
#define max( x, y )  ( (x) >= (y) ? (x) : (y) )

// Returns the minimum of x and y.
#define min( x, y )  ( (x) <= (y) ? (x) : (y) )

// Returns a random number greater than or equal to 0, and less than n.
unsigned int randomN( unsigned int n );

// Converts s into a long long int which is placed in n. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error.
bool readLL( char * s, long long int * n );

// Converts s into a double which is placed in x. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error. s must not be negative and must have no exponent. E.g. "55.666"
// "55." and "242" are valid strings. The string must begin with a digit so
// e.g. ".666" isn't valid.
bool readReal( char * s, double * x );

// Like readReal except that an optional '-' prefix is allowed.
bool readSignedReal( char * s, double * x );

// Converts s into an unsigned int which is placed in n. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error.
bool readUInt( char * s, unsigned int * n );

// Converts s into an unsigned int which is placed in n. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error.
//   The string may specify a different base than 10. E.g.
// binary '0b1101', hex '0xd', octal '015' or plain decimal '13'.
bool readUIntBase( char * s, unsigned int * n );

// Converts *sPtr into an unsigned int which is placed in n. Returns true iff
// a parse error has occurred. Trailing non-number characters in s will *not*
// result in parse error. Instead, *sPtr is updated to point to the first
// trailing non-number. (Still *sPtr must start with a number though, i.e.
// **sPtr must be a digit when the function is called, otherwise true (parse
// error) will be returned.)
bool readUIntPart( char * * sPtr, unsigned int * n );

// Converts s into an unsigned long long int which is placed in n. Returns
// true iff a parse error has occurred. Trailing non-number characters in s
// will result in parse error.
bool readULLInt( char * s, unsigned long long int * n );

// Converts s into an unsigned long long int which is placed in n. Returns
// true iff a parse error has occurred. Trailing non-number characters in s
// will result in parse error.
//   The string may specify a different base than 10. E.g.
// binary '0b1101', hex '0xd', octal '015' or plain decimal '13'.
bool readULLIntBase( char * s, unsigned long long int * n );


// Swaps 'a' and 'b' using temp as temporary storage.
#define swapM( a, b, temp )  ( (temp) = (a), (a) = (b), (b) = (temp) )

// Sorts a, b and c using temp as temporary storage.
//   The code is small but there are many swaps.
#define sort3MShort( a, b, c, temp )     \
{                                        \
    if ( (b) < (a) )                     \
    {                                    \
        swapM( (a), (b), (temp) );       \
    }                                    \
                                         \
    if ( (c) < (b) )                     \
    {                                    \
        swapM( (b), (c), (temp) );       \
                                         \
        if ( (b) < (a) )                 \
        {                                \
            swapM( (a), (b), (temp) );   \
        }                                \
    }                                    \
}

// Sorts a, b and c optimally using temp as temporary storage.
#define sort3M( a, b, c, temp )                 \
{                                               \
    if ( (a) > (b) )                            \
    {                                           \
        if ( (a) > (c) )                        \
        {                                       \
            if ( (b) > (c) )                    \
            {                                   \
                swapM( (a), (c), (temp) );      \
            }                                   \
            else                                \
            {                                   \
                (temp) = (a);                   \
                (a) = (b);                      \
                (b) = (c);                      \
                (c) = (temp);                   \
            }                                   \
        }                                       \
        else                                    \
        {                                       \
            swapM( (a), (b), (temp) );          \
        }                                       \
    }                                           \
    else                                        \
    {                                           \
        if ( (b) > (c) )                        \
        {                                       \
            if ( (a) > (c) )                    \
            {                                   \
                (temp) = (a);                   \
                (a) = (c);                      \
                (c) = (b);                      \
                (b) = (temp);                   \
            }                                   \
            else                                \
            {                                   \
                swapM( (b), (c), (temp) );      \
            }                                   \
        }                                       \
    }                                           \
}


// Sorts a, b and c optimally using temp as temporary storage.
// 'f' is an array. The order relation used is (< o f).
#define sort3FM( a, b, c, temp, f )             \
{                                               \
    if ( (f)[a] > (f)[b] )                      \
    {                                           \
        if ( (f)[a] > (f)[c] )                  \
        {                                       \
            if ( (f)[b] > (f)[c] )              \
            {                                   \
                swapM( (a), (c), (temp) );      \
            }                                   \
            else                                \
            {                                   \
                (temp) = (a);                   \
                (a) = (b);                      \
                (b) = (c);                      \
                (c) = (temp);                   \
            }                                   \
        }                                       \
        else                                    \
        {                                       \
            swapM( (a), (b), (temp) );          \
        }                                       \
    }                                           \
    else                                        \
    {                                           \
        if ( (f)[b] > (f)[c] )                  \
        {                                       \
            if ( (f)[a] > (f)[c] )              \
            {                                   \
                (temp) = (a);                   \
                (a) = (c);                      \
                (c) = (b);                      \
                (b) = (temp);                   \
            }                                   \
            else                                \
            {                                   \
                swapM( (b), (c), (temp) );      \
            }                                   \
        }                                       \
    }                                           \
}

#endif // common_H
