// Module containing the main-function for ivsExtract.
// ivsExtract extracts data from World Values Survey data, in csv format.


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


#include "ivsExt.h"
#include "options.h"
#include "common.h"
//#include "parse.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>



// Prints various info.
static void printMoreInfo( IVSExtInstance * iei )
{
    Settings * s = iei->settings;
    //IVS      * i = iei->ivs;


    printf(
"  verbosity-vector=%#llx",
(unsigned long long int)s->verbosityVector
          );

    putc( '\n', stdout );
}



int main( int argc, char * * argv )
{
    // For measuring cpu time used.
    clock_t clockStart = clock();

    IVSExtInstance * iei = ivsExt_newInstance();

    if ( iei == NULL )
    {
        fprintf( stderr, "Error: not enough memory\n" );

        return 1;
    }


    // Used for various calls.
    int result;


    // Read the command line options.
    {
        result = options_parseCommandLineOptions( iei, argc, argv );

        if ( result > 0 )
        {
            return result;
        }
    }


    // Allocate arrays. Including set of countries, if "country" is region.
    if ( ivsExt_allocArrays( iei ) )
    {
        if ( iei->settings->verbosityVector & IVSExtVerbosity_printErrors )
        {
            fprintf( stderr, "Error: not enough memory\n" );
        }

        return 1;
    }


    // Print various info.
    if ( iei->settings->verbosityVector & IVSExtVerbosity_printMore )
    {
        printMoreInfo(iei);
    }


    // Save current time.
    if ( iei->settings->verbosityVector & IVSExtVerbosity_printTime )
    {
	      clockStart = clock();
    }


    // Extract data.
    if ( ivsExt_ext(iei) )
    {
        if ( iei->settings->verbosityVector & IVSExtVerbosity_printErrors )
        {
            fprintf( stderr, "Error\n\n" );
        }

        return 1;
    }


    // Print cpu time used.
    if ( iei->settings->verbosityVector & IVSExtVerbosity_printTime )
    {
        printf( "cpu time used, in seconds: %g\n",
                (double)(clock()-clockStart) / CLOCKS_PER_SEC );
    }


    ivsExt_free(iei);


    return 0;
}
