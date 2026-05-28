// Module extracting csv data.


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


#ifndef ivsExt_H
#define ivsExt_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "iso3166CountryCodes.h"
#include "bitSet.h"



// The default column separator. Typically ',', or ';'.
#define DefaultSep (',')

// The default size of the answers array.
#define DefaultAnswerSize (4*3)

// Default dimension.
#define DefaultDim 2

// The default poi separator. Typically ':'.
#define POISep (':')

// The default column with the country code.
#define DefaultCountryColumn (10)

// The default column with the wave code.
#define DefaultWaveColumn (4)


// Country code for the world (all countries), with country codes ignored.
#define CountryWorld (1)

// Country code for all countries treated separately.
#define CountryAllSep (1000)

// Country code for the world (all countries),
// as an explicit set, with country codes checked.
#define CountryWorldSet (1001)

// Country code for a set of countries, defined separately.
#define CountrySet (1002)

// An undefined country.
#define CountryUndef (UINT16_MAX)

// World Bank Low Income Countries.
#define CountryLowInc (1003)

// World Bank Lower-middle Income Countries.
#define CountryLowerMidInc (1004)

// World Bank Upper-middle Income Countries.
#define CountryUpperMidInc (1005)

// World Bank High Income Countries.
#define CountryHighInc (1006)

// World Bank MENA
#define CountryMENA (1007)


// Character separating countries in lists.
//   Not really used; any non-digit can separate countries.
#define CountrySep (',')

// Precision used when printing floats.
#define DefaultPrecision (6)

// An undefined wave.
#define UndefWave (UINT8_MAX)

// The default z-value for CIs.
#define DefaultCIz (1.96)

// An undefined dimension.
#define UndefDim (UINT8_MAX)

// An undefined column that is also max.
#define MaxColumn (UINT16_MAX)



// Types. -------------------------------------------------------------

typedef uint16_t Answer;   // The type of an answer.
// They are typically small but countries can be answers, and
// are coded using ISO 3166-1, which is 3 digits.


typedef Answer AnswerN;   // Number of different answers.

typedef uint16_t ColumnsN;  // Number of questions.

typedef ColumnsN Column;  // csv column.

typedef uint32_t Rows;   // Number of people surveyed. (Rows.)

// Defined in iso3166CountryCodes.h
//typedef uint16_t Country;  // Country code. An extension of ISO 3166-1.
                           // 1 means world.

typedef uint8_t Wave;  // Waves.


// The main structure containing the investigation settings and data.
typedef struct IVS_
{
    // Where it makes sense, dimensions x, y, z, ... will be coded 0, 1, 2, ...
    // Externally, they are coded 1, 2, 3, ...

    Rows * answers;  // Counters for the y (or z, or w_n) answers for each 
                     // x (* y * ...) answer. [nrOfAns_1 * ... * nrOfAns_dim]
                     //   Counters for x=0 will occupy the first 
                     // answersSize/nrOfAns_1 positions. And so on. See
                     // end of body parse function, in two places.

    Column * poi;  // Where answers of interest are. [dim]
                   // In order x, y, z, w_1...
                   //   Size is rather [dim+composite], in case
                   // a dimension is composite. The composite
                   // columns or answers are placed at dim, dim+1, ...
                   // The composite dimension's column will be set to
                   // MaxColumn.

    Column * poiSorted;  // poi sorted. [dim+composite]

    uint8_t * poiPos;  // The position of x, y, ... in poiSorted. [dim]
                       //   Or [dim+composite], rather. The first poi for
                       // a composite has "dimension" dim, the second has
                       // "dimension" dim+1, ... 

    uint8_t * poiSortedPos;  // The position of dimensions in poiSorted in
                             // the dimension list x, y, ...  [dim]
                             //   Or [dim+composite] rather.
    // poiPos[poiSortedPos[d]] == d. Bijections and inverses.

    Country country;  // The country investigated.

    uint8_t dim;  // The number of dimensions.

    AnswerN * nrOfAns;  // The number of different answers for each 
                        // dimension. [dim+composite]
                        // In order x, y, z, w_1...
                        //   Any composite dimension will have the range
                        // of the composite function as value.

    uint16_t answersSize;  // The size of answers array.

    uint8_t * tempMem;  // [dim] Space used temporarily in functions.

    uint64_t mentioned;  // Bit vector. If a dimension d's question is on
                         // the "mentioned" format, its bit will be set 
                         // here. lsb for the first dim in poiSorted, ...
                         //   It will at first, during options reading,
                         // be sorted by dim, i.e. poi!
                         //   "[dim+composite]"

    uint8_t composite;  // The number of parts used in a
                        // composite measure. E.g. 3 for a 
                        // 3 part composite value.
                        // 0 if there is no composite value.

    uint8_t compositeDim;  // The dimension that is a composite.
                           // UndefDim if there is no composite value.

    uint8_t func;  // The composite function. 0 means a normalized and
                   // scaled average.

    bool wave;  // True iff country batches should be split in waves.

    BitSet countries;  // A set of countries to be investigated.

    uint8_t waveN;  // Only do this wave, if != UndefWave.
                    // If set, ->wave will be set to true for convenience.
    
} IVS;



// Each bit in a verbosity vector decides if some particular info should be
// printed when appropriate. See below for what each bit means.
typedef uint32_t IVSExtVerbosityVector;

#define IVSExtVerbosity_printErrors      ( (IVSExtVerbosityVector)1 << 0 )
#define IVSExtVerbosity_printResult      ( (IVSExtVerbosityVector)1 << 1 )
#define IVSExtVerbosity_printInfo        ( (IVSExtVerbosityVector)1 << 2 )
#define IVSExtVerbosity_printMore        ( (IVSExtVerbosityVector)1 << 3 )
#define IVSExtVerbosity_printCountry     ( (IVSExtVerbosityVector)1 << 4 )
#define IVSExtVerbosity_printCountrySet  ( (IVSExtVerbosityVector)1 << 5 )
#define IVSExtVerbosity_printTime        ( (IVSExtVerbosityVector)1 << 6 )
#define IVSExtVerbosity_printAll         ( (IVSExtVerbosityVector)1 << 7 )
#define IVSExtVerbosity_printNames       ( (IVSExtVerbosityVector)1 << 8 )


// Structure containing settings.
typedef struct Settings_
{
    int sep;  // The csv separator.

    // A bit vector where each bit determines if some particular info should
    // be printed. 0 means quiet.
    IVSExtVerbosityVector verbosityVector;

    FILE * csvFile;      // The csv data file.
    char * csvFileName;  // The name of csvFile.

    FILE * outFile;      // The file that the extracted data is written to.
    char * outFileName;  // The name of outFile.

    Column countryColumn;  // The column with the country code.

    Column waveColumn;  // The column with the wave code.

    uint8_t precision;   // The precision used when printing floats.

    double ciz;  // The z-value used for CIs.

    Country maxCountryCode;  // The max country code used. For BitSet.
    
} Settings;



// The main structure containing everything.
typedef struct IVSExtInstance_
{
    IVS * ivs;

    Settings * settings;

} IVSExtInstance;


// Returns a new IVSExtInstance instance.
IVSExtInstance * ivsExt_newInstance(void);


// Allocates arrays. Including set of countries, if "country" is region.
//   Returns true iff there wasn't enough memory.
bool ivsExt_allocArrays( IVSExtInstance * iei );


// Extracts csv data.
//   Returns true iff there wasn't enough memory.
bool ivsExt_ext( IVSExtInstance * iei );


// Frees and closes iei.
void ivsExt_free( IVSExtInstance * iei );


// Eats and prints until characters d or '\n', or EOF.
// Returns last read character, or EOF.
int printUntilCharLF( FILE * file, int d );



#endif // ivsExt_H
