// Module extracting data from csv data.


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
#include "common.h"
#include "compilerMacros.h"
#include "iso3166CountryCodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <float.h>
#include <ctype.h>


// For threads.
//   However, OpenMP could slow down the program a lot, on things
// that should be to some extent embarrassingly parallelizable.
#ifdef OpenMP
//#include <omp.h>
#endif


// Returns a new IVSExtInstance. Some arrays are allocated.
IVSExtInstance * ivsExt_newInstance(void)
{
    IVSExtInstance * iei = malloc( sizeof(IVSExtInstance) );

    if ( iei == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return NULL;
    }


    // Set up the settings part. ----------------------------

    iei->settings = malloc( sizeof(Settings) );

    if ( iei->settings == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return NULL;
    }

    iei->settings->verbosityVector = 7;
    iei->settings->verbosityVector |= IVSExtVerbosity_printNames;

    iei->settings->sep = DefaultSep;
    iei->settings->precision = DefaultPrecision;
    iei->settings->ciz = DefaultCIz;
    iei->settings->maxCountryCode = iso3166CountryCodes_Max;

    iei->settings->csvFile = stdin;
    iei->settings->csvFileName = NULL;

    iei->settings->outFile = stdout;
    iei->settings->outFileName = NULL;

    iei->settings->countryColumn = DefaultCountryColumn;
    iei->settings->waveColumn = DefaultWaveColumn;


    // Set up the ivs part. ------------------------------

    iei->ivs = malloc( sizeof(IVS) );

    if ( iei->ivs == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return NULL;
    }

    iei->ivs->dim = DefaultDim;
    iei->ivs->country = CountryAllSep;
    iei->ivs->mentioned = 0;
    iei->ivs->composite = 0;
    iei->ivs->compositeDim = UndefDim;
    iei->ivs->func = 0;
    iei->ivs->wave = false;
    iei->ivs->countries = NULL;
    iei->ivs->waveN = UndefWave;

    iei->ivs->answers = NULL;
      // calloc( DefaultAnswerSize, sizeof(Rows) );
    iei->ivs->poi = calloc( iei->ivs->dim + iei->ivs->composite,
                            sizeof(Column) );
    iei->ivs->poiSorted = NULL;
      // calloc( iei->ivs->dim, sizeof(Column) );
    iei->ivs->nrOfAns = calloc( iei->ivs->dim + iei->ivs->composite,
                                sizeof(AnswerN) );

    if ( iei->ivs->poi == NULL       ||
         iei->ivs->nrOfAns == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return NULL;
    }


    // Initialize.
    iei->ivs->nrOfAns[0] = 4;
    iei->ivs->nrOfAns[1] = 3;

    iei->ivs->poi[0] = 166;
    iei->ivs->poi[1] = 78;

    //iei->ivs->poiSorted[0] = 78;
    //iei->ivs->poiSorted[1] = 166;


    // Allocated later, elsewhere.
    //iei->ivs->nrOfAns = NULL;
    //iei->ivs->poi = NULL;
    //iei->ivs->poiSorted = NULL;

    return iei;
}



// Comparison function for qsort.
static int comp(const void * a, const void * b) 
{
    Column c = *((Column*)a);
    Column d = *((Column*)b);

    if (c > d) return  1;
    if (c < d) return -1;
    return 0;
}



// Returns true iff code is name of region (or set of countries).
static bool isRegion( Country code )
{
    Country regions[] = { 2, 15, 202, 11, 14, 17, 18, 10, 19, 3, 21, 419,
                          5, 13, 29, 142, 30, 34, 35, 143, 145, 150, 39,
                          151, 154, 155, 9, 53, 54, 57, 61,
                          CountryWorldSet };
    // 830, Channel Islands, isn't included.

    uint16_t regionsSize = sizeof(regions) / sizeof(regions[0]);

    for ( uint16_t n = 0; n != regionsSize; n++ )
    {
        if ( code == regions[n] )
        {
            return true;
        }
    }

    return false;
}



// Inserts n countries in array a into s.
static void addToBitSet( Country * a, unsigned int size, BitSet s )
{
    for ( uint32_t n = 0; n != size; n++ )
    {
        bitSet_insert( a[n], s );
    }
}



// Inserts region with code code into s.
static void setRegion( Country code, BitSet s )
{
    // Region definitions, taken from UN site, and wikipedia:

    Country northernAfrica[] = { 12, 434, 504, 729, 732, 788, 818 };
    Country westernAfrica[] = { 132, 204, 270, 288, 324, 384, 430, 466, 478,
                                562, 566, 624, 654, 686, 694, 768, 854 };
    Country easternAfrica[] = { 86, 108, 174, 175, 231, 232, 260, 262, 404,
                                450, 454, 480, 508, 638, 646, 690, 706, 716,
                                728, 800, 834, 894 };
    Country middleAfrica[] = { 24, 120, 140, 148, 178, 180, 226, 266, 678 };
    Country southernAfrica[] = { 72, 426, 516, 710, 748 };

    Country northernAmerica[] = { 60, 124, 304, 666, 840 };
    Country southAmerica[] = { 32, 68, 74, 76, 152, 170, 218, 238,
                               239, 254, 328, 600, 604, 740, 858, 862 };
    Country centralAmerica[] = { 84, 188, 222, 320, 340, 484, 558, 591 };
    Country caribbean[] = { 28, 44, 52, 92, 136, 192, 212, 214, 308, 312,
                            332, 388, 474, 500, 531, 533, 534, 535, 630, 
                            652, 659, 660, 662, 663, 670, 780, 796, 850 };

    Country easternAsia[] = { 156, 158, 344, 392, 408, 410, 446, 496 };
    Country southernAsia[] = { 4, 50, 64, 144, 356, 364, 462, 524, 586 };
    Country southEasternAsia[] = { 96, 104, 116, 360, 418, 458, 608, 626,
                                   702, 704, 764 };
    Country centralAsia[] = { 398, 417, 762, 795, 860 };
    Country westernAsia[] = { 31, 51, 48, 196, 197, 268, 275, 368, 376, 400, 
                              414, 422, 512, 634, 682, 760, 792, 784, 887 };

    Country southernEurope[] = { 8, 20, 70, 191, 292, 300, 336, 380, 470, 
                                 499, 620, 674, 688, 705, 724, 807, 915 };
    Country easternEurope[] = { 100, 112, 203, 348, 498, 616, 642, 643, 703,
                                804 };
    Country northernEurope[] = { 208, 233, 234, 246, 248, 352, 372, 428, 440, 
                                 578, 744, 752, 826, 831, 832, 833, 909 };
    Country westernEurope[] = { 40, 56, 250, 276, 438, 442, 492, 528, 756 };

    Country australiaAndNZ[] = { 36, 162, 166, 334, 554, 574 };
    Country melanesia[] = { 90, 242, 540, 548, 598 };
    Country micronesia[] = { 296, 316, 520, 580, 581, 583, 584, 585 };
    Country polynesia[] = { 16, 184, 258, 570, 612, 772, 776, 798, 876, 882 };


    // World Bank income groups
    Country highInc[] = { 16, 20, 28, 533, 36, 40, 44, 48, 52, 56, 60, 92,
                          96, 100, 124, 136, 831, 832, 152, 188, 191, 531, 
                          196, 203, 208, 233, 234, 246, 250, 258, 276, 292, 
                          300, 304, 316, 328, 344, 348, 352, 372, 833, 376, 
                          380, 392, 410, 414, 428, 438, 440, 442, 446, 470, 
                          492, 520, 528, 540, 554, 580, 578, 512, 585, 591, 
                          616, 620, 630, 634, 642, 643, 674, 682, 690, 702, 
                          534, 703, 705, 724, 659, 663, 752, 756, 158, 780, 
                          796, 784, 826, 840, 858, 850 };
    Country lowInc[] = { 4, 854, 108, 140, 148, 180, 232, 270, 624, 408, 
                         430, 450, 454, 466, 508, 562, 646, 694, 706, 728, 
                         729, 760, 768, 800, 887};
    Country lowerMidInc[] = { 24, 50, 204, 64, 68, 116, 120, 174, 178, 384, 
                              262, 818, 748, 288, 324, 332, 340, 356, 400, 
                              404, 296, 417, 418, 422, 426, 478, 583, 504, 
                              104, 516, 524, 558, 566, 586, 598, 608, 678, 
                              686, 90, 144, 762, 834, 626, 788, 860, 548, 
                              704, 275, 894, 716};
    Country upperMidInc[] = { 8, 12, 32, 51, 31, 112, 84, 70, 72, 76, 132, 
                              156, 170, 192, 212, 214, 218, 222, 226, 242, 
                              266, 268, 308, 320, 360, 364, 368, 388, 398,
                              434, 458, 462, 584, 480, 484, 498, 496, 499, 
                              807, 600, 604, 882, 688, 710, 662, 670, 740, 
                              764, 776, 792, 795, 798, 804, 915};

    Country mena[] = { 12, 48, 262, 818, 364, 368, 376, 400, 414, 422, 434,
                       470, 504, 512, 275, 634, 682, 760, 788, 784, 887 };


    switch (code)
    {
    case 2:  // Africa
        setRegion( 15, s );
        setRegion( 202, s );

        break;

    case 3:  // North America  (Not used.)
        setRegion( 13, s );
        setRegion( 21, s );
        setRegion( 29, s );

        break;

    case 5:  // South America
        addToBitSet( southAmerica, 
                     sizeof(southAmerica) / sizeof(southAmerica[0]), s );
        break;

    case 9:  // Oceania
        setRegion( 53, s );
        setRegion( 54, s );
        setRegion( 57, s );
        setRegion( 61, s );

        break;

    case 10:  // Antarctica
        bitSet_insert( 10, s );  // ??      
        
        break;

    case 11:  // Western Africa
        addToBitSet( westernAfrica, 
                     sizeof(westernAfrica) / sizeof(westernAfrica[0]), s );
        break;

    case 13:  // Central America
        addToBitSet( centralAmerica, 
                     sizeof(centralAmerica) / sizeof(centralAmerica[0]), s );
        break;

    case 14:  // Eastern Africa
        addToBitSet( easternAfrica, 
                     sizeof(easternAfrica) / sizeof(easternAfrica[0]), s );
        break;

    case 15:  // Northern Africa
        addToBitSet( northernAfrica, 
                     sizeof(northernAfrica) / sizeof(northernAfrica[0]), s );
        break;

    case 17:  // Middle Africa
        addToBitSet( middleAfrica, 
                     sizeof(middleAfrica) / sizeof(middleAfrica[0]), s );
        break;

    case 18:  // Southern Africa
        addToBitSet( southernAfrica, 
                     sizeof(southernAfrica) / sizeof(southernAfrica[0]), s );
        break;

    case 19:  // Americas
        setRegion( 419, s );
        setRegion( 21, s );

        break;

    case 21:  // Northern America
        addToBitSet( northernAmerica, 
                     sizeof(northernAmerica) / sizeof(northernAmerica[0]), s );
        break;

    case 29:  // Caribbean
        addToBitSet( caribbean, 
                     sizeof(caribbean) / sizeof(caribbean[0]), s );
        break;

    case 30:  // Eastern Asia
        addToBitSet( easternAsia, 
                     sizeof(easternAsia) / sizeof(easternAsia[0]), s );
        break;

    case 34:  // Southern Asia
        addToBitSet( southernAsia, 
                     sizeof(southernAsia) / sizeof(southernAsia[0]), s );
        break;

    case 35:  // South-eastern Asia
        addToBitSet( southEasternAsia, 
                     sizeof(southEasternAsia) / sizeof(southEasternAsia[0]), s );
        break;

    case 39:  // Southern Europe
        addToBitSet( southernEurope, 
                     sizeof(southernEurope) / sizeof(southernEurope[0]), s );
        break;

    case 53:  // Australia and NZ
        addToBitSet( australiaAndNZ, 
                     sizeof(australiaAndNZ) / sizeof(australiaAndNZ[0]), s );
        break;

    case 54:  // Melanesia
        addToBitSet( melanesia, 
                     sizeof(melanesia) / sizeof(melanesia[0]), s );
        break;

    case 57:  // Micronesia
        addToBitSet( micronesia, 
                     sizeof(micronesia) / sizeof(micronesia[0]), s );
        break;

    case 61:  // Polynesia
        addToBitSet( polynesia, 
                     sizeof(polynesia) / sizeof(polynesia[0]), s );
        break;

    case 142:  // Asia
        setRegion( 30, s );
        setRegion( 34, s );
        setRegion( 35, s );
        setRegion( 143, s );
        setRegion( 145, s );

        break;

    case 143:  // Central Asia
        addToBitSet( centralAsia, 
                     sizeof(centralAsia) / sizeof(centralAsia[0]), s );
        break;

    case 145:  // Western Asia
        addToBitSet( westernAsia, 
                     sizeof(westernAsia) / sizeof(westernAsia[0]), s );
        break;

    case 150:  // Europe
        setRegion( 39, s );
        setRegion( 151, s );
        setRegion( 154, s );
        setRegion( 155, s );

        break;

    case 151:  // Eastern Europe
        addToBitSet( easternEurope, 
                     sizeof(easternEurope) / sizeof(easternEurope[0]), s );
        break;

    case 154:  // Northern Europe
        addToBitSet( northernEurope, 
                     sizeof(northernEurope) / sizeof(northernEurope[0]), s );
        break;

    case 155:  // Western Europe
        addToBitSet( westernEurope, 
                     sizeof(westernEurope) / sizeof(westernEurope[0]), s );
        break;

    case 202:  // Sub-Saharan Africa
        setRegion( 11, s );
        setRegion( 14, s );
        setRegion( 17, s );
        setRegion( 18, s );

        break;

    case 419:  // Latin America and the Caribbean
        setRegion( 5, s );
        setRegion( 13, s );
        setRegion( 29, s );

        break;

    case CountryWorldSet:  // World, as an explicit set
        setRegion( 2, s );
        setRegion( 9, s );
        setRegion( 10, s );
        setRegion( 19, s );
        setRegion( 142, s );
        setRegion( 150, s );

        break;


    case CountryLowInc:  // World Bank Low Income Countries
        addToBitSet( lowInc, 
                     sizeof(lowInc) / sizeof(lowInc[0]), s );
        break;
        
    case CountryLowerMidInc:  // World Bank Lower-middle Income Countries
        addToBitSet( lowerMidInc, 
                     sizeof(lowerMidInc) / sizeof(lowerMidInc[0]), s );
        break;

    case CountryUpperMidInc:  // World Bank Upper-middle Income Countries
        addToBitSet( upperMidInc, 
                     sizeof(upperMidInc) / sizeof(upperMidInc[0]), s );
        break;

    case CountryHighInc:  // World Bank High Income Countries
        addToBitSet( highInc, 
                     sizeof(highInc) / sizeof(highInc[0]), s );
        break;


    case CountryMENA:  // World Bank MENA
        addToBitSet( mena, 
                     sizeof(mena) / sizeof(mena[0]), s );
        break;

        
    default:
        fprintf( stderr, "\nError: region code not supported: %u\n\n",
                 code );
        break;
    }
}



// Allocates IVS arrays. Including set of countries, if "country" is region.
//   Returns true iff there wasn't enough memory, or if sanity check failed.
//   Also prints info.
//   Also resorts mentioned vector from poi sorted to poiSorted sorted.
bool ivsExt_allocArrays( IVSExtInstance * iei )
{
    IVS * i = iei->ivs;
    // Rows * a = NULL;
    // Column * poi = NULL;
    AnswerN * nrOfAns = i->nrOfAns;
    uint8_t dim = i->dim;
    //uint8_t compo = i->composite;
    uint8_t dimC = dim + i->composite;


    //iei->ivs->poi = calloc( dimC, sizeof(Column) );
    iei->ivs->poiSorted = malloc( dimC * sizeof(Column) );
    //iei->ivs->nrOfAns = calloc( dimC, sizeof(AnswerN) );


    // Handle composite.
    if ( i->composite != 0 )
    {
        // Set nrOfAns for the composite dimension, if possible.
        switch ( i->func )
        {
        case 0:
            //nrOfAns[i->compositeDim] = nrOfAns[dim];
            
            break;
        
        default:
            //nrOfAns[i->compositeDim] = nrOfAns[dim];

            break;
        }

        // Set column for the composite dimension. For convenience.
        i->poi[i->compositeDim] = MaxColumn;
    }


    // Calculate answers size.
    Rows answersSize = 1; 
    for (uint8_t n = 0; n != dim; n++)
    {
        answersSize *= nrOfAns[n];
    }
    
    if ( answersSize == 0 )
    {
        fprintf( stderr, "Error: nrOfAns must be > 0. Set "
                         "with flag -a for each dimension.\n\n");
        return true;
    }

    iei->ivs->answersSize = answersSize;
    iei->ivs->answers = calloc( answersSize, sizeof(Rows) );

    i->poiPos = malloc( sizeof(uint8_t) * dimC );
    i->poiSortedPos = malloc( sizeof(uint8_t) * dimC );

    i->tempMem = malloc( sizeof(uint8_t) * dim );

    if ( iei->ivs->answers == NULL   ||
         iei->ivs->poi == NULL       ||
         iei->ivs->poiSorted == NULL || 
         iei->ivs->nrOfAns == NULL   ||
         i->poiPos == NULL           ||
         i->poiSortedPos == NULL     || 
         i->tempMem == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return true;
    }


    // If country code is region, allocate and set up
    // the set of countries.
    //   Should the region name be saved?? Now it's lost.
    if ( isRegion(i->country) )
    {
        i->countries = bitSet_new(iei->settings->maxCountryCode+1); 

        if ( i->countries == NULL )
        {
            fprintf( stderr, "\nError: not enough memory.\n\n" );
                        
            return true;
        }

        setRegion( i->country, i->countries );

        // Let's at least print the region name before it's lost:
        if ( iei->settings->verbosityVector & IVSExtVerbosity_printInfo )
        {
            printf( "Region of interest: %u  ", i->country );
            printf( "%s", iso3166CountryCodes_codeDescription(i->country) );
        }

        i->country = CountrySet;
    }


    // Sort poi.
    memcpy( i->poiSorted, i->poi, sizeof(Column) * dimC );
    qsort( i->poiSorted, dimC, sizeof(Column), comp );


    // Set up poiPos.
    Column * poi = i->poi;
    Column * poiSorted = i->poiSorted;
    uint8_t * poiPos = i->poiPos;

    for ( uint8_t d = 0; d != dimC; d++ )
    {
        // Find pos of d in poiSorted.
        uint8_t e = 0;
        for  ( ; poi[d] != poiSorted[e]; e++ )
        {
        }
        
        poiPos[d] = e;
    }

    // Set up poiSortedPos.
    uint8_t * poiSortedPos = i->poiSortedPos;
    for ( uint8_t d = 0; d != dimC; d++ )
    {
        // Set value for poiPos[d].
        poiSortedPos[poiPos[d]] = d;
    }

    #if AssertionLevel >= 2
    {
        for ( uint8_t d = 0; d != dimC; d++ )
        {
            assert2( poiPos[poiSortedPos[d]] == d );
        }
    }
    #endif

    // Resort mentioned vector.
    uint64_t oldVector = i->mentioned;
    uint64_t newVector = 0;

    for ( uint8_t d = 0; d != dimC; d++ )
    {
        // Set bit poiPos[d].
        if ( oldVector & ( 1 << d ) )
        {
            newVector |= ( 1 << poiPos[d] ); 
        }
    }

    i->mentioned = newVector;

    return false;
}



// Prints results with non-answers discarded.
static void printExtResult( IVSExtInstance * iei )
{
    IVS * i = iei->ivs;
    Settings * s = iei->settings;
    uint8_t dim = i->dim;
    FILE * outfile = s->outFile;

    // Temporary memory used to store answers viewed as "number".
    uint8_t * a = i->tempMem;
    memset( a, 0, sizeof(uint8_t) * dim );


    fprintf( outfile, "\nResults with non-answers discarded:\n");

    if ( dim == 2 )
    {
        fprintf( outfile, " x y: count  share  " );
    }
    else if ( dim == 3 )
    {
        fprintf( outfile, " x y z: count  share  " );
    }
    else
    {
        fprintf( outfile, " x y z ...: count share " );
    }    

    // Output spaces if floats are long.
    if ( s->precision > 3 )
    {
        for ( uint8_t k = min( s->precision - 3, 61 ); k != 0; k-- )
        {
            fputc( ' ', outfile );
        }
    }

    fprintf( outfile, "count / total" );

    // Print CIs. Only done for proportions, for now --- now done for
    // the general case too, but separately.
    if ( i->nrOfAns[dim-1] == 3 )
    {
        fprintf( outfile, "  +- MoE" );
    }

    fputc( '\n', outfile );


    Rows n = 1;  // Number of results gone through, with 0s not printed.
    Rows * answers = i->answers;
    AnswerN * nrOfAns = i->nrOfAns;

    Rows total;  // Total count for when non-least significant bits are fixed.

    while ( n != i->answersSize )
    {
        // Increment "number", i.e. a.
        uint8_t b = dim;  // The current 'bit' being worked on.
        do
        {
            b--;
            a[b] = ( a[b] + 1 ) % nrOfAns[b];
        } while ( a[b] == 0 );
        
        bool zeros = false;  // true iff a has 0s.

        // Check zeros.
        for ( uint8_t d = 0; d != dim  &&  !zeros; d++)
        {
            zeros = a[d] == 0;
        }

        // Print number and answer if there are no non-answers (0s).
        if (!zeros)
        {
            // If new batch, calculate new totals and CI.
            if ( a[dim-1] == 1 )
            {
                // Calculate new total counts.
                total = 0;
                for ( uint8_t e = 0; e != nrOfAns[dim-1]-1; e++ )
                {
                    total += answers[n+e];
                }


                // Calculate new CI for the general case.
                /* "
                    **CI =  `x̄`  ±  `z` * (`s` / √`n`)**

                    Where:
                    *   `x̄` (pronounced "x-bar") is the **sample mean** (the average of your data points).
                    *   `z` is the **critical value** from the standard normal distribution for your desired confidence level. For a 95% CI, `z` ≈ 1.96.
                    *   `s` is the **sample standard deviation**.
                    *   `n` is the **sample size** (the number of data points).

                    The term `s / √n` is called the **Standard Error of the Mean (SEM)**.

                    The most complex part is calculating the sample standard deviation, `s`. The formula for `s` is:

                    **s = √[ ( Σ(xᵢ - x̄)² ) / (n - 1) ]**

                    Where:
                    *   `Σ` is the summation symbol (sum of...).
                    *   `xᵢ` represents each individual data point.
                    *   `(xᵢ - x̄)²` is the squared difference of each data point from the sample mean.
                    *   `(n - 1)` is the degrees of freedom. Using `n-1` instead of `n` is called Bessel's correction and provides an unbiased estimate of the population variance
                   "
                */
                if ( i->nrOfAns[dim-1] >= 3 )
                {
                    uint32_t sum = 0;  // sample sum

                    for ( uint8_t e = 0; e != nrOfAns[dim-1]-1; e++ )
                    {
                         sum += (e+1) * answers[n+e];
                    }

                    double sampleMean = (double)sum / total;

                    // Calculate the sample standard deviation.

                    double sqDiff = 0;

                    for ( uint8_t e = 0; e != nrOfAns[dim-1]-1; e++ )
                    {
                        double sqDff = (e+1) - sampleMean;
                        sqDff = sqDff * sqDff;
                        sqDiff += answers[n+e] * sqDff;
                    }

                    double sampleStdDev = sqrt( sqDiff / (total-1) );
                    
                    double mOE = s->ciz * ( sampleStdDev / sqrt(total) );


                    // Print "number" without lsb.
                    for ( uint8_t d = 0; d != dim-1; d++ )
                    {
                        fprintf( outfile, " %u", a[d] );
                    }

                    fprintf( outfile, " * CI: " );

                    fprintf( outfile, "%5.*f",
                             s->precision, sampleMean );

                    fprintf( outfile, " +- %5.*f\n",
                             s->precision, mOE );
                }
            }

            // Print "number".
            for ( uint8_t d = 0; d != dim; d++ )
            {
                fprintf( outfile, " %u", a[d] );
            }

            fprintf( outfile, ": %5u  %5.*f  %5u / %4u",
                     answers[n], s->precision, (double)(answers[n])/total, 
                     answers[n], total );

            // Print CIs. Only done for proportions, for now --- now
            // done for the general case too, but earlier.
            // CI = `p` ± 1.96 * √(`p` * (1 - `p`) / `n`)
            if ( i->nrOfAns[dim-1] == 3 )
            {
                double p = (double)(answers[n]) / total;
                double se = sqrt( ( p * ( 1 - p ) ) / total );
                fprintf( outfile, "  %5.*f", s->precision,
                         s->ciz * se );
            }

            fputc( '\n', outfile );
        }

        n++;
    }
}



static void printCountrySet( BitSet countries, Country maxCountryCode )
{
    printf( "Countries in set, investigated together:\n" );

    for ( Country n = 0; n != maxCountryCode+1; n++ )
    {
        if ( bitSet_in( n, countries ) )
        {
            printf( "%u  ", n );
            printf( "%s", iso3166CountryCodes_codeDescription(n) );
        }
    }
}



// Prints iei->ivs.
static void printIVS( IVSExtInstance * iei )
{
    IVS * i = iei->ivs;
    //Rows * ans = i->answers;
    Column * p = i->poi;
    Column * pS = i->poiSorted;
    uint8_t * pPos = i->poiPos;
    uint8_t * poiSortedPos = i->poiSortedPos;
    Country c = i->country;
    uint8_t d = i->dim;
    AnswerN * ansN = i->nrOfAns;

    if ( iei->settings->verbosityVector & IVSExtVerbosity_printMore )
    {
        printf( "Dim: %u\n", d );
    }

    printf( "Country of interest: %u  ", c );
    printf( "%s", iso3166CountryCodes_codeDescription(c) );

    if ( c == CountrySet )
    {
        printCountrySet( i->countries, iei->settings->maxCountryCode );
    }

    if ( iei->settings->verbosityVector & IVSExtVerbosity_printMore )
    {
        if ( i->composite != 0 )
        {
            printf( "number of composite parts: %u\n", i->composite );
            printf( "composite dimension: %u\n", i->compositeDim + 1);
            printf( "composite function: %u\n", i->func );
        }

        if ( i->waveN == UndefWave )
        {
            printf( "All waves are investigated.\n" );
        }
        else
        {
            printf( "Wave of interest: %u\n", i->waveN );
        }

        printf( "answersSize: %u\n", i->answersSize );
        printf( "\"mentioned\" dimension questions, as bit vector, "
                "sorted wrt columns: %llb\n",
                (unsigned long long)i->mentioned );

        for ( uint8_t n = 0; n != d + i->composite; n++ )
        {
            printf( "poi:     %u:%u\n", n+1, p[n] );
            printf( "nrOfAns: %u:%u\n", n+1, ansN[n] );
            #ifndef NDEBUG
            if ( iei->settings->verbosityVector & IVSExtVerbosity_printAll )
            {
                printf( "poiSorted: %u:%u\n", n+1, pS[n] );
                printf( "poiPos: %u:%u\n", n+1, pPos[n] );
                printf( "poiSortedPos: %u:%u\n", n+1, poiSortedPos[n] );
            }

            if ( p[n] == 0 )
            {
                fprintf( stderr, "Error: poi must be > 0. Set "
                                "with flag -p for each dimension.\n\n");
                exit(1);
            }
            #endif
        }
    }
}



// Eats up a line.
static inline void eatLine( FILE * file )
{
    int c;

    do
    {
        c = getc(file);
    }
    while ( c != '\n'  &&  c != EOF );
}


// Eats until character d. Returns true iff d was not reached.
static inline bool eatUntilChar( FILE * file, int d )
{
    int c;

    do
    {
        c = getc(file);
    }
    while ( c != d  &&  c != EOF );

    return c != d;
}


// Eats until character d. Returns last read character.
static inline int eatUntilCharRet( FILE * file, int d )
{
    int c;

    do
    {
        c = getc(file);
    }
    while ( c != d  &&  c != EOF );

    return c;
}


// Eats until characters d or '\n'. Returns last read character.
static inline int eatUntilCharLF( FILE * file, int d )
{
    int c;

    do
    {
        c = getc(file);
    }
    while ( c != d  &&  c != '\n'  &&  c != EOF );

    return c;
}


// Like eatUntilChar but also prints the parsed file content.
static inline bool printUntilChar( FILE * file, int d )
{
    int c = getc(file);

    while ( c != d  &&  c != EOF )
    {
        putchar(c);
        c = getc(file);
    }

    return c != d;
}


// Like eatUntilCharLF but also prints the parsed file content.
int printUntilCharLF( FILE * file, int d )
{
    int c = getc(file);

    while ( c != d  &&  c != '\n'  &&  c != EOF )
    {
        putchar(c);
        c = getc(file);
    }

    return c;
}


// Parses the first line of a csv file.
//   Returns true iff something went wrong.
static bool parseHeader( IVSExtInstance * iei )
{
    IVS * i = iei->ivs;
    Settings * s = iei->settings;

    //Column * p = i->poi;
    Column * pS = i->poiSorted;
    uint8_t dim = i->dim;
    uint8_t composite = i->composite;

    Column cntryCol = s->countryColumn;
    int sep = s->sep;

    FILE * file = s->csvFile;

    Column c = 1;  // The current column.


    // Eat until country column.
    for ( ; c != cntryCol; c++ )
    {
        if ( eatUntilChar( file, sep ) )
        {
            fprintf( stderr, "Error: no country column (%u) "
                             "in csv file.\n\n", cntryCol );
            return true;
        }
    }

    // Handle country column.
    if ( iei->settings->verbosityVector & IVSExtVerbosity_printInfo )
    {
        printf( "Country column: %u ", cntryCol );
        printUntilChar( file, sep );
        putchar('\n');
    }
    else
    {
        eatUntilChar( file, sep );
    }

    c++;


    if ( iei->settings->verbosityVector & IVSExtVerbosity_printInfo )
    {
        putchar('\n');
    }


    // Read pois.
    //   If composites, then the last "pois", MaxColumn, will be skipped.
 
    int chr = EOF;  // Last character read.

    for ( uint8_t d = 0; d != dim+composite-(composite!=0); d++ )
    {
        // Read until current poi.
        while ( c != pS[d] )
        {
            if ( eatUntilChar( file, sep ) )
            {
                fprintf( stderr, "Error: no poi column (%u) "
                                 "in csv file.\n\n", pS[d] );
                return true;
            }

            c++;
        }
        
        // Handle poi column.
        if ( iei->settings->verbosityVector & IVSExtVerbosity_printInfo )
        {
            printf( "poi column: %3u ", c );
            chr = printUntilCharLF( file, sep );
            putchar('\n');
        }
        else
        {
            chr = eatUntilCharLF( file, sep );
        }

        c++;
    }


    if Likely1( chr != '\n' )
    {
        eatLine(file);
    }

    return false;
}


// Reads a number from file 'file' using int c and integer n. The number is
// stored in n. c contains in the end the character after the number. At start
// c must be the first character digit in the number!
#define getNumber( file, c, n )            \
    (n) = (c) - '0';                       \
    (c) = getc(file);                      \
    while ( isdigit( c ) )                 \
    {                                      \
        (n) = 10 * (n) + ( (c) - '0' );    \
        (c) = getc(file);                  \
    }



// Returns a normalized and scaled average of composite answers,
// scaled to range.
//   Non-answers are discarded. 
//   a is the answer array, poi sorted.
static Answer scaledAverage( Answer * a, uint8_t * poiPos,
                             uint8_t dim, uint8_t nrOfArgs, 
                             AnswerN * nrOfAns, AnswerN range )
{
    uint8_t nrOfProperAnswers = 0;  // Number of answers,
                                    // not counting non-answers.

    double x = 0;  // Accumulated normalized sum of proper answers,
                   // and later avg.                             
    // Answers will be normalized to >=0 and <=1.

    for ( uint8_t n = 0; n != nrOfArgs; n++)
    {
        if ( a[poiPos[dim+n]] != 0 )
        {
            nrOfProperAnswers++;

            // Normalize and add to sum.
            x += (double)(a[poiPos[dim+n]]-1) / (nrOfAns[dim+n]-2);
        }
    }

    if ( nrOfProperAnswers == 0 )
    {
        return 0;
    }

    x = x / nrOfProperAnswers;

    // Scale up x: 0 -> 0+0.5; 1 -> (range-1)+0.49999;
    // 1/2 -> 1/2*(range-1)+0.5
    x = x * (range-1);
    x += 0.5;
    x = round(x);

    Answer r = min( range-1, (Answer)x );

    assert(r>0);
    assert(r<range);

    return r;
}



// Returns the scaled average of composite answers, with last
// answer inverted, if it is proper. Scaled to range.
//   Non-answers are discarded. 
//   a is the answer array, poi sorted.
//   The answers, a, should have the same possible answers.
static Answer avgLastInv( Answer * a, uint8_t * poiPos,
                          uint8_t dim, uint8_t nrOfArgs, 
                          AnswerN * nrOfAns, AnswerN range )
{
    uint8_t nrOfProperAnswers = 0;  // Number of answers,
                                    // not counting non-answers.

    Answer x = 0;  // Accumulated sum of proper answers.

    uint8_t n = 0;

    for ( ; n != nrOfArgs-1; n++)
    {
        if ( a[poiPos[dim+n]] != 0 )
        {
            nrOfProperAnswers++;

            x += a[poiPos[dim+n]];
        }
    }

    if ( a[poiPos[dim+n]] != 0 )
    {
        nrOfProperAnswers++;

        x += nrOfAns[dim+n] - a[poiPos[dim+n]];
    }

    if ( nrOfProperAnswers == 0 )
    {
        return 0;
    }

    double y = x - nrOfProperAnswers;
    y = y / nrOfProperAnswers;

    // Scale up (and down) y: 0 -> 0+0.5; 1 -> (range-1)+0.49999;
    // 1/2 -> 1/2*(range-1)+0.5
    y = y * (range-1) / (nrOfAns[dim]-2);
    y += 0.5;
    y = round(y);

    Answer r = min( range-1, (Answer)y );

    assert(r>0);
    assert(r<range);

    return r;
}



// Returns the sum of composite answers, with last
// answer inverted, if it is proper.
//   a is the answer array, poi sorted.
//   Returns 0 if any composite part is 0.
//   Non-answers are discarded. 
static Answer sumLastInv( Answer * a, uint8_t * poiPos,
                          uint8_t dim, uint8_t nrOfArgs, 
                          AnswerN * nrOfAns, AnswerN range )
{
    uint8_t nrOfProperAnswers = 0;  // Number of answers,
                                    // not counting non-answers.

    Answer x = 0;  // Accumulated sum of proper answers.

    uint8_t n = 0;

    for ( ; n != nrOfArgs-1; n++)
    {
        if ( a[poiPos[dim+n]] != 0 )
        {
            nrOfProperAnswers++;

            x += a[poiPos[dim+n]];
        }
        else
        {
            return 0;
        }
    }

    if ( a[poiPos[dim+n]] != 0 )
    {
        nrOfProperAnswers++;

        x += nrOfAns[dim+n] - a[poiPos[dim+n]];
    }
    else
    {
        return 0;
    }

    if ( nrOfProperAnswers == 0 )
    {
        return 0;
    }

    x = min( range-1, x );

    assert(x>0);
    assert(x<range);

    return x;
}



// Parses the body of a csv file.
//   Returns true iff something went wrong.
//   This funtion and parseBodyAllSep are a bit hacky and should 
// be remade.
static bool parseBody( IVSExtInstance * iei )
{
    IVS * i = iei->ivs;
    Settings * s = iei->settings;

    //Column * p = i->poi;
    Column * pS = i->poiSorted;
    uint8_t * pPos = i->poiPos;
    uint8_t * poiSortedPos = i->poiSortedPos;
    uint8_t dim = i->dim;
    AnswerN * nrOfAns = i->nrOfAns;
    uint8_t composite = i->composite;

    Column cntryCol = s->countryColumn;
    int sep = s->sep;

    FILE * file = s->csvFile;

    Rows r = 1;  // Number of lines in csv file, including header.

    Rows rCountry = 0;  // Number of lines with investigated country.

    int chr;

    Wave currentWave = UndefWave;
    Wave oldWave = UndefWave;

    // For saving the answers for a row. [dim+composite]
    //   In sorted column poi order.
    Answer * rowAns = malloc( sizeof(Answer) * ( dim + composite ) );


    if ( rowAns == NULL )
    {
        fprintf( stderr, "Error: not enough memory.\n\n" );

        return true;
    }


    do
    {   // Parse one row.

        Column col = 1;  // The current column.

        uint32_t n;  // Used for reading numbers.

        bool newWave = false;  // True iff the row starts a new wave.

        // Handle wave column, if used.
        //   If EOF comes early, the line will be 
        // treated as white space, ending the file, and not treated
        // as an error.
        if ( i->wave  ||  i->waveN != UndefWave )
        {
            for ( ; col != s->waveColumn; col++ )
            {
                eatUntilChar( file, sep );
            }

            chr = getc(file);


            if ( !isdigit(chr)  &&  chr != EOF )
            {
                fprintf( stderr, "Error: wave code must be unsigned int. "
                                "It was %c on line %u in csv file.\n\n",
                                chr, r );
                return true;
            }

            getNumber( file, chr, n )

            if ( chr != sep  &&  chr != EOF )
            {
                fprintf( stderr, "Error: %c must follow wave code. "
                                "It was %c on line %u in csv file.\n\n",
                                sep, chr, r );
                return true;
            }

            if ( chr != EOF  &&  n != currentWave )
            {
                newWave = true;
                oldWave = currentWave;  // Only used for printing.
                currentWave = n;
            }

            col++;
        }
       

        // Eat until country column.
        //   If EOF comes early, the line will be 
        // treated as white space, ending the file, and not treated
        // as an error.
        for ( ; col != cntryCol; col++ )
        {
            eatUntilChar( file, sep );
        }

        chr = getc(file);

        if ( chr != EOF )
        {
            r++;

            // Handle country column.

            if ( i->country == CountryWorld )
            {
                // Just skip the country column.
                eatUntilChar( file, sep );
            }
            else
            {
                if ( !isdigit(chr) )
                {
                    fprintf( stderr, "Error: country code must be unsigned int. "
                                    "It was %c on line %u in csv file.\n\n",
                                    chr, r );
                    return true;
                }

                getNumber( file, chr, n )

                if ( chr != sep )
                {
                    fprintf( stderr, "Error: %c must follow country code. "
                                    "It was %c on line %u in csv file.\n\n",
                                    sep, chr, r );
                    return true;
                }
            }

            Rows * answers = i->answers;
            uint16_t answersSize = i->answersSize;

            if ( newWave  &&  oldWave != UndefWave  &&  
                 i->waveN == UndefWave )
            {
                // Print results, then start anew.

                if ( oldWave != UndefWave  &&
                     ( iei->settings->verbosityVector &
                       IVSExtVerbosity_printResult ) )
                {
                    //fprintf( s->outFile, "\ncountry: %u  ", 
                    //         currentCountry );
                    //fprintf( s->outFile, "%s",
                    //         iso3166CountryCodes_codeDescription(
                    //             currentCountry ) );

                    if ( i->wave )
                    {
                        fprintf( s->outFile, "wave: %u\n", oldWave );
                    }

                    fprintf( s->outFile, 
                             "rows with country this round: %u\n", rCountry );
                    printExtResult(iei);
                }

                if ( oldWave != UndefWave  &&
                     iei->settings->verbosityVector & IVSExtVerbosity_printAll )
                {
                    printf( " answers data dump, in order:\n" );

                    for ( uint16_t k = 0; k != i->answersSize; k++ )
                    {
                        printf( " %u", answers[k] );
                    }

                    putchar('\n');
                }

                // Reset.
                newWave = false;
                rCountry = 0;
                for ( Rows a = 0; a != answersSize; a++ )
                {
                    answers[a] = 0;
                }
            }


            if ( ( i->country == CountryWorld  ||  n == i->country  ||
                   i->country == CountrySet  &&  
                   bitSet_in( n, i->countries ) )   &&
                 ( i->waveN == UndefWave  ||  i->waveN == currentWave ) )
            {
                rCountry++;

                col++;

                // Read pois.
            
                for ( uint8_t d = 0; d != dim+composite-(composite!=0); d++ )
                {
                    // Read until current poi.
                    while ( col != pS[d] )
                    {
                        if ( eatUntilChar( file, sep ) )
                        {
                            fprintf( stderr, 
                                     "Error: missing poi column (%u) "
                                     "in csv file, line %u.\n\n",
                                     pS[d], r );
                            return true;
                        }

                        col++;
                    }
                    
                    // Handle poi column.

                    chr = getc(file);

                    if ( chr == '-' )
                    {
                        n = 0;
                        chr = eatUntilCharLF( file, sep );
                    }
                    else
                    {
                        if ( !isdigit(chr) )
                        {
                            fprintf( stderr, "Error: answer must be a number. "
                                             "It was %c on line %u in csv file.\n\n",
                                     chr, r );
                            return true;
                        }

                        getNumber( file, chr, n )

                        // Handle "mentioned" questions.
                        if ( (1<<d) & i->mentioned )
                        {
                            n++;
                        }
                        else if ( n >= nrOfAns[poiSortedPos[d]] )
                        {   // Handle large answers.

                            // This is how IVS codes it, inconsistently
                            // multiplying by 100 or 1000.
                            if ( n > i->country * 1000 )
                            {
                                n = n - ( i->country * 1000 );
                            }
                            else if ( n > i->country * 100 )
                            {
                                n = n - ( i->country * 100 );
                            }

                            if ( n >= nrOfAns[poiSortedPos[d]] )
                            {   // Treat answer as non-answer.
                                n = 0;
                            }
                        }

                        if ( chr != sep  &&  chr != '\n'  &&  chr != EOF )
                        {
                            fprintf( stderr, 
                                     "Error: %c, newline or EOF must follow answer."
                                     " It was %c on line %u in csv file.\n\n",
                                     sep, chr, r );
                            return true;
                        }
                    }

                    assert( n < nrOfAns[poiSortedPos[d]] );
                    rowAns[d] = n;

                    col++;
                }


                // Handle composite.
                if ( composite != 0 )
                {
                    assert( dim+composite-1 == 
                            pPos[i->compositeDim] );

                    switch (i->func)
                    {
                    case 0:
                        rowAns[dim+composite-1] = 
                            scaledAverage( rowAns, pPos,
                                           dim, composite, nrOfAns,
                                           nrOfAns[i->compositeDim] );
                        break;
                    
                    case 1:
                        rowAns[dim+composite-1] = 
                            avgLastInv( rowAns, pPos,
                                        dim, composite, nrOfAns,
                                        nrOfAns[i->compositeDim] );
                        break;

                    case 2:
                        rowAns[dim+composite-1] = 
                            sumLastInv( rowAns, pPos,
                                        dim, composite, nrOfAns,
                                        nrOfAns[i->compositeDim] );
                        break;

                    default:
                        rowAns[dim+composite-1] = 
                            scaledAverage( rowAns, pPos,
                                           dim, composite, nrOfAns,
                                           nrOfAns[i->compositeDim] );
                        break;
                    }
                }


                // Increment answers counter.

                for ( uint8_t d = 0; d != dim-1; d++ )
                {
                    answersSize = answersSize / nrOfAns[d];
                    answers += rowAns[pPos[d]] * answersSize;
                }

                assert( answersSize == nrOfAns[dim-1] );
                answers[rowAns[pPos[dim-1]]]++;
                
                if Likely1( chr != '\n' )
                {
                    eatLine(file);
                }
            }
            else
            {
                eatLine(file);
            }
        }
    }
    while ( chr != EOF );


    // Print info.
    if ( iei->settings->verbosityVector & IVSExtVerbosity_printInfo )
    {
        printf( "\nRows: %u\n", r );
        printf( "Rows with investigated country: %u\n", rCountry );

        // Print results with non-answers not discarded.
        //   We'll use rowAns as memory holding the current "number"
        // x y z ...., starting from 0 0 0 ...

        // Initialize.
        for ( uint8_t d = 0; d != dim; d++ )
        {
            rowAns[d] = 0;
        }

        printf( "\nResults with non-answers not discarded:\n");
        printf( " x y z ...: counter\n" );

        Rows n = 0;  // Number of results printed.
        Rows * answers = i->answers;

        // Print first number.
        for ( uint8_t d = 0; d != dim; d++ )
        {
            printf( " %u", rowAns[d] );
        }

        printf( ": %u\n", answers[n] );

        n++;

        while ( n != i->answersSize )
        {
            // Increment "number", i.e. rowAns.
            uint8_t b = dim;  // The current 'bit' being worked on.
            do
            {
                b--;
                rowAns[b] = ( rowAns[b] + 1 ) % nrOfAns[b];
            } while ( rowAns[b] == 0 );
            
            // Print number.
            for ( uint8_t d = 0; d != dim; d++ )
            {
                printf( " %u", rowAns[d] );
            }

            printf( ": %u\n", answers[n] );

            n++;
        }
    }
    

    if ( iei->settings->verbosityVector & IVSExtVerbosity_printAll )
    {
        Rows * answers = i->answers;

        printf( " answers data dump, in order:\n" );

        for ( uint16_t n = 0; n != i->answersSize; n++ )
        {
            printf( " %u", answers[n] );
        }

        putchar('\n');
    }


    free(rowAns);


    return false;
}



// Parses the body of a csv file. 
//   All countries will be processed, separately.
//   Returns true iff something went wrong.
static bool parseBodyAllSep( IVSExtInstance * iei )
{
    IVS * i = iei->ivs;
    Settings * s = iei->settings;

    //Column * p = i->poi;
    Column * pS = i->poiSorted;
    uint8_t * pPos = i->poiPos;
    uint8_t * poiSortedPos = i->poiSortedPos;
    uint8_t dim = i->dim;
    AnswerN * nrOfAns = i->nrOfAns;
    uint8_t composite = i->composite;

    Column cntryCol = s->countryColumn;
    int sep = s->sep;

    FILE * file = s->csvFile;

    Rows r = 1;  // Number of lines in csv file, including header.

    Rows rCountry = 0;  // Number of lines with investigated country.

    int chr;

    Country currentCountry = CountryUndef;
    Wave currentWave = UndefWave;
    Wave oldWave;  // Only used for printing.

    // For saving the answers for a row. [dim+composite]
    Answer * rowAns = malloc( sizeof(Answer) * ( dim + composite ) );


    if ( rowAns == NULL )
    {
        fprintf( stderr, "Error: not enough memory.\n\n" );

        return true;
    }


    do
    {   // Parse one row.

        Column col = 1;  // The current column.

        uint32_t n;  // Used for reading numbers.

        bool newWave = false;  // True iff the row starts a new wave.

        // Handle wave column, if used.
        //   If EOF comes early, the line will be 
        // treated as white space, ending the file, and not treated
        // as an error.
        if ( i->wave  ||  i->waveN != UndefWave )
        {
            for ( ; col != s->waveColumn; col++ )
            {
                eatUntilChar( file, sep );
            }

            chr = getc(file);


            if ( !isdigit(chr)  &&  chr != EOF )
            {
                fprintf( stderr, "Error: wave code must be unsigned int. "
                                "It was %c on line %u in csv file.\n\n",
                                chr, r );
                return true;
            }

            getNumber( file, chr, n )

            if ( chr != sep  &&  chr != EOF )
            {
                fprintf( stderr, "Error: %c must follow wave code. "
                                "It was %c on line %u in csv file.\n\n",
                                sep, chr, r );
                return true;
            }

            if ( chr != EOF  &&  n != currentWave )
            {
                newWave = true;
                oldWave = currentWave;  // Only used for printing.
                currentWave = n;
            }

            col++;
        }


        // Eat until country column.
        //   If EOF comes early, the line will be 
        // treated as white space, ending the file, and not treated
        // as an error.
        for ( ; col != cntryCol; col++ )
        {
            eatUntilChar( file, sep );
        }

        chr = getc(file);

        if ( chr != EOF )
        {
            r++;

            // Handle country column.

            if ( !isdigit(chr) )
            {
                fprintf( stderr, "Error: country code must be unsigned int. "
                                "It was %c on line %u in csv file.\n\n",
                                chr, r );
                return true;
            }

            getNumber( file, chr, n )

            if ( chr != sep )
            {
                fprintf( stderr, "Error: %c must follow country code. "
                                "It was %c on line %u in csv file.\n\n",
                                sep, chr, r );
                return true;
            }

            Rows * answers = i->answers;
            uint16_t answersSize = i->answersSize;

            if ( n != currentCountry  ||  newWave )
            {
                // Print results, then start anew.

                if ( currentCountry != CountryUndef  &&
                     ( iei->settings->verbosityVector &
                       IVSExtVerbosity_printResult ) )
                {
                    fprintf( s->outFile, "\ncountry: %u  ", 
                             currentCountry );
                    fprintf( s->outFile, "%s",
                             iso3166CountryCodes_codeDescription(
                                 currentCountry ) );

                    if ( i->wave )
                    {
                        fprintf( s->outFile, "wave: %u\n", oldWave );
                    }

                    fprintf( s->outFile, 
                             "rows with country this round: %u\n", rCountry );
                    printExtResult(iei);
                }

                if ( currentCountry != CountryUndef  &&
                     iei->settings->verbosityVector & IVSExtVerbosity_printAll )
                {
                    printf( " answers data dump, in order:\n" );

                    for ( uint16_t k = 0; k != i->answersSize; k++ )
                    {
                        printf( " %u", answers[k] );
                    }

                    putchar('\n');
                }

                // Reset.
                newWave = false;
                currentCountry = n;
                rCountry = 0;
                for ( Rows a = 0; a != answersSize; a++ )
                {
                    answers[a] = 0;
                }
            }


            if ( i->waveN == UndefWave  ||  i->waveN == currentWave ) 
            {            
                // Continue with pois.

                rCountry++;

                col++;

                // Read pois.
            
                for ( uint8_t d = 0; d != dim+composite-(composite!=0); d++ )
                {
                    // Read until current poi.
                    while ( col != pS[d] )
                    {
                        if ( eatUntilChar( file, sep ) )
                        {
                            fprintf( stderr, 
                                        "Error: missing poi column (%u) "
                                        "in csv file, line %u.\n\n",
                                        pS[d], r );
                            return true;
                        }

                        col++;
                    }
                    
                    // Handle poi column.

                    chr = getc(file);

                    if ( chr == '-' )
                    {
                        n = 0;
                        chr = eatUntilCharLF( file, sep );
                    }
                    else
                    {
                        if ( !isdigit(chr) )
                        {
                            fprintf( stderr, 
                                    "Error: answer must be a number. "
                                    "It was %c on line %u in csv file.\n\n",
                                    chr, r );
                            return true;
                        }

                        getNumber( file, chr, n )

                        // Handle "mentioned" questions.
                        if ( ( 1 << d ) & i->mentioned )
                        {
                            n++;
                        }
                        else if ( n >= nrOfAns[poiSortedPos[d]] )
                        {   // Handle large answers.

                            // This is how IVS codes it, inconsistently
                            // multiplying by 100 or 1000.
                            if ( n > currentCountry * 1000 )
                            {
                                n = n - ( currentCountry * 1000 );
                            }
                            else if ( n > currentCountry * 100 )
                            {
                                n = n - ( currentCountry * 100 );
                            }

                            if ( n >= nrOfAns[poiSortedPos[d]] )
                            {   // Treat answer as non-answer.
                                n = 0;
                            }
                        }

                        if ( chr != sep  &&  chr != '\n'  &&  chr != EOF )
                        {
                            fprintf( stderr, 
                                    "Error: %c, newline or EOF must follow answer."
                                    " It was %c on line %u in csv file.\n\n",
                                    sep, chr, r );
                            return true;
                        }
                    }

                    assert( n < nrOfAns[poiSortedPos[d]] );
                    rowAns[d] = n;

                    col++;
                }


                // Handle composite.
                if ( composite != 0 )
                {
                    switch (i->func)
                    {
                    case 0:
                        rowAns[dim+composite-1] = 
                            scaledAverage( rowAns, pPos,
                                           dim, composite, nrOfAns,
                                           nrOfAns[i->compositeDim] );
                        break;
                    
                    case 1:
                        rowAns[dim+composite-1] = 
                            avgLastInv( rowAns, pPos,
                                        dim, composite, nrOfAns,
                                        nrOfAns[i->compositeDim] );
                        break;

                    case 2:
                        rowAns[dim+composite-1] = 
                            sumLastInv( rowAns, pPos,
                                        dim, composite, nrOfAns,
                                        nrOfAns[i->compositeDim] );
                        break;

                    default:
                        rowAns[dim+composite-1] = 
                            scaledAverage( rowAns, pPos,
                                           dim, composite, nrOfAns,
                                           nrOfAns[i->compositeDim] );
                        break;
                    }
                }


                // Increment answers counter.

                for ( uint8_t d = 0; d != dim-1; d++ )
                {
                    answersSize = answersSize / nrOfAns[d];
                    answers += rowAns[pPos[d]] * answersSize;
                }

                assert( answersSize == nrOfAns[dim-1] );
                answers[rowAns[pPos[dim-1]]]++;
                
                if Likely1( chr != '\n' )
                {
                    eatLine(file);
                }
            }
            else
            {
                eatLine(file);
            }
        }
    }
    while ( chr != EOF );


    // Print info for last country.
    if ( currentCountry != CountryUndef  &&
         ( iei->settings->verbosityVector &
           IVSExtVerbosity_printResult ) )
    {
        fprintf( s->outFile, "\ncountry: %u  ", 
                    currentCountry );
        fprintf( s->outFile, "%s",
                    iso3166CountryCodes_codeDescription(
                        currentCountry ) );

        if ( i->wave )
        {
            fprintf( s->outFile, "wave: %u\n", currentWave );
        }
                    
        fprintf( s->outFile, "rows with country this round: %u\n", rCountry );
        printExtResult(iei);
    }


    // Print info.

    if ( iei->settings->verbosityVector & IVSExtVerbosity_printInfo )
    {
        printf( "\nTotal rows: %u\n\n", r );
    }

    if ( iei->settings->verbosityVector & IVSExtVerbosity_printMore )
    {
        //printf( "\nTotal rows: %u\n\n", r );
        //printf( "Rows with last investigated country: %u\n", rCountry );

        // Print results with non-answers not discarded.
        //   We'll use rowAns as memory holding the current "number"
        // x y z ...., starting from 0 0 0 ...

        // Initialize.
        for ( uint8_t d = 0; d != dim; d++ )
        {
            rowAns[d] = 0;
        }

        printf( "Results for last country with non-answers not discarded:\n");
        printf( " x y z ...: counter\n" );

        Rows n = 0;  // Number of results printed.
        Rows * answers = i->answers;

        // Print first number.
        for ( uint8_t d = 0; d != dim; d++ )
        {
            printf( " %u", rowAns[d] );
        }

        printf( ": %u\n", answers[n] );

        n++;

        while ( n != i->answersSize )
        {
            // Increment "number", i.e. rowAns.
            uint8_t b = dim;  // The current 'bit' being worked on.
            do
            {
                b--;
                rowAns[b] = ( rowAns[b] + 1 ) % nrOfAns[b];
            } while ( rowAns[b] == 0 );
            
            // Print number.
            for ( uint8_t d = 0; d != dim; d++ )
            {
                printf( " %u", rowAns[d] );
            }

            printf( ": %u\n", answers[n] );

            n++;
        }
    }
    

    if ( iei->settings->verbosityVector & IVSExtVerbosity_printAll )
    {
        Rows * answers = i->answers;

        printf( " answers data dump, in order:\n" );

        for ( uint16_t n = 0; n != i->answersSize; n++ )
        {
            printf( " %u", answers[n] );
        }

        putchar('\n');
    }


    free(rowAns);


    return false;
}



// Extracts csv data.
//   Returns true iff there wasn't enough memory or something went wrong.
bool ivsExt_ext( IVSExtInstance * iei )
{
    Settings * s = iei->settings;


    if ( iei->settings->verbosityVector & IVSExtVerbosity_printInfo )
    {
        printIVS(iei);
    }


    if ( parseHeader(iei) )
    {
        fprintf( stderr, "Error reading first line of csv file %s.\n\n",
                 s->csvFileName );
        return true;
    }


    bool parseResult;

    if ( iei->ivs->country == CountryAllSep )
    {
        parseResult = parseBodyAllSep(iei);
    }
    else
    {
        parseResult = parseBody(iei);
    }

    if (parseResult)
    {
        fprintf( stderr, "Error reading body of csv file %s.\n\n",
                 s->csvFileName );
        return true;
    }


    // Print result.
    if ( iei->ivs->country != CountryAllSep  &&
         ( iei->settings->verbosityVector & IVSExtVerbosity_printResult ) )
    {
        printExtResult(iei);
    }


    return false;
}



// Frees and closes iei.
void ivsExt_free( IVSExtInstance * iei )
{
    Settings * s = iei->settings;
    IVS * i = iei->ivs;


    if ( s->csvFileName != NULL )
    {
        if ( s->csvFile != stdin )
        {
            fclose(s->csvFile);
        }

        free(s->csvFileName);
    }

    if ( s->outFileName != NULL )
    {
        if ( s->outFile != stdout  &&  s->outFile != stderr )
        {
            fclose(s->outFile);
        }

        free(s->outFileName);
    }


    free(i->answers);
    free(i->poi);
    free(i->poiSorted);
    free(i->nrOfAns);
    free(i->poiPos);
    free(i->poiSortedPos);
    free(i->tempMem);
    bitSet_delete(i->countries);

    free(s);
    free(i);
    free(iei);
}

