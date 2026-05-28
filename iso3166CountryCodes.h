// For looking up ISO 3166-1 country codes.


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
    daniel.vallstrom@gmail.com.

    All software distributed under the License is provided in the hope
    that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    PURPOSE. See the License for more details.
*/

#ifndef iso3166CountryCodes_H
#define iso3166CountryCodes_H


#include <stdint.h>


// The largest country code.
#define iso3166CountryCodes_Max 915


typedef uint16_t Country;  // Country code. An extension of ISO 3166-1.
                           // 1 means world.


// Returns string with description of a ISO 3166-1 country code.
char * iso3166CountryCodes_codeDescription( unsigned int code );


#endif // iso3166CountryCodes_H
