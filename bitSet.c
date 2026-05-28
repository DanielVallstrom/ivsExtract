// Module for handling sets as bits.


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


#include "bitSet.h"

#include <stdlib.h>
#include <string.h>


// Free s.
void bitSet_delete( BitSet s )
{
    free(s);
}


// Create a new empty set.
BitSet bitSet_new( BitSetElementN size )
{
    BitSetElementN nrOfSlots =  size/nrOfBitsInSlot +
                               (size%nrOfBitsInSlot != 0);

    return calloc( nrOfSlots, sizeof(BitSetSlot) );
}


// Resize s.
BitSet bitSet_resize( BitSet s, BitSetElementN oldSize,
                                BitSetElementN newSize )
{
    BitSetElementN nrOfSlots =  newSize/nrOfBitsInSlot +
                               (newSize%nrOfBitsInSlot != 0);
    BitSet t = realloc( s, nrOfSlots * sizeof(BitSetSlot) );

    if ( t == NULL )
    {
        return NULL;
    }

    // Initialize any new space to be empty.
    if ( newSize > oldSize )
    {
        BitSetElementN oldNrOfSlots =  oldSize/nrOfBitsInSlot +
                                      (oldSize%nrOfBitsInSlot != 0);
        memset( t + oldNrOfSlots, 0,
                (nrOfSlots - oldNrOfSlots) * sizeof(BitSetSlot) );
    }

    return t;
}

