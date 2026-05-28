// Module for handling command line options.


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


#include "options.h"
#include "common.h"
#include "ivsExt.h"

// To do: Switch from getopt to Argp?
#ifndef NoGetopt
#include <getopt.h>
#endif

#include <stdbool.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>


#ifndef ivsExtVersion
#define ivsExtVersion "<no version number available>"
#endif



// Prints the --help message.
static void printHelpMessage( IVSExtInstance * iei )
{
    Settings * s = iei->settings;
    IVS      * i = iei->ivs;

    fprintf(
stdout,
"\nUsage: ivsExtract [options] [csv-file]\n"
"ivsExtract extracts data from a csv file.\n"
"The format of the csv file should follow the IVS format.\n"
"POI columns should contain numbers.\n"
"  If an answer after transformations is still >= <number of different "
"answers>, the answer is treated as a non-answer (0).\n"
"  Example: ./ivsExtract -f EVS_WVS_Joint_Csv_v5_0.csv -F 6 -p 1:166 "
"-p 2:78 -a 1:4 -a 2:3 -o ivs_atheism_trust.result -c 1000\n"
"will extract data for x (1) dimension atheism (-p 1:166) and y (2) "
"dimension trust (-p 2:78).\n"
"  Example: ./ivsExtract -f ZA7503_v3-0-0.csv -F 6 -p 1:431 -p 2:155 "
"-a 1:4 -a 2:3 -c 1000 -v -C 9 -W 8 -w\n"
"  Example: ./ivsExtract -f EVS_WVS_Joint_Csv_v5_0.csv -F 6 -B 3 -D 2 " 
"-p 1:166 -a 1:4 -p 3:52 -p 4:55 -p 5:61 -c 1 "
"-m 3 -m 4 -m 5 -O 1 -v -a 2:7\n"
"will extract data for a composite emancipative measure.\n"
           );

    fprintf(
stdout,
"\nOptions, with defaults in [ ]:\n"
           );

    fprintf(
stdout,
"  -a --answers <unsigned integer>%c<unsigned integer>\n"
"                          Set the number of different answers for a\n"
"                          dimension. In order x (1), y (2), z (3), w_1 (4), ...\n"
"                          E.g., '-a 2%c4' sets the y-dimension to have \n"
"                          4 different answers. Answers <= 0 all count\n"
"                          as the same (non-)answer (0). [1%c%u, 2%c%u]\n"
"                            '-a 5%c4' sets the third composite part to\n"
"                          have 4 different answers, in a 2 dimensional\n"
"                          investigation.\n"
"                            (Re-) define -d or -B before using large\n"
"                          dimensions for -a (or -p).\n"
"                            If dimension is composite, then this sets the\n"
"                          range of the composite function, if applicable.\n",
POISep, POISep,
POISep, (unsigned int)(i->nrOfAns[0]),
POISep, (unsigned int)(i->nrOfAns[1]),
POISep
           );

    fprintf(
stdout,
"  -B --composite <unsigned integer>\n"
"                          Set number of parts used in a\n"
"                          composite measure. E.g. 3 for a\n" 
"                          3 part composite value.\n"
"                          0 if there is no composite value. [%u]\n"
"                            The first poi for a composite has\n"
"                          \"dimension\" maxdim+1, the second has\n"
"                          \"dimension\" maxdim+2, ... \n",
(unsigned int)i->composite
           );

    fprintf(
stdout,
"  -c --country <unsigned integer>\n"
"                          Set country. ISO 3166-1. %u means world.\n"
"                          %u means all countries separately.\n"
"                          %u means world as an explicit set. [%u]\n"
"                          You can also set a region or set of\n"
"                          countries, following UN M49 (which will set -S).\n"
"  -C --country-column <unsigned integer>\n"
"                          Set column with country code. ISO 3166-1.\n"
"                          Must be < POI columns. [%u]\n",
(unsigned int)CountryWorld,
(unsigned int)CountryAllSep,
(unsigned int)CountryWorldSet,
(unsigned int)i->country,
(unsigned int)s->countryColumn
           );

    fprintf(
stdout,
"  -d --dim <unsigned integer>\n"
"                          Set dimension. Set this early if >2. [%u]\n",
(unsigned int)i->dim
           );

    fprintf(
stdout,
"  -D --composite-dim <unsigned integer>\n"
"                          Set dimension that is composite.\n"
           );

    fprintf(
stdout,
"  -f --file <csv-file>    Specify the csv file. - means stdin. [stdin]\n"
           );

    fprintf(
stdout,
"  -h --help               Print this message.\n"
"  -H --print-header       Print header columns.\n"
"  -i --iso-codes          Print ISO 3166-1, UN M49, and World Bank\n"
"                          country and region codes.\n"
"  -L --largest-country-code\n"
"                          Set largest country code in use. (For sets.) [%u]\n"
"  -m --mentioned <unsigned integer>\n"
"                          Set a dimension's question as \"mentioned\".\n"
"                          Implies that the dimension has 3 possible\n"
"                          answers: 0 (non-answers), 1 (not mentioned),\n"
"                          2 (mentioned).\n",
(unsigned int)s->maxCountryCode
           );

    fprintf(
stdout,
"  -o --outfile <file>     Specify an output file. - means stdout. [stdout]\n"
           );

    fprintf(
stdout,
"  -O --func <unsigned integer>\n"
"                          Specify the composite function.\n"
"                            See ivsExt.c for exact function definitions.\n"
"                            The first argument for the funciton will come from\n"
"                          \"dimension\" maxdim+1. The second comes from\n"
"                          \"dimension\" maxdim+2. Third from maxdim+3. ...\n"
"                            The range will be defined by -a for the\n"
"                          composite dimension, where applicable. E.g. for\n"
"                          function \"0\".\n"
"                            0 means a normalized and scaled average;\n"
"                          non-answers are discarded.\n"
"                            1 means scaled average, with last answer inverted\n"
"                          if it is a proper answer. Non-answers are discarded.\n"
"                          The composite parts are assumed to\n"
"                          have the same possible answers.\n"
"                            2 means sum of composite answers, with last\n"
"                          answer inverted. Returns 0 if any composite part is 0.\n"
           );

    fprintf(
stdout,
"  -p --poi <unsigned integer>%c<unsigned integer>\n"
"                          Set one point (column) of interest.\n"
"                          In order x (1), y (2), z (3), w_1 (4), ...\n"
"                          E.g., '-p 2%c55' sets the y-dimension to\n"
"                          column 55. [1%c%u, 2%c%u]\n"
"                            '-p 5%c33' sets the third composite part to\n"
"                          column 33, in a 2 dimensional investigation.\n"
"                            (Re-) define -d or -B before using large\n"
"                          dimensions for -p (or -a).\n"
"  -P --print-info[=no|yes]\n"
"                          Print info if possible. [%s]\n"
"  -F --precision <unsigned integer>\n"
"                          Set precision used when printing floats.\n"
"  --print-options         Print option settings and then quit.\n"
"                          To fine-tune the verbosity, run e.g.\n"
"                          './ivsExtract -v3 --print-options'\n"
"                          and then tune the printed verbosity vector.\n"
"  --print-more[=no|yes]   Print more info. [%s]\n"
"  -q --quiet --silent     Run silently; only print error messages.\n",
POISep, POISep,
POISep, (unsigned int)(i->poi[0]),
POISep, (unsigned int)(i->poi[1]),
POISep,
(s->verbosityVector & IVSExtVerbosity_printInfo) ? "yes" : "no",
(s->verbosityVector & IVSExtVerbosity_printMore) ? "yes" : "no"
           );

    fprintf(
stdout,
"  -S --set-of-countries <list of countries>\n"
"                          Investigate a set of countries, together.\n"
"                          Example: '-S 752,578,208'\n"
           );

    fprintf(
stdout,
"  -s --sep <character>    Set csv separator, e.g. -s ';'. [%c]\n"
"  -v --verbose [level]    Set verbosity level (0-9). No arg means 8. [4]\n"
"  --verbosity-vector <unsigned integer>\n"
"                          Set the verbosity vector. [%#llx]\n"
"                          The integer can be bin (0b), hex (0x),\n"
"                          octal (0), or plain decimal.\n"
"  --version               Print the version number.\n"
"  -w --wave[=no|yes]      Split country batches in waves. [%s]\n"
"  -N --wave-number <unsigned integer>\n"
"                          If set, only this wave will be considered.\n"
"                          Undefined by default, meaning that all waves\n"
"                          are investigated. (Will also set -w.)\n"
"  -W --wave-column <unsigned integer>\n"
"                          Set column with wave code.\n"
"                          Must be < country column. [%u]\n"
"  -z --CIz <float>        Set z-value for CIs. Default is 95%% CIs. [%f]\n",
s->sep,
(unsigned long long int)s->verbosityVector,
(i->wave) ? "yes" : "no",
s->waveColumn,
s->ciz
           );

    fprintf(
stdout,
"\nFor more on what the options mean, see ivsExt.h.\n"
           );

    fprintf(
stdout,
"\nivsExtract is open source licensed under the Reciprocal Public\n"
"License, version 1.1; Copyright (C) 2025 Daniel Vallstrom.\n"
           );

    fprintf(
stdout,
"\nSend bug reports, feedback, etc. to daniel.vallstrom@gmail.com.\n\n"
           );
}



// Sets the verbosity level.
static void setVerbosityLevel( IVSExtInstance * iei, unsigned int vl )
{
    IVSExtVerbosityVector vv = 0;

    switch ( vl )
    {

    // All cases are fall-throughs:

    default:

    case 9: 
        vv |= IVSExtVerbosity_printAll;
    case 8:
    case 7:
        vv |= IVSExtVerbosity_printMore;
    case 6:
        vv |= IVSExtVerbosity_printTime;
    case 5:
        vv |= IVSExtVerbosity_printInfo;
    case 4:
        vv |= IVSExtVerbosity_printNames;
    case 3:
        vv |= IVSExtVerbosity_printResult;
    case 2:
    case 1:
        vv |= IVSExtVerbosity_printErrors;
    case 0:

        break;
    }

    iei->settings->verbosityVector = vv;
}



// Prints option settings.
static void printSettings( IVSExtInstance * iei )
{
    Settings * s = iei->settings;
    //IVS      * i = iei->ivs;

    printf(
"  --print-info=%s",
(s->verbosityVector & IVSExtVerbosity_printInfo) ? "yes" : "no"
          );

    printf(
"  --verbosity-vector=%#llx",
(unsigned long long int) s->verbosityVector
          );

    putc( '\n', stdout );
}



// Converts s into an unsigned int dimension, which is placed in d, and an
// unsigned int column, which is placed in c. Returns true iff a 
// parse error has occurred. Trailing non-number characters in s will result in
// parse error. s must be on form dim:col (or dim<POISep>col).
static bool readPOI( char * s, unsigned int * d, unsigned int * c )
{
    // Contains the accumulated number.
    unsigned int k;

    if ( s == NULL  ||  !isdigit(*s) )
    {
        return true;
    }

    // Read dim.

    k = *s - '0';
    s++;
    while ( isdigit(*s) )
    {
        k = 10 * k + ( *s - '0' );
        s++;
    }

    if ( *s != POISep )
    {
        return true;
    }
        
    *d = k;
    s++;

    if ( s == NULL  ||  !isdigit(*s) )
    {
        return true;
    }

    // Read c.

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
        
    *c = k;

    return false;
}



static void printISO3166CountryCodes(void)
{
    printf( "ISO 3166-1 and UN M49 country and region codes\n"
            "(from Wikipedia and UN), with additions:\n"
"001    World (country codes ignored) \n"        
"002    Africa \n"
"003    North America  (Not used.) \n"
"004 	Afghanistan \n" 	
"005    South America \n"
"008 	Albania  \n"
"009    Oceania \n"
"010    Antarctica 	 \n"
"011    Western Africa \n"
"012 	Algeria 	 \n"
"013    Central America \n"
"014    Eastern Africa \n"
"015    Northern Africa \n"
"016 	American Samoa 	 \n"
"017	Middle Africa \n"
"018    Southern Africa \n"
"019    Americas \n"
"020 	Andorra 	 \n"
"021    Northern America \n"
"024 	Angola 	 \n"
"028 	Antigua and Barbuda \n" 	
"029    Caribbean \n"
"030    Eastern Asia \n"
"031 	Azerbaijan 	Before 1991: part of the USSR \n"
"032 	Argentina 	 \n"
"034    Southern Asia \n"
"035    South-eastern Asia \n"
"036 	Australia 	 \n"
"039    Southern Europe \n"
"040 	Austria 	 \n"
"044 	Bahamas 	 \n"
"048 	Bahrain 	 \n"
"050 	Bangladesh 	 \n"
"051 	Armenia 	Before 1991: part of the USSR \n"
"052 	Barbados 	 \n"
"053    Australia and New Zealand \n"
"054    Melanesia \n"
"056 	Belgium 	 \n"
"057    Micronesia \n"
"060 	Bermuda 	 \n"
"061    Polynesia \n"
"064 	Bhutan 	 \n" );
    printf(
"068 	Bolivia, Plurinational State of \n"	
"070 	Bosnia and Herzegovina 	Before 1992: part of Yugoslavia \n"
"072 	Botswana 	 \n"
"074 	Bouvet Island  \n"	
"076 	Brazil 	 \n"
"084 	Belize 	Formerly British Honduras \n"
"086 	British Indian Ocean Territory 	 \n"
"090 	Solomon Islands 	Formerly British Solomon Islands \n"
"092 	Virgin Islands (British) 	 \n"
"096 	Brunei Darussalam 	 \n"
"100 	Bulgaria 	 \n"
"104 	Myanmar 	Formerly Burma \n"
"108 	Burundi 	 \n"
"112 	Belarus 	Formerly Byelorussian SSR \n"
"116 	Cambodia 	 \n"
"120 	Cameroon 	 \n"
"124 	Canada 	 \n"
"132 	Cabo Verde 	Formerly Cape Verde \n"
"136 	Cayman Islands 	 \n"
"140 	Central African Republic  \n"	
"142    Asia \n"
"143    Central Asia \n"
"144 	Sri Lanka 	Formerly Ceylon \n"
"145    Western Asia \n"
"148 	Chad 	 \n"
"150    Europe \n"
"151    Eastern Europe \n"
"152 	Chile 	 \n"
"154    Northern Europe \n"
"155    Western Europe \n"
"156 	China 	 \n"
"158 	Taiwan \n"
"162 	Christmas Island 	 \n"
"166 	Cocos (Keeling) Islands \n" 	
"170 	Colombia 	 \n"
"174 	Comoros 	 \n"
"175 	Mayotte 	Before 1975: part of Comoros; ISO code assigned in 1993 \n"
"178 	Congo 	 \n"
"180 	Congo, Democratic Republic of the 	 \n"
"184 	Cook Islands 	 \n"
"188 	Costa Rica 	 \n"
"191 	Croatia 	Before 1992: part of Yugoslavia \n"
"192 	Cuba 	 \n" );
    printf( 
"196 	Cyprus \n" 	
"197 	Northern Cyprus \n" 
"202    Sub-Saharan Africa \n"
"203 	Czechia 	Before 1993: part of Czechoslovakia \n"
"204 	Benin 	Formerly Dahomey \n"
"208 	Denmark 	 \n"
"212 	Dominica 	 \n"
"214 	Dominican Republic 	 \n"
"218 	Ecuador 	 \n"
"222 	El Salvador  \n"	
"226 	Equatorial Guinea \n" 	
"231 	Ethiopia 	 \n"
"232 	Eritrea 	Before 1993: part of Ethiopia \n"
"233 	Estonia 	Before 1991: part of the USSR \n"
"234 	Faroe Islands 	Previously spelled as Faeroe Islands \n"
"238 	Falkland Islands (Malvinas) 	 \n"
"239 	South Georgia and the South Sandwich Islands 	Before 1993: part of the Falkland Islands \n"
"242 	Fiji 	 \n"
"246 	Finland  \n"	
"248 	Åland Islands 	Before 2004: included in Finland \n"
"250 	France 	 \n"
"254 	French Guiana \n" 	
"258 	French Polynesia \n" 	
"260 	French Southern Territories \n" 	
"262 	Djibouti 	Formerly French Territory of the Afars and the Issas \n"
"266 	Gabon 	 \n"
"268 	Georgia 	Before 1991: part of the USSR \n"
"270 	Gambia 	 \n"
"275 	Palestine, State of 	Replaced the Gaza Strip, which was assigned code 274 by the United Nations Statistics Division \n"
"276 	Germany 	A unified country since 1990 \n"
"288 	Ghana 	 \n"
"292 	Gibraltar  \n"	
"296 	Kiribati 	Formerly Gilbert and Ellice Islands \n"
"300 	Greece 	 \n" );
    printf( 
"304 	Greenland  \n"	
"308 	Grenada 	 \n"
"312 	Guadeloupe 	 \n"
"316 	Guam 	 \n"
"320 	Guatemala  \n"	
"324 	Guinea 	 \n"
"328 	Guyana 	 \n"
"332 	Haiti 	 \n"
"334 	Heard Island and McDonald Islands \n" 	
"336 	Holy See 	 \n"
"340 	Honduras 	 \n"
"344 	Hong Kong 	 \n"
"348 	Hungary 	 \n"
"352 	Iceland 	 \n"
"356 	India 	 \n"
"360 	Indonesia  \n" );
    printf( 	
"364 	Iran, Islamic Republic of \n" 	
"368 	Iraq 	 \n"
"372 	Ireland  \n"	
"376 	Israel 	 \n"
"380 	Italy 	 \n"
"384 	Côte d'Ivoire 	Formerly Ivory Coast \n"
"388 	Jamaica 	 \n"
"392 	Japan 	 \n"
"398 	Kazakhstan 	Before 1991: part of the USSR \n"
"400 	Jordan 	 \n"
"404 	Kenya 	 \n"
"408 	Korea, Democratic People's Republic of \n" 	
"410 	Korea, Republic of 	 \n"
"414 	Kuwait 	 \n"
"417 	Kyrgyzstan 	Before 1991: part of the USSR \n"
"418 	Lao People's Democratic Republic 	 \n"
"419    Latin America and the Caribbean \n"
"422 	Lebanon 	 \n"
"426 	Lesotho 	 \n"
"428 	Latvia 	Before 1991: part of the USSR \n"
"430 	Liberia 	 \n"
"434 	Libya 	 \n"
"438 	Liechtenstein \n" 	
"440 	Lithuania 	Before 1991: part of the USSR \n"
"442 	Luxembourg 	 \n"
"446 	Macao 	 \n"
"450 	Madagascar \n" 	
"454 	Malawi 	 \n" );
    printf( 
"458 	Malaysia  \n"	
"462 	Maldives 	 \n"
"466 	Mali 	 \n"
"470 	Malta 	 \n"
"474 	Martinique \n" 	
"478 	Mauritania 	 \n"
"480 	Mauritius 	 \n"
"484 	Mexico 	 \n"
"492 	Monaco 	 \n"
"496 	Mongolia  \n"	
"498 	Moldova, Republic of 	Before 1991: part of the USSR \n"
"499 	Montenegro 	Before 2006: part of Yugoslavia/Serbia and Montenegro \n"
"500 	Montserrat 	 \n"
"504 	Morocco 	 \n"
"508 	Mozambique 	 \n"
"512 	Oman 	Formerly Muscat and Oman \n"
"516 	Namibia 	 \n"
"520 	Nauru 	 \n"
"524 	Nepal 	 \n"
"528 	Netherlands, Kingdom of the \n" 	
"531 	Curaçao 	Before 2010: part of the Netherlands Antilles \n"
"533 	Aruba 	Before 1986: part of the Netherlands Antilles \n"
"534 	Sint Maarten (Dutch part) 	Before 2010: part of the Netherlands Antilles \n"
"535 	Bonaire, Sint Eustatius and Saba 	Before 2010: part of the Netherlands Antilles \n"
"540 	New Caledonia 	 \n"
"548 	Vanuatu 	Formerly New Hebrides \n"
"554 	New Zealand 	 \n"
"558 	Nicaragua 	 \n" );
    printf( 
"562 	Niger 	 \n"
"566 	Nigeria  \n"	
"570 	Niue 	 \n"
"574 	Norfolk Island \n" 	
"578 	Norway 	 \n"
"580 	Northern Mariana Islands 	Before 1986: part of Pacific Islands (Trust Territory) \n"
"581 	United States Minor Outlying Islands 	Merger of uninhabited U.S. islands on the Pacific Ocean in 1986 \n"
"583 	Micronesia, Federated States of 	Before 1986: part of Pacific Islands (Trust Territory) \n"
"584 	Marshall Islands \n"
"585 	Palau \n"
"586 	Pakistan \n" 	
"591 	Panama 	 \n"
"598 	Papua New Guinea \n" 	
"600 	Paraguay 	 \n"
"604 	Peru 	 \n"
"608 	Philippines \n" 	
"612 	Pitcairn 	 \n"
"616 	Poland 	 \n"
"620 	Portugal  \n"	
"624 	Guinea-Bissau 	Formerly Portuguese Guinea \n"
"626 	Timor-Leste 	Formerly Portuguese Timor and East Timor \n"
"630 	Puerto Rico 	 \n"
"634 	Qatar 	 \n"
"638 	Réunion  \n"	
"642 	Romania  \n" );
    printf( 	
"643 	Russian Federation 	Before 1991: part of the USSR \n"
"646 	Rwanda 	 \n"
"652 	Saint Barthélemy 	Before 2007: part of Guadeloupe \n"
"654 	Saint Helena, Ascension and Tristan da Cunha 	 \n"
"659 	Saint Kitts and Nevis 	Before 1985: part of Saint Kitts-Nevis-Anguilla \n"
"660 	Anguilla \n"
"662 	Saint Lucia \n" 	
"663 	Saint Martin (French part) 	Before 2007: part of Guadeloupe \n"
"666 	Saint Pierre and Miquelon 	 \n"
"670 	Saint Vincent and the Grenadines \n" 	
"674 	San Marino 	 \n"
"678 	Sao Tome and Principe 	 \n"
"682 	Saudi Arabia 	 \n"
"686 	Senegal 	 \n"
"688 	Serbia 	Before 2006: part of Yugoslavia/Serbia and Montenegro \n"
"690 	Seychelles 	 \n"
"694 	Sierra Leone  \n"	
"702 	Singapore 	 \n"
"703 	Slovakia 	Before 1993: part of Czechoslovakia \n"
"704 	Viet Nam 	Official name: Socialist Republic of Viet Nam \n"
"705 	Slovenia 	Before 1992: part of Yugoslavia \n"
"706 	Somalia 	 \n"
"710 	South Africa  \n"	
"716 	Zimbabwe 	Formerly Southern Rhodesia \n"
"724 	Spain 	 \n"
"728 	South Sudan 	Before 2011: part of Sudan \n"
"729 	Sudan 	 \n" );
    printf( 
"732 	Western Sahara 	Formerly Spanish Sahara \n"
"740 	Suriname 	 \n"
"744 	Svalbard and Jan Mayen 	 \n"
"748 	Eswatini 	Formerly Swaziland \n"
"752 	Sweden 	 \n"
"756 	Switzerland \n" 	
"760 	Syrian Arab Republic \n" 	
"762 	Tajikistan 	Before 1991: part of the USSR \n"
"764 	Thailand 	 \n"
"768 	Togo 	 \n"
"772 	Tokelau  \n"	
"776 	Tonga 	 \n"
"780 	Trinidad and Tobago \n" 	
"784 	United Arab Emirates 	Formerly Trucial States \n"
"788 	Tunisia 	 \n"
"792 	Türkiye 	 \n"
"795 	Turkmenistan 	Before 1991: part of the USSR \n"
"796 	Turks and Caicos Islands 	\n"
"798 	Tuvalu 	\n"
"800 	Uganda 	\n"
"804 	Ukraine 	Before 1991: part of the USSR \n"
"807 	North Macedonia 	Before 1993: part of Yugoslavia \n"
"818 	Egypt 	Formerly United Arab Republic \n"
"826 	United Kingdom of Great Britain and Northern Ireland \n" 	
"831 	Guernsey 	Before 2006: included with the United Kingdom \n"
"832 	Jersey \n"
"833 	Isle of Man \n" );
    printf( 
"834 	Tanzania, United Republic of \n" 	
"840 	United States of America 	 \n"
"850 	Virgin Islands (U.S.) 	 \n"
"854 	Burkina Faso 	Formerly Upper Volta \n"
"858 	Uruguay 	 \n"
"860 	Uzbekistan 	Before 1991: part of the USSR \n"
"862 	Venezuela, Bolivarian Republic of 	 \n"
"876 	Wallis and Futuna 	 \n"
"882 	Samoa 	Formerly Western Samoa \n"
"887 	Yemen 	A unified country since 1990 \n"
"894 	Zambia \n" 
"909    Northern Ireland \n" 
"915    Kosovo \n"
"1000   All countries, seperately \n" );

    printf(
"%u World, as an explicit set (with country codes checked) \n"
"%u (A set of countries, specified elsewhere.)\n", 
CountryWorldSet, CountrySet );

    printf(
"%u World Bank Low Income Countries \n"
"%u World Bank Lower-middle Income Countries \n"
"%u World Bank Upper-middle Income Countries \n"
"%u World Bank High Income Countries \n",
CountryLowInc, CountryLowerMidInc, CountryUpperMidInc, CountryHighInc );

    printf(
"%u World Bank MENA \n",
CountryMENA );

}



// Print the columns in the header in the csv file.
static void printHeader( IVSExtInstance * iei )
{
    Settings * s = iei->settings;
    int sep = s->sep;
    FILE * file = s->csvFile;

    Column c = 1;  // The current column.

    char chr;

    printf( "\nColumns in the header:\n" );

    do
    {
        assert( c == 1  ||  chr == sep );
        printf( "%u: ", c );
        chr = printUntilCharLF( file, sep );
        putchar('\n');
        c++;
    }
    while ( chr != '\n'  &&  chr != EOF );

    putchar('\n');

}




// Reads a list of countries and places them in the set of countries.
//   Returns true iff something went wrong.
//   Any non-digit can separate the countries.
static bool readCountries( IVS * ivs, char * s )
{
    // Contains the accumulated number.
    unsigned int k;

    while ( *s != '\0' )
    {
        // Read country.

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

        // Insert k.
        bitSet_insert( k, ivs->countries );

        if ( *s != '\0' )
        {
            s++;
        }
    }

    return false;
}



// Parse the command line options.
//   Returns 0 iff everything went fine and you should continue on.
// Returns 1 iff there was an error. Returns > 1 for e.g. --help and you
// should stop.
static int parseCommandLineOptions( IVSExtInstance * iei,
                                    int argC, char * * argV )
{
    #ifndef NoGetopt

    // Tells whether or not a csv file has been specified.
    bool csvFileIsSet = false;

    {
        int c;

        int optionIndex;   // Never used.

        static struct option longOptions[] =
        {
            { "composite",              required_argument, NULL, 'B' },
            { "country-column",         required_argument, NULL, 'C' },
            { "composite-dim",          required_argument, NULL, 'D' },
            { "precision",              required_argument, NULL, 'F' },
            { "print-header",           no_argument,       NULL, 'H' },
            { "largest-country-code",   required_argument, NULL, 'L' },
            { "wave-number",            required_argument, NULL, 'N' },
            { "func",                   required_argument, NULL, 'O' },
            { "print-info",             optional_argument, NULL, 'P' },
            { "set-of-countries",       required_argument, NULL, 'S' },
            { "wave-column",            required_argument, NULL, 'W' },
            { "answers",                required_argument, NULL, 'a' },
            { "country",                required_argument, NULL, 'c' },
            { "dim",                    required_argument, NULL, 'd' },
            { "file",                   required_argument, NULL, 'f' },
            { "help",                   no_argument,       NULL, 'h' },
            { "iso-codes",              no_argument,       NULL, 'i' },
            { "mentioned",              required_argument, NULL, 'm' },
            { "outfile",                required_argument, NULL, 'o' },
            { "poi",                    required_argument, NULL, 'p' },
            { "quiet",                  no_argument,       NULL, 'q' },
            { "silent",                 no_argument,       NULL, 'q' },
            { "sep",                    required_argument, NULL, 's' },
            { "verbose",                optional_argument, NULL, 'v' },
            { "wave",                   optional_argument, NULL, 'w' },
            { "CIz",                    required_argument, NULL, 'z' },
            { "version",                no_argument,       NULL, CHAR_MAX+2 },
            { "print-more",             optional_argument, NULL, CHAR_MAX+3 },
            { "print-options",          no_argument,       NULL, CHAR_MAX+4 },
            { "verbosity-vector",       required_argument, NULL, CHAR_MAX+5 },
            { 0, 0, 0, 0 }
        };

        while ( true )
        {
            c = getopt_long( argC, argV,
                             "B:C:D:F:HL:N:O:P::S:W:a:c:d:f:him:o:p:qs:v::w::z:",
                             longOptions, &optionIndex );

            if ( c == -1 )
            {
                break;
            }

            switch ( c )
            {
            case 'B':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -B and --composite\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    // If n > iei->ivs->composite, realloc poi and nrOfAns.
                    if ( n > iei->ivs->composite )
                    {
                        iei->ivs->poi = 
                          realloc( iei->ivs->poi, 
                                   sizeof(Column) * 
                                   ( iei->ivs->dim + n ) );
                        iei->ivs->nrOfAns = 
                          realloc( iei->ivs->nrOfAns, 
                                   sizeof(AnswerN) *
                                   ( iei->ivs->dim + n ) );
                     
                        if ( iei->ivs->poi == NULL  ||  
                             iei->ivs->nrOfAns == NULL )
                        {
                            fprintf( stderr, "\nError: not enough memory.\n" );

                            return 1;
                        }
                    }

                    iei->ivs->composite = n; 
                }

                break;

            case 'C':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -C and --country-column\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->settings->countryColumn = n; 
                }

                break;

            case 'D':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -D and --composite-dim\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->ivs->compositeDim = n-1; 
                }

                break;

            case 'F':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -F and --precision must be\n"
                                 "an unsigned integer."
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->settings->precision = n;
                }

                break;

            case 'H':
                printHeader(iei);

                return 2;

            case 'L':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -L and --largest-country-code\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->settings->maxCountryCode = n; 
                }

                break;

            case 'N':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -N and --wave-number\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->ivs->waveN = n; 
                    iei->ivs->wave = true;
                }

                break;

            case 'O':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -O and --func\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->ivs->func = n; 
                }

                break;

            case 'P':
                if ( optarg )
                {
                    if ( strcmp( optarg, "no" ) == 0 )
                    {
                        iei->settings->verbosityVector &=
                            ~IVSExtVerbosity_printInfo;
                    }
                    else if ( strcmp( optarg, "yes" ) == 0 )
                    {
                        iei->settings->verbosityVector |=
                            IVSExtVerbosity_printInfo;
                    }
                    else
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -P and --print-info must be\n"
                                 "no or yes. You supplied %s.\n\n", optarg );

                        return 1;
                    }
                }
                else
                {
                    iei->settings->verbosityVector |=
                        IVSExtVerbosity_printInfo;
                }

                break;

            case 'S':
                {
                    iei->ivs->country = CountrySet;
                    
                    iei->ivs->countries =
                        bitSet_new( iei->settings->maxCountryCode + 1 ); 

                    if ( iei->ivs->countries == NULL )
                    {
                        fprintf( stderr,
                                 "\nError: not enough memory.\n\n" );
                                 
                        return 1;
                    }

                    if ( readCountries( iei->ivs, optarg ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -S and --set-of-countries\n"
                                 "must be a list of unsigned integers, "
                                 "separated by %c. "
                                 "You supplied %s.\n\n", CountrySep,
                                  optarg );

                        return 1;
                    }
                }

                break;

            case 'W':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -W and --wave-column\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->settings->waveColumn = n; 
                }

                break;

            case 'a':
                {
                    unsigned int d;
                    unsigned int n;

                    if ( readPOI( optarg, &d, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -a and --answers must be\n"
                                 "<unsigned integer>%c<unsigned integer>. "
                                 "You supplied %s.\n\n", POISep, optarg );

                        return 1;
                    }

                    if ( d == 0  ||  d > iei->ivs->dim + iei->ivs->composite  ||
                         n == 0 )
                    {
                        fprintf( stderr,
                                 "\nError: in the argument d%cc to command line "
                                 "option -a and --answers, d must not be 0, "
                                 "d<=dim+composite, and c>0.\n",
                                 POISep );

                        return 1;
                    }

                    iei->ivs->nrOfAns[d-1] = n;
                }

                break;

            case 'c':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -c and --country\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->ivs->country = n; 
                }

                break;

            case 'd':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -d and --dim must be\n"
                                 "an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    // If n > iei->ivs->dim, realloc poi and nrOfAns.
                    if ( n > iei->ivs->dim )
                    {
                        iei->ivs->poi = 
                          realloc( iei->ivs->poi, 
                                   sizeof(Column) * 
                                   ( n + iei->ivs->composite ) );
                        iei->ivs->nrOfAns = 
                          realloc( iei->ivs->nrOfAns, 
                                   sizeof(AnswerN) *
                                   ( n + iei->ivs->composite ) );
                     
                        if ( iei->ivs->poi == NULL  ||  
                             iei->ivs->nrOfAns == NULL )
                        {
                            fprintf( stderr, "\nError: not enough memory.\n" );

                            return 1;
                        }
                    }

                    // Initialize new extension?? There are no good values to
                    // initialize with though. Now it's left to user to set 
                    // it if she extends it.

                    iei->ivs->dim = n;
                }

                break;

            case 'f':
                csvFileIsSet = true;

                if ( strcmp( optarg, "-" ) != 0 )
                {
                    char * name = allocStrCopy(optarg);

                    if ( name == NULL )
                    {
                        fprintf( stderr, "\nError: not enough memory.\n" );

                        return 1;
                    }

                    iei->settings->csvFile = fopen( optarg, "r" );
                    iei->settings->csvFileName = name;

                    if ( iei->settings->csvFile == NULL )
                    {
                        fprintf( stderr, "\nError: file %s couldn't "
                                         "be opened.\n", optarg );
                        perror( NULL );
                        putc( '\n', stderr );

                        return 1;
                    }
                }
                else
                {
                    iei->settings->csvFile = stdin;
                    iei->settings->csvFileName = NULL;
                }

                break;

            case 'h':
                printHelpMessage(iei);

                return 2;

            case 'i':
                printISO3166CountryCodes();

                return 2;

            case 'm':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n )  ||  n == 0 )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -m and --mentioned must be\n"
                                 "an unsigned integer > 0. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->ivs->mentioned |= 1<<(n-1);
                    iei->ivs->nrOfAns[n-1] = 3;
                }

                break;

            case 'o':
                if ( strcmp( optarg, "-" ) != 0 )
                {
                    char * name = allocStrCopy(optarg);

                    if ( name == NULL )
                    {
                        fprintf( stderr, "\nError: not enough memory.\n" );

                        return 1;
                    }

                    iei->settings->outFile = fopen( optarg, "w" );
                    iei->settings->outFileName = name;

                    if ( iei->settings->outFile == NULL )
                    {
                        fprintf( stderr, "\nError: output file %s couldn't "
                                         "be opened.\n", optarg );
                        perror( NULL );
                        putc( '\n', stderr );

                        return 1;
                    }
                }
                else
                {
                    iei->settings->outFile = stdout;
                    iei->settings->outFileName = NULL;
                }

                break;

            case 'p':
                {
                    unsigned int d;
                    unsigned int n;

                    if ( readPOI( optarg, &d, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -p and --poi must be\n"
                                 "<unsigned integer>%c<unsigned integer>. "
                                 "You supplied %s.\n\n", POISep, optarg );

                        return 1;
                    }

                    if ( d == 0  ||  d > iei->ivs->dim + iei->ivs->composite  ||
                         n == 0 )
                    {
                        fprintf( stderr,
                                 "\nError: in the argument d%cc to command line "
                                 "option -p and --poi, d must not be 0, "
                                 "d<=dim+composite, and c>0.\n",
                                 POISep );

                        return 1;
                    }

                    iei->ivs->poi[d-1] = n;
                }

                break;

            case 'q':
                iei->settings->verbosityVector = 0;

                break;

            case 's':
                iei->settings->sep = *optarg; 

                break;

            case 'v':
                if ( optarg )
                {
                    unsigned int vl;

                    if ( readUInt( optarg, &vl ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -v and --verbose must be an\n"
                                 "unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    setVerbosityLevel( iei, vl );
                }
                else
                {
                    //iei->settings->verbosityVector = 0xffffffffffffffff;
                    setVerbosityLevel( iei, 8 );
                }

                break;

            case 'w':
                if ( optarg )
                {
                    if ( strcmp( optarg, "no" ) == 0 )
                    {
                        iei->ivs->wave = false;
                    }
                    else if ( strcmp( optarg, "yes" ) == 0 )
                    {
                        iei->ivs->wave = true;
                    }
                    else
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -w and --wave must be\n"
                                 "no or yes. You supplied %s.\n\n", optarg );

                        return 1;
                    }
                }
                else
                {
                        iei->ivs->wave = true;
                }

                break;


            case 'z':
                {
                    double ciz;

                    if ( readReal( optarg, &ciz ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -z and --CIz\n"
                                 "must be a real on form '1.2', '3.' or '4'. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }
 
                    iei->settings->ciz = ciz;
                }

                break;

                

            case CHAR_MAX+2:
                printf( "ivsExtract %s\n", ivsExtVersion );

                return 2;

            case CHAR_MAX+3:
                if ( optarg )
                {
                    if ( strcmp( optarg, "no" ) == 0 )
                    {
                        iei->settings->verbosityVector &=
                            ~IVSExtVerbosity_printMore;
                    }
                    else if ( strcmp( optarg, "yes" ) == 0 )
                    {
                        iei->settings->verbosityVector |=
                            IVSExtVerbosity_printMore;
                    }
                    else
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "option --print-more must be\n"
                                 "no or yes. You supplied %s.\n\n", optarg );

                        return 1;
                    }
                }
                else
                {
                    iei->settings->verbosityVector |=
                        IVSExtVerbosity_printMore;
                }

                break;

            case CHAR_MAX+4:
                printSettings( iei );

                return 2;

            case CHAR_MAX+5:
                {
                    unsigned long long int vv;

                    if ( readULLIntBase( optarg, &vv ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "option --verbosity-vector must be\n"
                                 "an integer on form '0b1101', '0xd' "
                                 "'015' or '13'. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    iei->settings->verbosityVector = vv;
                }

                break;

            default:
                fprintf( stderr, "\nError: getopt returned %d.\n", c );

                return 1;
            }
        }
    }


    // Handle leftover arguments. If no csv file has been specified, the
    // first leftover argument is assumed to be that file.
    if ( optind < argC )
    {
        if ( !csvFileIsSet )
        {
            if ( strcmp( argV[optind], "-" ) != 0 )
            {
                char * name = allocStrCopy(argV[optind]);

                if ( name == NULL )
                {
                    fprintf( stderr, "\nError: not enough memory.\n" );

                    return 1;
                }

                iei->settings->csvFile = fopen( argV[optind], "r" );
                iei->settings->csvFileName = name;

                if ( iei->settings->csvFile == NULL )
                {
                    fprintf( stderr, "\nError: file %s couldn't "
                                     "be opened.\n", argV[optind] );
                    perror( NULL );
                    putc( '\n', stderr );

                    return 1;
                }
            }
            else
            {
                iei->settings->csvFile = stdin;
                iei->settings->csvFileName = NULL;
            }

            csvFileIsSet = true;

            optind++;
        }

        // Any additional argument is an error.
        if ( optind != argC )
        {
            fprintf( stderr, "\nError: non-option argv elements "
                             "(remember no space between flags, e.g. 'v', "
                             "and optional arguments):\n" );

            for ( ; optind != argC; optind++ )
            {
                fprintf( stderr, "  %s\n", argV[optind]);
            }

            fprintf( stderr, "See ivsExtract --help.\n\n" );

            return 1;
        }
    }


    #endif // #ifndef NoGetopt

    return 0;
}



// Returns 0 iff everything went fine and you should continue on.
// Returns 1 iff there was an error. Returns > 1 for e.g. --help and you
// should stop.
int options_parseCommandLineOptions( IVSExtInstance * iei,
                                     int argC, char * * argV )
{
    int result = parseCommandLineOptions( iei, argC, argV );

    #ifndef NoGetopt

    // Reset getopt so that more calls to options_parseCommandLineOptions
    // can be made. An alternative would be to set optind to 1 at the start
    // of parseCommandLineOptions. Best would be though to dump getopt
    // altogether and write something good instead!!
    optind = 1;  // Is this enough???

    #endif // #ifndef NoGetopt

    return result;
}
