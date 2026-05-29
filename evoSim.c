// Module simulating evolution.


/*
    Copyright (C) 2019 Daniel Vallstrom. All rights reserved.

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
#include "common.h"
#include "compilerMacros.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <float.h>


// For threads.
//   However, OpenMP slows down the program a lot --- 2-3 times on things
// that should be to some extent embarrassingly parallelizable.
#ifdef OpenMP
//#include <omp.h>
#endif


// Returns a new EvoSimInstance, with arrays NULL.
EvoSimInstance * evoSim_newInstance(void)
{
    EvoSimInstance * esi = malloc( sizeof(EvoSimInstance) );

    if ( esi == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return NULL;
    }


    // Set up the settings part. ----------------------------

    esi->settings = malloc( sizeof(Settings) );

    if ( esi->settings == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return NULL;
    }

    esi->settings->verbosityVector = 3;
    esi->settings->verbosityVector |= EvoSimVerbosity_printSeed;

    esi->settings->seed = time(NULL);
    srand(esi->settings->seed);

    esi->settings->coopSignal = DefaultCoopSignal;
    esi->settings->coopMaxAtStart = CoopMaxAtStart;
    esi->settings->coopOffsetAtStart = CoopOffsetAtStart;

    esi->settings->respPropDiff = DefaultRespPropDiff;
    esi->settings->coopBound = DefaultCoopBound;

    esi->settings->changeProb = DefaultChangeProb;
    esi->settings->wrongChangeProb = DefaultWrongChangeProb;

    esi->settings->respContri = DefaultRespContri;

    esi->settings->returns = DefaultReturns;


    // Set up the evo part. ------------------------------

    esi->evo = malloc( sizeof(Evo) );

    if ( esi->evo == NULL )
    {
        fprintf( stderr, "\nError: not enough memory.\n\n" );

        return NULL;
    }

    esi->evo->nrOfRounds = DefaultNrOfRounds;

    esi->evo->pop = evoSim_appropriatePop(esi);


    // Add 1 to population size iff pop and respPropDiff aren't both
    // odd or even.
    if ( ( esi->evo->pop & 1 )  !=  ( esi->settings->respPropDiff & 1 ) )
    {
        esi->evo->pop += 1;
    }


    // Allocated later, elsewhere.
    esi->evo->wealth = NULL;
    esi->evo->coop = NULL;
    esi->evo->coop2 = NULL;
    esi->evo->coopOrd = NULL;
    esi->evo->coopBins = NULL;
    esi->evo->coopPos = NULL;

    return esi;
}


// Prints various coop info.
static void printCoopInfo( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Settings * s = esi->settings;
    //Coop maxC = 0;
    Coop maxCP = 0;  // For proposers.
    //Coop maxCR = 0;  // For responders.
    //Coop minC = Coop_Max;
    Coop minCP = Coop_Max;
    //Coop minCR = Coop_Max;
    //uint64_t sum = 0;
    uint64_t sumP = 0;
    //uint64_t sumR = 0;
    Coop * c = e->coop;
    //Pop pop = e->pop;

    Pop nrOfProposers = (e->pop - s->respPropDiff) / 2;

    Pop a = 0;

    assert2( ( (e->pop - s->respPropDiff) & 1 ) == 0 );

    for (; a != nrOfProposers; a++)
    {
        minCP = min(minCP,c[a]);
        maxCP = max(maxCP,c[a]);
        sumP += c[a];
    }

    fprintf( stdout, "prop coop: min: %u, max: %u, avg: %g;\n", minCP,
             maxCP, (double)sumP/nrOfProposers );

    return;
}


// Prints various coop2 info.
static void printCoop2Info( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Settings * s = esi->settings;
    Coop maxC = 0;
    Coop maxCP = 0;  // For proposers.
    Coop maxCR = 0;  // For responders.
    Coop minC = Coop_Max;
    Coop minCP = Coop_Max;
    Coop minCR = Coop_Max;
    uint64_t sum = 0;
    uint64_t sumP = 0;
    uint64_t sumR = 0;
    Coop * c = e->coop2;
    Pop pop = e->pop;

    Pop nrOfProposers = (e->pop - s->respPropDiff) / 2;

    Pop a = 0;

    assert2( ( (e->pop - s->respPropDiff) & 1 ) == 0 );

    for (; a != nrOfProposers; a++)
    {
        minCP = min(minCP,c[a]);
        maxCP = max(maxCP,c[a]);
        sumP += c[a];
    }

    fprintf( stdout, "prop coop2: min: %u, max: %u, avg: %g;  ", minCP,
             maxCP, (double)sumP/nrOfProposers );


    minC = minCP;
    maxC = maxCP;
    sum = sumP;

    for (; a != pop; a++)
    {
        minCR = min(minCR,c[a]);
        maxCR = max(maxCR,c[a]);
        sumR += c[a];

        minC = min(minC,c[a]);
        maxC = max(maxC,c[a]);
        sum += c[a];
    }

    fprintf( stdout, "resp coop2: min: %u, max: %u, avg: %g;  ", minCR,
             maxCR, (double)sumR/(pop-nrOfProposers) );

    //fprintf( stdout, "all coop: min: %u, max: %u, avg: %g\n", minC, maxC,
    //         (double)sum/pop );
    fprintf( stdout, "all coop2: avg: %g\n", (double)sum/pop );


    if ( randomN(5) == 0 )
    {
        fprintf( stdout, "ordered coop2 bins: " );
        for ( uint32_t b = 0; b != 2*Coop_Max + 2; b += 2 )
        {
            if ( e->coopBins[b] != UndefPop )
            {
                fprintf( stdout, "%u ", e->coopBins[b] );
            }
        }
        fputc( '\n', stdout );
    }


    return;
}


// Allocates evolution simulation arrays.
//   Returns true iff there wasn't enough memory.
bool evoSim_allocArrays( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Wealth * w = NULL;
    Coop * c = NULL;
    Coop * c2 = NULL;
    Coop coopMax = esi->settings->coopMaxAtStart;
    Coop coopOffset = esi->settings->coopOffsetAtStart;
    Pop popSize = e->pop;

    Pop * cOrd = NULL;
    Pop * cBins = NULL;
    Pop * cPos = NULL;

    Pop nrOfProposers = (e->pop - esi->settings->respPropDiff) / 2;
    assert2( ( (e->pop - esi->settings->respPropDiff) & 1 ) == 0 );


    // Allocate arrays. They will be set up next.
    w = malloc( sizeof(Wealth) * popSize );
    //c = malloc( sizeof(Coop) * popSize );
    c = malloc( sizeof(Coop) * nrOfProposers );
    c2 = malloc( sizeof(Coop) * popSize );

    cOrd = malloc( sizeof(Pop) * popSize );
    cBins = malloc( sizeof(Pop) * 2*(Coop_Max+1) );
    cPos = malloc( sizeof(Pop) * popSize );

    if ( w == NULL  ||  c == NULL  ||  c2 == NULL ||  cOrd == NULL  ||
         cBins == NULL ||  cPos == NULL )
    {
        return true;
    }

    // Initialize c.
    for (Pop a = 0; a != nrOfProposers; a++)
    {
        c[a] = coopOffset + a * coopMax / nrOfProposers;  // ??
    }                        // coopMax is a strict upper bound now.


    // Initialize w and c2.
    for (Pop a = 0; a != popSize; a++)
    {
        w[a] = 10;  // ? Not used.
        c2[a] = coopOffset + a * coopMax / popSize;  // ??
    }                         // coopMax is a strict upper bound now.


    // Initialize cOrd and cPos.
    {
        Pop b = popSize;

        for (Pop a = 0; a != popSize; a++)
        {
            b--;
            cOrd[a] = b;
            cPos[b] = a;
        }
    }


    e->wealth = w;
    e->coop = c;
    e->coop2 = c2;
    e->coopOrd = cOrd;
    e->coopBins = cBins;
    e->coopPos = cPos;


    // Initialize cBins.
    {
        for ( uint32_t b = 0; b != 2*Coop_Max + 2; b += 2 )
        {
            cBins[b]   = UndefPop;
            cBins[b+1] = UndefPop;
        }

        Pop a = 0;
        Coop curr;  // The current c2 value being handled.
        do
        {
            curr = c2[cOrd[a]];
            cBins[2*curr] = a;

            // Find next spot that differs.
            for ( ; ( a != popSize )  &&  ( c2[cOrd[a]] == curr ); a++ )
            {
            }

            cBins[2*curr+1] = a-1;
        }
        while ( a != popSize );
    }


    if ( esi->settings->verbosityVector & EvoSimVerbosity_printInfo )
    {
        printCoopInfo(esi);
        printCoop2Info(esi);
    }


    return false;
}


// Returns an appropriate population size.
Pop evoSim_appropriatePop( EvoSimInstance * esi )
{
    return max( DefaultPop - 0 * (esi->evo->nrOfRounds/100), 2 );  // ??
}


// Checks that the coop2 order arrays are consistent.
static bool ordCheck( Evo * e )
{
    Coop * c2 = e->coop2;
    Pop * cOrd = e->coopOrd;
    Pop * cBins = e->coopBins;
    Pop * cPos = e->coopPos;
    Pop popSize = e->pop;

    Pop a = 0;
    //Pop fst;    // The first entity in bin.
    Coop curr;  // The current c2 value being handled.
    do
    {
        curr = c2[cOrd[a]];
        //fst = a;
        assert( cBins[2*curr] == a );

        // Find next spot that differs.
        for ( ; ( a != popSize )  &&  ( c2[cOrd[a]] == curr ); a++ )
        {
            assert( cPos[cOrd[a]] == a );
        }

        assert( cBins[2*curr+1] == a-1 );
    }
    while ( a != popSize );


    Coop cv = Coop_Max;
    do
    {
        cv++;
        if ( cBins[2*cv+1] == UndefPop )
        {
            assert( cBins[2*cv] == UndefPop );
        }
        else
        {
            assert( cBins[2*cv] <= cBins[2*cv+1] );
            assert( c2[cOrd[cBins[2*cv]]] == c2[cOrd[cBins[2*cv+1]]] );
        }
    }
    while( cv != Coop_Max );


    for ( a = 0; a != popSize; a++ )
    {
        assert( cOrd[cPos[a]] == a );
    }


    return true;
}


// Do a round of material cooperation.
//   The proposer gaining the most is returned.
static Pop coopMat( EvoSimInstance * esi)
{
    Settings * s = esi->settings;
    Evo      * e = esi->evo;


    if Likely1( s->respPropDiff >= 0 )
    {
        // All proposals will be accepted, except 0s.
        Pop p = 0;  // Proposer.
        Pop r = e->pop - 1;  // Responder.
        Pop nrOfProposers = (e->pop - s->respPropDiff) / 2;
        Wealth * w = e->wealth;
        Coop * c = e->coop;
        Wealth rMax = 0;
        Wealth pMax = 0;
        Pop eRMax = r;  // sic, default in case 0 is max
        Pop ePMax = p;  // sic, default in case 0 is max
        assert2( ( (e->pop - s->respPropDiff) & 1 ) == 0 );

        while ( p != nrOfProposers )
        {
            // Calculate gain.
            // Don't cooperate if gain is 0.(?) Check that already at mut?
            if Unlikely( ( c[p] == 0 )  ||  ( c[p] == Coop_Max ) )
            {
                w[p] = 0;
                p++;
            }
            else
            {
                w[r] = c[p];  //c[p]/Coop_Max;
                w[p] = Coop_Max - w[r];  //1 - w[r];  // switch to float?
                // now it's stored as n [/Coop_Max]

                // Update max values.

                if Unlikely( w[r] > rMax )
                {
                  eRMax = r;
                  rMax = w[r];
                }

                if Unlikely( w[p] > pMax )
                {
                  ePMax = p;
                  pMax = w[p];
                }

                p++;
                r--;
            }
        }

        // Handle leftover responders.
        for ( Pop respEnd = nrOfProposers-1; r != respEnd; r--)
        {
            w[r] = 0;
        }

        return ePMax;
    }
    else // s->respPropDiff < 0
    {

    }
}


// Returns some 2-logarithm ceiling of v+1, minus 1 when v equals
// 2^k-1. gcc has some fast built-in function, but it shouldn't
// matter much which one is used.
#ifdef __GNUC__
#define log2C(v) ( (uint8_t)( sizeof(unsigned int) * 8 -     \
                              __builtin_clz( (v) + 1 ) -     \
                              ( ( ( (v) + 1 ) & (v) ) == 0 ) ) )
#else
static uint8_t log2C( Wealth v )
{
    uint8_t n = 0;

    for ( ; v != 0; v >>= 1 )
    {
        n++;
    }

    return n;
}
#endif


// Reduce or convert material wealth to a type of overall wealth.
//   Some diminishing returns reduction is used.
static void dimRet( EvoSimInstance * esi )
{
    //Settings * s = esi->settings;
    Evo * e = esi->evo;
    Pop popSize = e->pop;
    Wealth * w = e->wealth;

    for ( Pop a = 0; a != popSize; a++ )
    {
        w[a] = 2*log2C(w[a]);  // ??
    }
}


// Reduce or convert material wealth to a type of overall wealth.
//   sqrt returns reduction is used.
static void sqrtRet( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Pop popSize = e->pop;
    Wealth * w = e->wealth;

    for ( Pop a = 0; a != popSize; a++ )
    {
        w[a] = round( sqrt(w[a]) );
    }
}


// Reduce or convert material wealth to a type of overall wealth.
//   cbrt returns reduction is used.
static void cbrtRet( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Pop popSize = e->pop;
    Wealth * w = e->wealth;

    for ( Pop a = 0; a != popSize; a++ )
    {
        w[a] = round( cbrt(w[a]) );
    }
}


// Reduce or convert material wealth to a type of overall wealth.
//   esi->settings->returns-th root returns reduction is used.
static void rootRet( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Pop popSize = e->pop;
    Wealth * w = e->wealth;
    double r = 1.0 / esi->settings->returns;

    for ( Pop a = 0; a != popSize; a++ )
    {
        w[a] = round( pow( w[a], r ) );  // The rth root of w[a].
    }
}


static void returns( EvoSimInstance * esi )
{
    uint32_t r = esi->settings->returns;

    switch (r)
    {
    case 0:
        dimRet(esi);
        break;

    case 1:
        break;

    case 2:
        sqrtRet(esi);
        break;

    case 3:
        cbrtRet(esi);
        break;

    default:
        rootRet(esi);
        break;
    }
}


// Do a second cycle of cooperation.
//   The population will cooperate pairwise. Top proposer will be paired
// with top leftover cooperator. And so on.
static void coop2( EvoSimInstance * esi )
{
    Settings * s = esi->settings;
    Evo  * e = esi->evo;
    Coop * c2 = e->coop2;
    Pop popSize = e->pop;
    Wealth * w = e->wealth;
    Pop * coop2Ord = e->coopOrd;
    Pop halfPopSize = e->pop/2;

    assert( ( popSize % 2 ) == 0 );

    for ( Pop a = 0; a != halfPopSize; a+=1 )
    {
        Pop p = coop2Ord[a];
        Pop r = coop2Ord[a+halfPopSize];

        assert( c2[p] >= c2[r] );

        // Calculate gain.

        // Don't cooperate if offer is 0.(?)
        if Unlikely( ( c2[p] == 0 ) )
        {
            assert( c2[coop2Ord[a+1]] == 0 );
            return;  // The rest will be 0 too.
        }

        w[r] += c2[p];
        w[p] += Coop_Max - c2[p];

        // Add some cooperation contribution from r to p too.
        w[p] += c2[r] * s->respContri;  // ??
    }

    return;
}


// Decrease coop2 for a. Update coopOrd, coopPos and coopBins too.
static void decCoop2( Evo * e, Pop a )
{
    Coop * c2 = e->coop2;
    Pop * cOrd = e->coopOrd;
    Pop * cBins = e->coopBins;
    Pop * cPos = e->coopPos;

    // Switch a to last pos in its bin.
    assert( cOrd[cPos[a]] == a );
    Pop lastEntCOrdPos = cBins[2*c2[a]+1];
    Pop lastEnt = cOrd[lastEntCOrdPos];  // The last entity in bin.
    assert( c2[a] == c2[lastEnt] );

    if ( a != lastEnt )
    {
        cOrd[cPos[a]] = lastEnt;
        cPos[lastEnt] = cPos[a];
        cPos[a] = lastEntCOrdPos;
        cOrd[lastEntCOrdPos] = a;

        cBins[2*c2[a]+1]--;
    }
    else if ( cBins[2*c2[a]] != lastEntCOrdPos )
    {
        cBins[2*c2[a]+1]--;
    }
    else
    {
        assert( cBins[2*c2[a]] == cBins[2*c2[a]+1] );
        cBins[2*c2[a]]   = UndefPop;
        cBins[2*c2[a]+1] = UndefPop;
    }

    // Make the decrement.

    c2[a]--;

    if Likely1( cBins[2*c2[a]] != UndefPop )
    {
        cBins[2*c2[a]]--;
    }
    else
    {
        cBins[2*c2[a]]   = lastEntCOrdPos;
        cBins[2*c2[a]+1] = lastEntCOrdPos;
    }
}


// Increase coop2 for a. Update coopOrd, coopPos and coopBins too.
static void incCoop2( Evo * e, Pop a )
{
    Coop * c2 = e->coop2;
    Pop * cOrd = e->coopOrd;
    Pop * cBins = e->coopBins;
    Pop * cPos = e->coopPos;

    // Switch a to first pos in its bin.
    assert( cOrd[cPos[a]] == a );
    Pop fstEntCOrdPos = cBins[2*c2[a]];
    Pop fstEnt = cOrd[fstEntCOrdPos];  // The first entity in bin.
    assert( c2[a] == c2[fstEnt] );

    if ( a != fstEnt )
    {
        cOrd[cPos[a]] = fstEnt;
        cPos[fstEnt] = cPos[a];
        cPos[a] = fstEntCOrdPos;
        cOrd[fstEntCOrdPos] = a;

        cBins[2*c2[a]]++;
    }
    else if ( cBins[2*c2[a]+1] != fstEntCOrdPos )
    {
        cBins[2*c2[a]]++;
    }
    else
    {
        assert( cBins[2*c2[a]] == cBins[2*c2[a]+1] );
        cBins[2*c2[a]]   = UndefPop;
        cBins[2*c2[a]+1] = UndefPop;
    }

    // Make the increment.

    c2[a]++;

    if Likely1( cBins[2*c2[a]] != UndefPop )
    {
        cBins[2*c2[a]+1]++;
    }
    else
    {
        cBins[2*c2[a]]   = fstEntCOrdPos;
        cBins[2*c2[a]+1] = fstEntCOrdPos;
    }
}


// Rein in coop[a] if it's outside acceptable range w.r.t. coop2.
static void checkCoopRange( EvoSimInstance * esi, Pop a )
{
    Settings * s = esi->settings;
    Evo      * e = esi->evo;

    Coop * c  = e->coop;
    Coop * c2 = e->coop2;

    assert( a < (e->pop - s->respPropDiff) / 2 );  // nrOfProposers

    if ( ( c[a] < c2[a] )  &&  ( s->coopBound * c[a] < c2[a] ) )
    {
        c[a] = (Coop) min( Coop_Max, ceil( c2[a] / s->coopBound ) );
    }

    assert( ( c[a] >= c2[a] )  ||  ( s->coopBound * c[a] >= c2[a] ) );
}


// Check that coop is within bounds.
static bool rangeCheck( EvoSimInstance * esi )
{
    Settings * s = esi->settings;
    Evo      * e = esi->evo;

    Coop * c  = e->coop;
    Coop * c2 = e->coop2;

    Pop nrOfProposers = ( e->pop - s->respPropDiff ) / 2;
    double coopBound = s->coopBound;

    for ( Pop a = 0; a != nrOfProposers; a++ )
    {
        if (!( ( c[a] >= c2[a] )  ||  ( coopBound * c[a] >= c2[a] ) ))
        {
            printf( "a: %u, c[a]: %u, c2[a]: %u \n", a, c[a], c2[a] );

            return false;
        }

        assert( ( c[a] >= c2[a] )  ||  ( coopBound * c[a] >= c2[a] ) );
    }

    return true;
}


// Evolve.
static void evolve( EvoSimInstance * esi )
{
    Settings * s = esi->settings;
    Evo      * e = esi->evo;

    Pop popSize = e->pop;

    //Pop p = 0;  // Proposer.
    //Pop r = e->pop - 1;  // Responder.
    Pop a;
    Pop nrOfProposers = (e->pop - s->respPropDiff) / 2;
    Wealth * w = e->wealth;
    //Coop * c = e->coop;
    Wealth rMax = 0;
    Wealth pMax = 0;
    Pop eRMax = 0;  // sic, default in case 0 is max
    Pop ePMax = popSize-1;  // sic, default in case 0 is max
    assert2( ( (e->pop - s->respPropDiff) & 1 ) == 0 );


    // Find max values.

    // Handle proposers.
    for ( a = 0; a != nrOfProposers; a++ )
    {
        if Unlikely( w[a] > pMax )
        {
            ePMax = a;
            pMax = w[a];
        }
    }

    // Handle responders.
    for ( ; a != popSize; a++ )
    {
        if Unlikely( w[a] > rMax )
        {
            eRMax = a;
            rMax = w[a];
        }
    }


    // Evolve.

    if Likely1( s->respPropDiff >= 0 )
    {
        Coop * c  = e->coop;
        Coop * c2 = e->coop2;
        Coop pCMax = c[ePMax];
        Coop pC2Max = c2[ePMax];
        Coop rC2Max = c2[eRMax];
        double cP = s->changeProb;
        double wCP = s->wrongChangeProb;


        // Handle proposers.
        for ( a = 0; a != nrOfProposers; a++ )
        {
            // Handle coop2.

            // Probabilities greater than this are interpreted as true.
            double rd = RandomD();

            if ( rd <= cP )
            {
                if ( w[a] == pMax )
                {
                    if ( rd <= wCP )
                    {
                        if ( rd < wCP/2 )
                        {
                            if ( c2[a] != Coop_Max )
                            {
                                //c2[a]++;
                                // Update coopOrd.
                                incCoop2( e, a );
                            }
                        }
                        else
                        {
                            if ( c2[a] != 0 )
                            {
                                //c2[a]--;
                                decCoop2( e, a );
                            }
                        }
                    }
                }
                else if ( c2[a] < pC2Max )
                {
                    if ( rd < wCP )
                    {
                        if ( c2[a] != 0 )
                        {
                            decCoop2( e, a );
                        }
                    }
                    else
                    {
                        assert( c2[a] != Coop_Max );
                        incCoop2( e, a );
                    }
                }
                else if ( c2[a] > pC2Max )
                {
                    if ( rd < wCP )
                    {
                        if ( c2[a] != Coop_Max )
                        {
                            incCoop2( e, a );
                        }
                    }
                    else
                    {
                        assert( c2[a] != 0 );
                        decCoop2( e, a );
                    }
                }
                else
                {
                    if ( rd < wCP )
                    {
                        if ( rd < wCP/2 )
                        {
                            if ( c2[a] != Coop_Max )
                            {
                                incCoop2( e, a );
                            }
                        }
                        else
                        {
                            if ( c2[a] != 0 )
                            {
                                decCoop2( e, a );
                            }
                        }
                    }
                }

                // Make sure that coop is within acceptable range.
                checkCoopRange( esi, a );
            }


            // Handle coop.

            rd = RandomD();

            if ( rd <= cP )
            {
                if ( w[a] == pMax )
                {
                    if ( rd <= wCP )
                    {
                        if ( rd < wCP/2 )
                        {
                            if ( c[a] != Coop_Max )
                            {
                                c[a]++;
                            }
                        }
                        else
                        {
                            if ( c[a] != 0 )
                            {
                                c[a]--;
                            }
                        }
                    }
                }
                else if ( c[a] < pCMax )
                {
                    if ( rd < wCP )
                    {
                        if ( c[a] != 0 )
                        {
                            c[a]--;
                        }
                    }
                    else
                    {
                        assert( c[a] != Coop_Max );
                        c[a]++;
                    }
                }
                else if ( c[a] > pCMax )
                {
                    if ( rd < wCP )
                    {
                        if ( c[a] != Coop_Max )
                        {
                            c[a]++;
                        }
                    }
                    else
                    {
                        assert( c[a] != 0 );
                        c[a]--;
                    }
                }
                else
                {
                    if ( rd <= wCP )
                    {
                        if ( rd < wCP/2 )
                        {
                            if ( c[a] != Coop_Max )
                            {
                                c[a]++;
                            }
                        }
                        else
                        {
                            if ( c[a] != 0 )
                            {
                                c[a]--;
                            }
                        }
                    }
                }

                // Make sure that coop is within acceptable range.
                checkCoopRange( esi, a );
            }
        }


        // Handle responders.
        assert( a == nrOfProposers );
        for ( ; a != popSize; a++ )
        {
            // Probabilities greater than this are interpreted as true.
            double rd = RandomD();

            if ( rd <= cP )
            {
                if ( w[a] == rMax )
                {
                    if ( rd <= wCP )
                    {
                        if ( rd < wCP/2 )
                        {
                            if ( c2[a] != Coop_Max )
                            {
                                incCoop2( e, a );
                            }
                        }
                        else
                        {
                            if ( c2[a] != 0 )
                            {
                                decCoop2( e, a );
                            }
                        }
                    }
                }
                else if ( c2[a] < rC2Max )
                {
                    if ( rd < wCP )
                    {
                        if ( c2[a] != 0 )
                        {
                            decCoop2( e, a );
                        }
                    }
                    else
                    {
                        assert( c2[a] != Coop_Max );
                        incCoop2( e, a );
                    }
                }
                else if ( c2[a] > rC2Max )
                {
                    if ( rd < wCP )
                    {
                        if ( c2[a] != Coop_Max )
                        {
                            incCoop2( e, a );
                        }
                    }
                    else
                    {
                        assert( c2[a] != 0 );
                        decCoop2( e, a );
                    }
                }
                else
                {
                    if ( rd < wCP )
                    {
                        if ( rd < wCP/2 )
                        {
                            if ( c2[a] != Coop_Max )
                            {
                                incCoop2( e, a );
                            }
                        }
                        else
                        {
                            if ( c2[a] != 0 )
                            {
                                decCoop2( e, a );
                            }
                        }
                    }
                }
            }
        }


        return;
    }
    else // s->respPropDiff < 0
    {
        if (!randomN(3000))
            fprintf( stderr, "Error: More proposers than responders is not "
                             "supported yet.\n" );
    }
}


static void printSimResult( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Settings * s = esi->settings;
    Coop maxC2 = 0;
    Coop maxC2P = 0;  // For proposers.
    Coop maxC2R = 0;  // For responders.
    Coop minC2 = Coop_Max;
    Coop minC2P = Coop_Max;
    Coop minC2R = Coop_Max;
    uint64_t sum2 = 0;
    uint64_t sum2P = 0;
    uint64_t sum2R = 0;
    Coop * c2 = e->coop2;
    Pop pop = e->pop;

    Pop nrOfProposers = (e->pop - s->respPropDiff) / 2;

    Coop maxCP = 0;  // For proposers.
    Coop minCP = Coop_Max;
    uint64_t sumP = 0;
    Coop * c = e->coop;

    assert2( ( (e->pop - s->respPropDiff) & 1 ) == 0 );

    for ( Pop a = 0; a != nrOfProposers; a++ )
    {
        minCP = min(minCP,c[a]);
        maxCP = max(maxCP,c[a]);
        sumP += c[a];
    }

    fprintf( stdout, "prop coop:  min: %g, max: %g, avg: %g\n",
             (double)minCP/Coop_Max, (double)maxCP/Coop_Max,
             (double)sumP/nrOfProposers/Coop_Max );


    for ( Pop a = 0; a != nrOfProposers; a++ )
    {
        minC2P = min(minC2P,c2[a]);
        maxC2P = max(maxC2P,c2[a]);
        sum2P += c2[a];
    }

    fprintf( stdout, "prop coop2: min: %g, max: %g, avg: %g\n",
             (double)minC2P/Coop_Max, (double)maxC2P/Coop_Max,
             (double)sum2P/nrOfProposers/Coop_Max );

    minC2 = minC2P;
    maxC2 = maxC2P;
    sum2 = sum2P;

    for ( Pop a = nrOfProposers; a != pop; a++)
    {
        minC2R = min(minC2R,c2[a]);
        maxC2R = max(maxC2R,c2[a]);
        sum2R += c2[a];

        minC2 = min(minC2,c2[a]);
        maxC2 = max(maxC2,c2[a]);
        sum2 += c2[a];
    }

    fprintf( stdout, "resp coop2: min: %g, max: %g, avg: %g\n",
             (double)minC2R/Coop_Max, (double)maxC2R/Coop_Max,
             (double)sum2R/(pop-nrOfProposers)/Coop_Max );

    fprintf( stdout, "all  coop2: avg: %g\n",
             (double)sum2/pop/Coop_Max );

    /*
    fprintf( stdout, "coopBound * \"coop avg\": %g\n",
             ( (double)sum2P/nrOfProposers/Coop_Max ) *
             s->coopBound );
    */
}


// Do a secondary sorting, within the bins, of coop2, using the coop values.
//   Fast and partial.
//   Goes from greater to lesser.
static void sndCoop2SortGtToLt( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Settings * s = esi->settings;

    Coop * c = e->coop;

    Pop nrOfProposers = (e->pop - s->respPropDiff) / 2;

    Pop * bins = e->coopBins;
    Pop * cOrd = e->coopOrd;
    Pop * cPos = e->coopPos;

    // Go through the bins and do some floating or bubble sorting
    // passes, from greater to lesser.

    #pragma omp parallel for
    for ( uint32_t b = 0; b != Coop_Max + 1; b += 1 )
    //for ( uint32_t b = 0; b != 2*Coop_Max + 2; b += 2 )
    {
        if ( bins[2*b] != bins[2*b+1] )
        {
            Pop a = bins[2*b];
            Pop lastPos = bins[2*b+1];

            // Find first proposer.
            while ( a != lastPos  &&  cOrd[a] >= nrOfProposers )
            {
                a++;
            }

            while ( a != lastPos )
            {
                assert( cOrd[a] < nrOfProposers );

                // Find next proposer.

                Pop d = a+1;

                for ( ; d != lastPos  &&  cOrd[d] >= nrOfProposers; d++ )
                {
                }

                if ( cOrd[d] < nrOfProposers )
                {
                    if ( c[cOrd[a]] < c[cOrd[d]] )
                    {
                        // Swap a and d entries.

                        Pop tmp = cOrd[a];
                        cPos[tmp] = d;
                        cPos[cOrd[d]] = a;
                        cOrd[a] = cOrd[d];
                        cOrd[d] = tmp;
                    }
                }

                // Continue with d instead of a.
                a = d;  // This will work for all cases.
            }
        }
    }
}


// Do a secondary sorting, within the bins, of coop2, using the coop values.
//   Fast and partial.
//   Goes from lesser to greater.
static void sndCoop2SortLtToGt( EvoSimInstance * esi )
{
    Evo * e = esi->evo;
    Settings * s = esi->settings;

    Coop * c = e->coop;

    Pop nrOfProposers = (e->pop - s->respPropDiff) / 2;

    Pop * bins = e->coopBins;
    Pop * cOrd = e->coopOrd;
    Pop * cPos = e->coopPos;

    // Go through the bins and do some floating or bubble sorting
    // passes, from lesser to greater.

    #pragma omp parallel for
    for ( uint32_t b = 0; b != Coop_Max + 1; b += 1 )
    //for ( uint32_t b = 0; b != 2*Coop_Max + 2; b += 2 )
    {
        if ( bins[2*b] != bins[2*b+1] )
        {
            Pop fstPos = bins[2*b];
            Pop a = bins[2*b+1];

            // Find last proposer.
            while ( a != fstPos  &&  cOrd[a] >= nrOfProposers )
            {
                a--;
            }

            while ( a != fstPos )
            {
                assert( cOrd[a] < nrOfProposers );

                // Find previous proposer.

                Pop d = a-1;

                for ( ; d != fstPos  &&  cOrd[d] >= nrOfProposers; d-- )
                {
                }

                if ( cOrd[d] < nrOfProposers )
                {
                    if ( c[cOrd[a]] > c[cOrd[d]] )
                    {
                        // Swap a and d entries.

                        Pop tmp = cOrd[a];
                        cPos[tmp] = d;
                        cPos[cOrd[d]] = a;
                        cOrd[a] = cOrd[d];
                        cOrd[d] = tmp;
                    }
                }

                // Continue with d instead of a.
                a = d;  // This will work for all cases.
            }
        }
    }
}


// Simulates evolution.
//   Returns true iff there wasn't enough memory.
//   One round takes linear time in the number of entities.
bool evoSim_sim( EvoSimInstance * esi )
{
    //Evo * evo = esi->evo;
    Round nrOfRounds = esi->evo->nrOfRounds;

    for ( Round r = 0; r != nrOfRounds; r++ )
    {
        assert4( ordCheck(esi->evo) );
        assert4( rangeCheck(esi) );


        // Do a round of material cooperation.
        coopMat(esi);


        // Reduce or convert material wealth to a type of overall wealth.
        returns(esi);


        // Do a quick secondary sorting of coop2.
        for ( uint8_t n = 0; n != esi->settings->coopSignal; n++ )
        {
            sndCoop2SortLtToGt(esi);  // Do more LtToGt??
            sndCoop2SortGtToLt(esi);
            //sndCoop2SortLtToGt(esi);  // Do more LtToGt??
        }


        // Do a second cycle of cooperation.
        coop2(esi);


        // Evolve.
        evolve(esi);


        if ( ( esi->settings->verbosityVector &
               EvoSimVerbosity_printInfo )  &&
             ( r % 10000 == 0 ) )
        {
            fprintf( stdout, "after round %llu:\n",
                     (unsigned long long)r );

            printCoopInfo(esi);
            printCoop2Info(esi);

            assert3( rangeCheck(esi) );
        }
    }


    // Print result.
    if ( esi->settings->verbosityVector & EvoSimVerbosity_printResult )
    {
        printSimResult(esi);
    }


    return false;
}
