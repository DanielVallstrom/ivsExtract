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


#include "common.h"
#include "compilerMacros.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>



// Returns a new copy of s, or NULL if the allocation failed.
char * allocStrCopy( char * s )
{
    size_t sSize = strlen(s) + 1;

    char * t = malloc(sSize);

    if ( Unlikely( t == NULL ) )
    {
        return NULL;
    }

    memcpy( t, s, sSize );

    return t;
}


// Returns a random number greater than or equal to 0, and less than n.
unsigned int randomN( unsigned int n )
{
    return (unsigned int) ( rand() / ( (double) RAND_MAX + 1 ) * n );
}


// Converts s into a long long int which is placed in n. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error.
bool readLL( char * s, long long int * n )
{
    // Contains the accumulated number.
    long long int k;

    // Holds the sign of the number.
    long long int negSign;


    if ( s == NULL )
    {
        return true;
    }

    if ( *s == '-' )
    {
        negSign = -1;
        s++;
    }
    else
    {
        negSign = 1;
    }

    if ( !isdigit(*s) )
    {
        return true;
    }
   
    k = *s - '0';
    s++;
    while ( isdigit(*s) )
    {
        k = 10 * k + ( *s - '0' );
        s++;
    }

    if ( *s != '\0' )
    {
        return true;
    }
        
    *n = negSign * k;

    return false;
}


// Converts s into a double which is placed in x. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error. s must not be negative and have no exponent. E.g. "55.666"
// "55." and "242" are valid strings. The string must begin with a digit so
// e.g. ".666" isn't valid.
bool readReal( char * s, double * x )
{
    // Contains the accumulated number.
    double y;

    if ( s == NULL  ||  !isdigit(*s) )
    {
        return true;
    }

    y = *s - '0';
    s++;
    while ( isdigit(*s) )
    {
        y = 10 * y + ( *s - '0' );
        s++;
    }

    if ( *s == '.' )
    {
        s++;
        for ( double z = 0.1; isdigit(*s); s++, z *= 0.1 )
        {
            y += z * ( *s - '0' );
        }
    }

    if ( *s != '\0' )
    {
        return true;
    }
        
    *x = y;

    return false;
}


// Like readReal except that an optional '-' prefix is allowed.
bool readSignedReal( char * s, double * x )
{
    if ( s == NULL  ||  *s != '-' )
    {
        return readReal( s, x );
    }

    if ( readReal( s+1, x ) )
    {
        return true;
    }

    *x = -*x;

    return false;
}


// Converts s into an unsigned int which is placed in n. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error.
bool readUInt( char * s, unsigned int * n )
{
    // Contains the accumulated number.
    unsigned int k;

    if ( s == NULL  ||  !isdigit(*s) )
    {
        return true;
    }

    k = *s - '0';
    s++;
    while ( isdigit(*s) )
    {
        k = 10 * k + ( *s - '0' );
        s++;
    }

    if ( *s != '\0' )
    {
        return true;
    }
        
    *n = k;

    return false;
}


// Converts s into an unsigned int which is placed in n. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error.
//   The base used is base. Digits must be less than base. The digit-value of
// a char c is c-'0'.
static bool readUIntB( char * s, unsigned int * n, unsigned int base )
{
    // Contains the accumulated number.
    unsigned int k = 0;

    if ( s == NULL  ||  *s < '0'  ||  *s-'0' >= base )
    {
        return true;
    }

    do
    {
        k = base * k + ( *s - '0' );
        s++;
    }
    while ( *s >= '0'  &&  *s-'0' < base );

    if ( *s != '\0' )
    {
        return true;
    }
        
    *n = k;

    return false;
}


// Converts a character to its corresponding number. Returns -1u for '\0'.
// Returns -1u if there is no number corresponding to c.
static unsigned int hexCharToInt( char c )
{
    switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;

    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;

    case '\0': return -1u;

    default: return -1u;
    }
}


// Converts s into an unsigned int which is placed in n. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error.
//   The base used is base. Digits must be less than base. 'a', 'b',... will
// also count as digits according to hexCharToInt above.
static bool readUIntHex( char * s, unsigned int * n, unsigned int base )
{
    // Contains the accumulated number.
    unsigned int k = 0;

    // The current digit being processed.
    unsigned int m;

    if ( s == NULL )
    {
        return true;
    }

    m = hexCharToInt(*s);

    if ( m >= base )
    {
        return true;
    }

    do
    {
        k = base * k + m;
        s++;
        m = hexCharToInt(*s);
    }
    while ( m < base );

    if ( *s != '\0' )
    {
        return true;
    }
        
    *n = k;

    return false;
}



bool readUIntBase( char * s, unsigned int * n )
{
    if ( s == NULL )
    {
        return true;
    }

    if ( *s == '0' )
    {
        if ( s[1] == 'b' )
        {
            return readUIntB( s+2, n, 2 );
        }
        else if ( s[1] == 'x' )
        {
            return readUIntHex( s+2, n, 16 );
        }
        else
        {
            return readUIntB( s+1, n, 8 );
        }
    }
    else
    {
        return readUInt( s, n );
    }
}



// Converts *sPtr into an unsigned int which is placed in n. Returns true iff
// a parse error has occurred. Trailing non-number characters in s will *not*
// result in parse error. Instead, *sPtr is updated to point to the first
// trailing non-number.
bool readUIntPart( char * * sPtr, unsigned int * n )
{
    char * s = *sPtr;

    // Contains the accumulated number.
    unsigned int k;

    if ( s == NULL  ||  !isdigit(*s) )
    {
        return true;
    }

    k = *s - '0';
    s++;
    while ( isdigit(*s) )
    {
        k = 10 * k + ( *s - '0' );
        s++;
    }

    *n = k;
    *sPtr = s;

    return false;
}



// Converts s into an unsigned long long int which is placed in n. Returns
// true iff a parse error has occurred. Trailing non-number characters in s
// will result in parse error.
bool readULLInt( char * s, unsigned long long int * n )
{
    // Contains the accumulated number.
    unsigned long long int k;

    if ( s == NULL  ||  !isdigit(*s) )
    {
        return true;
    }

    k = *s - '0';
    s++;
    while ( isdigit(*s) )
    {
        k = 10 * k + ( *s - '0' );
        s++;
    }

    if ( *s != '\0' )
    {
        return true;
    }
        
    *n = k;

    return false;
}


// Converts s into an unsigned long long int which is placed in n. Returns
// true iff a parse error has occurred. Trailing non-number characters in s
// will result in parse error.
//   The base used is base. Digits must be less than base. The digit-value of
// a char c is c-'0'.
static bool readULLIntB( char * s, unsigned long long int * n,
                         unsigned long long int base )
{
    // Contains the accumulated number.
    unsigned long long int k = 0;

    if ( s == NULL  ||  *s < '0'  ||  *s-'0' >= base )
    {
        return true;
    }

    do
    {
        k = base * k + ( *s - '0' );
        s++;
    }
    while ( *s >= '0'  &&  *s-'0' < base );

    if ( *s != '\0' )
    {
        return true;
    }
        
    *n = k;

    return false;
}


// Converts s into an unsigned long long int which is placed in n. Returns
// true iff a parse error has occurred. Trailing non-number characters in s
// will result in parse error.
//   The base used is base. Digits must be less than base. 'a', 'b',... will
// also count as digits according to hexCharToInt above.
static bool readULLIntHex( char * s, unsigned long long int * n,
                           unsigned long long int base )
{
    // Contains the accumulated number.
    unsigned long long int k = 0;

    // The current digit being processed.
    unsigned int m;

    if ( s == NULL )
    {
        return true;
    }

    m = hexCharToInt(*s);

    if ( m >= base )
    {
        return true;
    }

    do
    {
        k = base * k + m;
        s++;
        m = hexCharToInt(*s);
    }
    while ( m < base );

    if ( *s != '\0' )
    {
        return true;
    }
        
    *n = k;

    return false;
}


bool readULLIntBase( char * s, unsigned long long int * n )
{
    if ( s == NULL )
    {
        return true;
    }

    if ( *s == '0' )
    {
        if ( s[1] == 'b' )
        {
            return readULLIntB( s+2, n, 2 );
        }
        else if ( s[1] == 'x' )
        {
            return readULLIntHex( s+2, n, 16 );
        }
        else
        {
            return readULLIntB( s+1, n, 8 );
        }
    }
    else
    {
        return readULLInt( s, n );
    }
}
