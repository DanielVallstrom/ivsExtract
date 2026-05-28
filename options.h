// Module for parsing command line options.


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


/*
  options_parseCommandLineOptions  -- Parses the command line options.
*/


#ifndef options_H
#define options_H


#include "ivsExt.h"


// Parses the command line options. esi will be updated according to the
// options. argC and argV work like C's 'main' function arguments.
//   2 is returned if a help, version or print-options flag is encountered.
// 1 is returned iff something went wrong. 0 is returned otherwise.
int options_parseCommandLineOptions( IVSExtInstance * iei,
                                     int argC, char * * argV );


#endif // options_H
