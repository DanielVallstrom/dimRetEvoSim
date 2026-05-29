// Module containing the main-function for dimRetEvoSim.
// dimRetEvoSim simulates evolution under assumptions of
// some diminishing returns to test cooperation.


/*
    Copyright (C) 2022 Daniel Vallstrom. All rights reserved.

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


#include "evoSim.h"
#include "options.h"
#include "common.h"
#include "printEvo.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


// Prints various info.
static void printInfo( EvoSimInstance * esi )
{
    Settings * s = esi->settings;
    Evo      * e = esi->evo;

    printf(
"coop-max-at-start=%u"
"  resp-prop-diff=%d"
"  change-prob=%g"
"  wrong-change-prob=%g"
"  coop-bound=%g"
"  coop-offset-at-start=%u"
"  pop=%u"
"  nr-of-rounds=%llu"
"  print-sim=%s",
(unsigned int)s->coopMaxAtStart,
(signed int)s->respPropDiff,
(double)s->changeProb,
(double)s->wrongChangeProb,
(double)s->coopBound,
(unsigned int)s->coopOffsetAtStart,
(unsigned int)e->pop,
(unsigned long long int)e->nrOfRounds,
(s->verbosityVector & EvoSimVerbosity_printSim) ? "yes" : "no"
          );

    printf(
"  verbosity-vector=%#llx"
"  resp-contri=%g"
"  coop-signal=%u",
(unsigned long long int)s->verbosityVector,
(double)s->respContri,
(unsigned int)s->coopSignal
          );

    putc( '\n', stdout );
}


int main( int argc, char * * argv )
{
    // For measuring cpu time used.
    clock_t clockStart = clock();

    EvoSimInstance * esi = evoSim_newInstance();

    if ( esi == NULL )
    {
        fprintf( stderr, "Error: not enough memory\n" );

        return 1;
    }

    // Used for various calls.
    int result;

    // Read the command line options.
    {
        result = options_parseCommandLineOptions( esi, argc, argv );

        if ( result > 0 )
        {
            return result;
        }
    }

    // Allocate arrays.
    if ( evoSim_allocArrays( esi ) )
    {
        if ( esi->settings->verbosityVector & EvoSimVerbosity_printErrors )
        {
            fprintf( stderr, "Error: not enough memory\n" );
        }

        return 1;
    }

    // Print seed
    if ( esi->settings->verbosityVector & EvoSimVerbosity_printSeed )
    {
        printf( "seed: %lu\n", esi->settings->seed );
    }


    // Print various info.
    if ( esi->settings->verbosityVector & EvoSimVerbosity_printInfo )
    {
        printInfo(esi);
    }


    // Save current time.
    if ( esi->settings->verbosityVector & EvoSimVerbosity_printTime )
    {
	      clockStart = clock();
    }


    // Make an evolution simulation run.
    if ( evoSim_sim(esi) )
    {
        if ( esi->settings->verbosityVector & EvoSimVerbosity_printErrors )
        {
            fprintf( stderr, "Error: not enough memory\n" );
        }

        return 1;
    }


    // Print cpu time used.
    if ( esi->settings->verbosityVector & EvoSimVerbosity_printTime )
    {
        printf( "cpu time used, in seconds: %g\n",
                (double)(clock()-clockStart) / CLOCKS_PER_SEC );
    }


    return 0;
}
