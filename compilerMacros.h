// Header file defining compiler specific macros.


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


#ifndef compiler_H
#define compiler_H


// Used to indicate that an expression is likely to be true or false. E.g:
// "if ( Unlikely( k == 0 ) ) {...}"
#ifdef __GNUC__
  #define Likely(e)    ( __builtin_expect( !!(e), 1 ) )
  #define Likely1(e)   ( __builtin_expect(   (e), 1 ) )
  #define Unlikely(e)  ( __builtin_expect(   (e), 0 ) )
#else
  #define Likely(e)    (e)
  #define Likely1(e)   (e)
  #define Unlikely(e)  (e)
#endif


// Typically used to indicate that a function parameter isn't used,
// suppressing compiler warnings. For example:
//     int constantEvil( int dummyArg attribute__(unused) ) { return 666; }
#ifdef __GNUC__
  #define attribute__(a)  __attribute__ ((a))
#else
  #define attribute__(a)  
#endif


#endif // compiler_H
