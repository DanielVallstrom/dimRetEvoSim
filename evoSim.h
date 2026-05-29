// Module simulating evolution.


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


#ifndef evoSim_H
#define evoSim_H


#include <stdbool.h>
#include <stdint.h>



// The default number of rounds of the simulation.
#define DefaultNrOfRounds (500000)

// The default population size.
#define DefaultPop (1000)

// The default effort spent to break ties in second cooperation cycle.
//   0 means no effort. 1 means a bit. 2 a bit more...
#define DefaultCoopSignal (4)

// The default size difference between responders and proposers.
#define DefaultRespPropDiff (0)

// How bound to coop2 coop should be by default.
// coopBound*coop[a] >= coop2[a].
#define DefaultCoopBound (1.1)

// The default probability that entities not gaining the most for a round
// will change (likely in the most promising direction).
#define DefaultChangeProb (0.4)

// The default probability that entities gaining the most for a round
// will change, in a random direction. And the default probability that
// entities not gaining the most for a round will change in the least
// promising direction.
#define DefaultWrongChangeProb (0.16)

// The default of how much the cooperation contribution from the responder
// should matter to a coop2 proposer's gain.
//   Set to 0 if you want to turn this off.
#define DefaultRespContri (0.25)

// The default value of material returns. Should be 0, meaning diminishing
// or log.
//   0 means log. 1 means linear. 2 means square root. 3 means cube
// root...
#define DefaultReturns (0)


// Types. -------------------------------------------------------------


// A round for simulations.
typedef uint32_t Round;

// The maximum value of the type Round.
#define Round_Max UINT32_MAX

// An undefined round.
//#define UndefRound Round_Max


// Population.
typedef uint16_t Pop;

// The maximum value of the type Pop.
#define Pop_Max UINT16_MAX

// An undefined population.
#define UndefPop Pop_Max


// A measure of cooperation.
//   An unsigned type.
//   c/Coop_Max is the normalized cooperation value.
typedef uint8_t Coop;

// The maximum value of the type Coop.
#define Coop_Max UINT8_MAX

// The default strict upper bound for coop at the start.
//   Relative value compared to lowest; see offset; the real strict
// upper bound is this plus the offset.
#define CoopMaxAtStart (Coop_Max/4 + 1)

// The offset for coop at the start.
#define CoopOffsetAtStart (10)


// Wealth.
//   Should it be of some float type?? That would have better distribution
// and value range. But it might be slower, and more cumbersome --- you
// might want to do bit operations e.g.?
typedef uint16_t Wealth;
//typedef Coop Wealth;

// The maximum value of the type Wealth.
#define Wealth_Max UINT16_MAX
//#define Wealth_Max Coop_Max

// An undefined wealth.
//#define UndefWealth Wealth_Max


// Bin range. Enough to hold 2*Coop_Max. Or just use uint32_t.
// Can be small since coopBins can't be too long.
typedef uint16_t Bin;


typedef struct Evo_
{
    Wealth * wealth;   // The wealth for each entity.

    Coop * coop;       // The material cooperation tendency for proposers.
    Coop * coop2;      // The cooperation tendency for each entity.

    Pop * coopOrd;     // The population in decreasing coop2 order.
    Pop * coopBins;    // The first and last positions where coopOrd changes.
                       //   [0..2*Coop_Max]
                       //   UndefPop is used as an undefined or n/a value.
                       //   For fast sorting.
                       //   (Naively kind of in "reverse order".)
    Pop * coopPos;     // The position in coopOrd of an entity.

    Pop pop;           // The number of entities in the simulation.

    Round nrOfRounds;  // How many rounds the simulation will run.
    // Should be moved to Settings.

} Evo;


// Each bit in a verbosity vector decides if some particular info should be
// printed when appropriate. See below for what each bit means.
typedef uint64_t EvoSimVerbosityVector;

#define EvoSimVerbosity_printErrors       ( (EvoSimVerbosityVector)1 << 0 )
#define EvoSimVerbosity_printResult       ( (EvoSimVerbosityVector)1 << 1 )
#define EvoSimVerbosity_printSim          ( (EvoSimVerbosityVector)1 << 2 )
#define EvoSimVerbosity_printSeed         ( (EvoSimVerbosityVector)1 << 3 )
#define EvoSimVerbosity_printInfo         ( (EvoSimVerbosityVector)1 << 4 )
#define EvoSimVerbosity_asciiPrintSim     ( (EvoSimVerbosityVector)1 << 5 )
#define EvoSimVerbosity_printTime         ( (EvoSimVerbosityVector)1 << 6 )


// Structure containing settings.
typedef struct Settings_
{
    //Round nrOfRounds;  // How many rounds the simulation should run.

    //Pop pop;           // The number of entities in the simulation.


    // A bit vector where each bit determines if some particular info should
    // be printed. 0 means quiet.
    EvoSimVerbosityVector verbosityVector;

    // The current seed. The default is time(NULL).
    unsigned long int seed;

    // The effort spent to break ties in second cooperation cycle.
    //   0 means no effort. 1 means a bit. 2 a bit more...
    uint8_t coopSignal;

    // A strict upper bound for coop at the start.
    //   Relative value compared to lowest; see offset; the real strict
    // upper bound is this plus the offset.
    Coop coopMaxAtStart;

    // An offset for coop at the start.
    Coop coopOffsetAtStart;

    // The number of time steps the simulation should run.
    // Time nrOfSteps;

    // pop = proposers + responders.
    // responders - proposers = respPropDiff.
    //   propRespDiff takes precedence over population: 1 will be added to
    // population size iff pop and respPropDiff aren't both odd or even.
    //   Meant to be non-negative to create pressure for unfair,
    // non-cooperative proposers, initially. But can be negative.
    signed int respPropDiff;
    // Should be moved to Evo.

    // How bound to coop2 coop should be. coopBound*coop[a] >= coop2[a].
    double coopBound;

    // The probability that entities not gaining the most for a round
    // will change, in the most promising direction.
    double changeProb;

    // The probability that entities gaining the most for a round
    // will change, in a random direction. And the probability that
    // entities not gaining the most for a round will change, in the least
    // promising direction.
    double wrongChangeProb;

    // How much the cooperation contribution from the responder
    // should matter to a coop2 proposer's gain.
    //   Set to 0 if you want to turn this off and keep some sort of
    // dictator game.
    double respContri;


    // The material returns. 0 is the default and means diminishing or
    // logarithmic returns.
    //   0 means log. 1 means linear. 2 means square root. 3 means cube
    // root...
    uint32_t returns;

} Settings;


// The main structure containing everything.
typedef struct EvoSimInstance_
{
    Evo * evo;

    Settings * settings;

} EvoSimInstance;


// Returns a new EvoSimInstance instance.
EvoSimInstance * evoSim_newInstance(void);


// Allocates evolution simulation arrays.
//   Returns true iff there wasn't enough memory.
bool evoSim_allocArrays( EvoSimInstance * esi );


// Returns an appropriate population size.
Pop evoSim_appropriatePop( EvoSimInstance * esi );


// Simulates evolution.
//   Returns true iff there wasn't enough memory.
bool evoSim_sim( EvoSimInstance * esi );


#endif // evoSim_H
