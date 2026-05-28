// Module for handling sets of numbers as bits.


/*
    Copyright (C) 2025 Daniel Vallstrom. All rights reserved.

    Unless explicitly acquired and licensed from Licensor under a license
    other than the Reciprocal Public License ("RPL"), the contents of this
    file are subject to the RPL Version 1.1, or subsequent versions as
    allowed by the RPL, and You may not copy or use this file in either
    source code or executable form, except in compliance with the terms
    and conditions of the RPL.

    You should be able to find a copy of the RPL (the "License") in a file
    named LICENSE that should come along with this file; if not, write to
    vallst@gmail.com.

    All software distributed under the License is provided in the hope
    that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    PURPOSE. See the License for more details.
*/


/*
  bitSet_delete      -- Deletes, i.e. frees, a set.
  bitSet_in          -- Tells whether or not a number is in a set.
  bitSet_inAnd       -- Tells whether or not a number and its left neighbor is
                        in a set.
  bitSet_inOr        -- Tells whether or not a number or its left neighbor is
                        in a set.
  bitSet_insert      -- Inserts an element into a set.
  bitSet_insertPair  -- Inserts an element and its left neighbor into a set.
  bitSet_new         -- Creates a new empty set.
  bitSet_remove      -- Removes an element from a set.
  bitSet_removePair  -- Removes an element and its left neighbor from a set.
  bitSet_resize      -- Resizes a set.
*/

// Note that the in general possibly expensive operations / and % should here
// automatically be transformed into fast bit-manipulating operations ---
// typically >> 3 and & 7. Is this really the case everywhere or should one
// make sure of it???


#ifndef bitSet_H
#define bitSet_H


#include <limits.h>

#include "iso3166CountryCodes.h"


// The "word" type for this implementation. char is probably fastest.
typedef unsigned char BitSetSlot;

// A set of numbers is an array of BitSetSlots.
typedef BitSetSlot * BitSet;

// The type of the elements of the sets. The potential elements are 0, 1, ...
// The type should probably be as small as possible. Since the sets are
// intended for country codes, the type should be Country.
typedef Country BitSetElement;

// The type for the number of elements in a set. Should be the same as
// BitSetElement.
typedef BitSetElement BitSetElementN;


// The number of bits in BitSetSlot.
#define nrOfBitsInSlot  ( sizeof(BitSetSlot) * CHAR_BIT )

// The slot number, starting with 0, for a potential element a.
//#if nrOfBitsInSlot == 8
//#define bitSetSlotNr(a)  ( (a) >> 3 )
//#else
#define bitSetSlotNr(a)  ( (a) / nrOfBitsInSlot )
//#endif

// The bit mask for a potential element a.
#define bitMask(a)  ( (BitSetSlot)1  <<  ( (a) % nrOfBitsInSlot ) )
//#define bitMask(a)  ( (BitSetSlot)1  <<  ( (a) & 7 ) )

// The bit mask for a potential element a and its left neighbor.
#define bitMaskPair(a)  ( (BitSetSlot)3  <<  ( (a) % nrOfBitsInSlot ) )


// Returns 0 iff the potential element a is not in the set s.
#define bitSet_in(a,s)  ( (s)[bitSetSlotNr(a)] & bitMask(a) )

// Returns 0 iff the potential elements a and a+1 both are not in the set s.
#define bitSet_inOr(a,s)  ( (s)[bitSetSlotNr(a)] & bitMaskPair(a) )

// Returns 0 iff the potential element a or a+1 is not in the set s.
//   Is this efficient enough??
#define bitSet_inAnd(a,s)  ( ( (s)[bitSetSlotNr(a)] & bitMaskPair(a) )  ==   \
                             bitMaskPair(a) )

// Inserts element a into set s.
#define bitSet_insert(a,s)  ( (s)[bitSetSlotNr(a)] |= bitMask(a) )

// Inserts elements a and a+1 into set s. If a equals 8*nrOfBitsInSlot-1,
// only a will be inserted.
#define bitSet_insertPair(a,s)  ( (s)[bitSetSlotNr(a)] |= bitMaskPair(a) )

// Removes element a from set s.
#define bitSet_remove(a,s)  ( (s)[bitSetSlotNr(a)] &= ~bitMask(a) )

// Removes elements a and a+1 from set s. If a equals 8*nrOfBitsInSlot-1,
// only a will be removed.
#define bitSet_removePair(a,s)  ( (s)[bitSetSlotNr(a)] &= ~bitMaskPair(a) )


// Deletes, i.e. frees, the set s.
void bitSet_delete( BitSet s );

// Returns a new empty set of size 'size'. The largest number that can be an
// element of the new set is size-1. If not enough memory for the new set could
// be allocated, NULL will be returned. If 'size' is 0 the behavior will be 
// like malloc's.
BitSet bitSet_new( BitSetElementN size );

// Deletes, i.e. frees, s and returns a set t of size newSize. oldSize should
// be the size of s. If newSize < oldSize, s and t will agree on numbers less
// than newSize. If newSize >= oldSize, s and t will contain the same elements,
// i.e. the new space in t will be initialized to be empty. If not enough 
// memory for t could be allocated, NULL will be returned and s will not be
// deleted. If newSize or oldSize is 0 the behavior will be like realloc's.
BitSet bitSet_resize( BitSet s, BitSetElementN oldSize,
                                BitSetElementN newSize );


#endif // bitSet_H
