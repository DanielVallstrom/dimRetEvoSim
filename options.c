// Module for handling command line options.


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


#include "options.h"
#include "common.h"
#include "evoSim.h"

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


#ifndef evoSimVersion
#define evoSimVersion "<no version number available>"
#endif


// Prints the --help message.
static void printHelpMessage( EvoSimInstance * esi )
{
    Settings * s = esi->settings;
    Evo      * e = esi->evo;

    fprintf(
stdout,
"\nUsage: dimRetEvoSim [options] [<nrOfRounds>]\n"
"Simulates evolution under diminishing returns.\n"
           );

    fprintf(
stdout,
"\nOptions, with defaults in [ ]:\n"
"For more on what the options mean, see evoSim.h.\n"
           );

    fprintf(
stdout,
"  -h --help               Print this message.\n"
           );

    fprintf(
stdout,
"  -R --returns [integer]  Set material returns. 0 is the default and \n"
"                          means diminishing or logarithmic returns.\n"
"                            0 means log. 1 means linear. 2 means square\n"
"                          root. 3 means cube root. n means nth root. [%u]\n"
"  -b --coop-bound <float>\n"
"                          Set how bound to coop2 coop should be.\n"
"                          coopBound*coop[a] >= coop2[a]. [%g]\n"
"  -c --coop-max-at-start <unsigned integer>\n"
"                          Set strict upper bound for coop at start. [%u]\n"
"  -d --resp-prop-diff <integer>\n"
"                          Set respPropDiff value.\n"
"                          pop = proposers + responders.\n"
"                          responders - proposers = respPropDiff.\n"
"                          propRespDiff takes precedence over population:\n"
"                          1 will be added to pop size iff pop and\n"
"                          respPropDiff aren't both odd or even.\n"
"                          Meant to be non-negative to create pressure\n"
"                          for unfair, non-cooperative proposers,\n"
"                          initially. But can be negative. [%d]\n"
"  -e --change-prob <float>\n"
"                          The probability that entities not gaining the\n"
"                          most for a round will change (likely in the\n"
"                          most promising direction). [%g]\n"
"  -g --wrong-change-prob <float>\n"
"                          The probability that entities gaining the most\n"
"                          for a round will change, in a random direction.\n"
"                          And the probability that entities not gaining\n"
"                          the most for a round will change in the least\n"
"                          promising direction. [%g]\n"
"  -o --coop-offset-at-start <unsigned integer>\n"
"                          Set coop offset at start. [%u]\n"
"  -p --pop <unsigned integer>\n"
"                          Set the population size. [%u]\n"
"  --print-sim[=no|yes]    Print simulation if possible. [%s]\n"
"  --print-options         Print option settings and then quit.\n"
"                             E.g., to fine-tune the verbosity, run e.g.\n"
"                          './dimRetEvoSim -v3 --print-options'\n"
"                          and then tune the printed verbosity vector.\n"
"  -r --nr-of-rounds <unsigned integer>\n"
"                          Set the number of rounds to be simulated. [%llu]\n"
"  -q --quiet --silent     Run silently; only print error messages.\n",
(unsigned int)s->returns,
(double)s->coopBound,
(unsigned int)s->coopMaxAtStart,
(signed int)s->respPropDiff,
(double)s->changeProb,
(double)s->wrongChangeProb,
(unsigned int)s->coopOffsetAtStart,
(unsigned int)e->pop,
(s->verbosityVector & EvoSimVerbosity_printSim) ? "yes" : "no",
(unsigned long long int)e->nrOfRounds
           );

    fprintf(
stdout,
"  -s --seed <seed>        Set the seed. [current time]\n"
           );

    fprintf(
stdout,

"  -t --coop-signal <unsigned integer>\n"
"                          Set the effort spent to break ties in second\n"
"                          cooperation cycle.\n"
"                            0 means no effort. 1 means a bit. 2 a bit\n"
"                          more... [%u]\n"
"  -v --verbose [level]    Set verbosity level (0-9). No arg means 8. [4]\n"
"  --verbosity-vector <unsigned integer>\n"
"                          Set the verbosity vector. [%#llx]\n"
"                          The integer can be bin (0b), hex (0x), octal (0) or"
                                                                           "\n"
"                          plain decimal.\n"
"  --version               Print the version number.\n"
"  -w --resp-contri <float>\n"
"                          Set how much the cooperation contribution from\n"
"                          the responder should matter to a coop2 proposer's\n"
"                          gain.\n"
"                            Set to 0 if you want to turn this off and keep\n"
"                          some sort of dictator game. [%g]\n",
(unsigned int)s->coopSignal,
(unsigned long long int)s->verbosityVector,
(double)s->respContri
           );

    fprintf(
stdout,
"\nFor more on what the options mean, see evoSim.h.\n"
           );

    fprintf(
stdout,
"\ndimRetEvoSim is open source licensed under the Reciprocal Public\n"
"License, version 1.1; Copyright (C) 2022 Daniel Vallstrom.\n"
           );

    fprintf(
stdout,
"\nSend bug reports, feedback, etc. to daniel.vallstrom@gmail.com.\n\n"
           );
}


// Sets the verbosity level.
static void setVerbosityLevel( EvoSimInstance * esi, unsigned int vl )
{
    EvoSimVerbosityVector vv = 0;

    switch ( vl )
    {

    // All cases are fall-throughs:

    default:

    case 9: case 8:
        vv |= EvoSimVerbosity_asciiPrintSim;
    case 7:
        vv |= EvoSimVerbosity_printInfo;
    case 6:
        vv |= EvoSimVerbosity_printTime;
    case 5:
        vv |= EvoSimVerbosity_printSim;
    case 4:
        vv |= EvoSimVerbosity_printSeed;
    case 3:
        vv |= EvoSimVerbosity_printResult;
    case 2:
    case 1:
        vv |= EvoSimVerbosity_printErrors;
    case 0:

        break;
    }

    esi->settings->verbosityVector = vv;
}


// Prints option settings.
static void printSettings( EvoSimInstance * esi )
{
    Settings * s = esi->settings;
    Evo      * e = esi->evo;

    printf(
"  --coop-max-at-start=%u"
"  --resp-prop-diff=%d"
"  --change-prob=%g"
"  --wrong-change-prob=%g"
"  --coop-bound=%g"
"  --coop-offset-at-start=%u"
"  --pop=%u"
"  --nr-of-rounds=%llu"
"  --print-sim=%s",
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
"  --verbosity-vector=%#llx"
"  --resp-contri=%g"
"  --coop-signal=%u",
(unsigned long long int) s->verbosityVector,
(double)s->respContri,
(unsigned int)s->coopSignal
          );

    putc( '\n', stdout );
}


// Parse the command line options.
//   Returns 0 iff everything went fine and you should continue on.
// Returns 1 iff there was an error. Returns > 1 for e.g. --help and you
// should stop.
//   If the population size isn't set, then it will be set to an appropriate
// value.
static int parseCommandLineOptions( EvoSimInstance * esi,
                                    int argC, char * * argV )
{
    #ifndef NoGetopt

    // Tells whether or not the number of rounds has been specified.
    bool nrOfRoundsIsSet = false;

    // Tells whether or not the population size has been specified.
    bool popIsSet = false;

    {
        int c;

        int optionIndex;   // Never used.

        static struct option longOptions[] =
        {
            { "returns",                optional_argument, NULL, 'R' },
            { "coop-bound",             required_argument, NULL, 'b' },
            { "coop-max-at-start",      required_argument, NULL, 'c' },
            { "resp-prop-diff",         required_argument, NULL, 'd' },
            { "change-prob",            required_argument, NULL, 'e' },
            { "wrong-change-prob",      required_argument, NULL, 'g' },
            { "help",                   no_argument,       NULL, 'h' },
            { "coop-offset-at-start",   required_argument, NULL, 'o' },
            { "pop",                    required_argument, NULL, 'p' },
            { "quiet",                  no_argument,       NULL, 'q' },
            { "silent",                 no_argument,       NULL, 'q' },
            { "nr-of-rounds",           required_argument, NULL, 'r' },
            { "seed",                   required_argument, NULL, 's' },
            { "coop-signal",            required_argument, NULL, 't' },
            { "verbose",                optional_argument, NULL, 'v' },
            { "resp-contri",            required_argument, NULL, 'w' },
            { "version",                no_argument,       NULL, CHAR_MAX+2 },
            { "print-sim",              optional_argument, NULL, CHAR_MAX+3 },
            { "print-options",          no_argument,       NULL, CHAR_MAX+4 },
            { "verbosity-vector",       required_argument, NULL, CHAR_MAX+5 },
            { 0, 0, 0, 0 }
        };

        while ( true )
        {
            c = getopt_long( argC, argV, "R::b:c:d:e:g:ho:p:qr:s:t:v::w:",
                             longOptions, &optionIndex );

            if ( c == -1 )
            {
                break;
            }

            switch ( c )
            {
            case 'R':
                if ( optarg )
                {
                    unsigned int vl;

                    if ( readUInt( optarg, &vl ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -R and --returns must be an\n"
                                 "unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->returns = vl;
                }
                else
                {
                    esi->settings->returns = 1;
                }

                break;

            case 'b':
                {
                    double value;

                    if ( readReal( optarg, &value ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -m and --coop-bound\n"
                                 "must be a real on form '1.2', '3.' or '4'. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->coopBound = value;
                }

                break;

            case 'c':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -c and --coop-max-at-start must\n"
                                 "be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->coopMaxAtStart = n;
                }

                break;

            case 'd':
                {
                    long long int n;

                    if ( readLL( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -d and --resp-prop-diff must be\n"
                                 "an integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->respPropDiff = n;
                }

                break;

            case 'e':
                {
                    double value;

                    if ( readReal( optarg, &value ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -e and --change-prob\n"
                                 "must be a real on form '1.2', '3.' or '4'. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->changeProb = value;
                }

                break;

            case 'g':
                {
                    double value;

                    if ( readReal( optarg, &value ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -g and --wrong-change-prob\n"
                                 "must be a real on form '1.2', '3.' or '4'. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->wrongChangeProb = value;
                }

                break;

            case 'h':
                printHelpMessage(esi);

                return 2;

            case 'o':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -o and --coop-offset-at-start\n"
                                 "must be an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->coopOffsetAtStart = n;
                }

                break;

            case 'p':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -p and --pop must be an\n"
                                 "unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    if ( n == 0 )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "option -p and --pop must not be 0.\n" );

                        return 1;
                    }

                    esi->evo->pop = n;
                    popIsSet = true;
                }

                break;

            case 'q':
                esi->settings->verbosityVector = 0;

                break;

            case 'r':
                {
                    unsigned long long int n;

                    if ( readULLInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -r and --nr-of-rounds must be an\n"
                                 "unsigned long integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->evo->nrOfRounds = n;
                    nrOfRoundsIsSet = true;
                }

                break;

            case 's':
                {
                    unsigned long long int seed;

                    if ( readULLInt( optarg, &seed ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -s and --seed must be an\n"
                                 "unsigned long integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->seed = seed;
                    srand(esi->settings->seed);
                }

                break;

            case 't':
                {
                    unsigned int n;

                    if ( readUInt( optarg, &n ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -t and --coop-signal must be\n"
                                 "an unsigned integer. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->coopSignal = n;
                }

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

                    setVerbosityLevel( esi, vl );
                }
                else
                {
                    //esi->settings->verbosityVector = 0xffffffffffffffff;
                    setVerbosityLevel( esi, 8 );
                }

                break;

            case 'w':
                {
                    double value;

                    if ( readReal( optarg, &value ) )
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "options -w and --resp-contri\n"
                                 "must be a real on form '1.2', '3.' or '4'. "
                                 "You supplied %s.\n\n", optarg );

                        return 1;
                    }

                    esi->settings->respContri = value;
                }

                break;

            case CHAR_MAX+2:
                printf( "dimRetEvoSim %s\n", evoSimVersion );

                return 2;

            case CHAR_MAX+3:
                if ( optarg )
                {
                    if ( strcmp( optarg, "no" ) == 0 )
                    {
                        esi->settings->verbosityVector &=
                            ~EvoSimVerbosity_printSim;
                    }
                    else if ( strcmp( optarg, "yes" ) == 0 )
                    {
                        esi->settings->verbosityVector |=
                            EvoSimVerbosity_printSim;
                    }
                    else
                    {
                        fprintf( stderr,
                                 "\nError: the argument to command line "
                                 "option --print-sim must be\n"
                                 "no or yes. You supplied %s.\n\n", optarg );

                        return 1;
                    }
                }
                else
                {
                        esi->settings->verbosityVector |=
                            EvoSimVerbosity_printSim;
                }

                break;

            case CHAR_MAX+4:
                printSettings( esi );

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

                    esi->settings->verbosityVector = vv;
                }

                break;

            default:
                fprintf( stderr, "\nError: getopt returned %d.\n", c );

                return 1;
            }
        }
    }


    // Handle leftover arguments. If no nr-of-rounds has been specified, the
    // first leftover argument is assumed to be that value.
    if ( optind < argC )
    {
        if ( !nrOfRoundsIsSet )
        {
            unsigned long long int n;

            if ( readULLInt( argV[optind], &n ) )
            {
                fprintf( stderr, "\nError: the argument to command line "
                                 "options -r and --nr-of-rounds must be an\n"
                                 "unsigned long integer. "
                                 "You supplied %s.\n\n", argV[optind] );

                return 1;
            }

            esi->evo->nrOfRounds = n;
            nrOfRoundsIsSet = true;

            optind++;
        }

        // Any additional argument is an error.
        if ( optind != argC )
        {
            fprintf( stderr, "\nError: non-option argv elements:\n" );

            for ( ; optind != argC; optind++ )
            {
                fprintf( stderr, "  %s\n", argV[optind]);
            }

            fprintf( stderr, "See dimRetEvoSim --help.\n\n" );

            return 1;
        }
    }


    // If pop hasn't been set, set it to an appropriate value.
    if ( !popIsSet )
    {
        esi->evo->pop = evoSim_appropriatePop(esi);
    }


    // Add 1 to population size iff pop and respPropDiff aren't both
    // odd or even.
    if ( ( esi->evo->pop & 1 )  !=  ( esi->settings->respPropDiff & 1 ) )
    {
        esi->evo->pop += 1;
    }


    #endif // #ifndef NoGetopt

    return 0;
}


// Returns 0 iff everything went fine and you should continue on.
// Returns 1 iff there was an error. Returns > 1 for e.g. --help and you
// should stop.
int options_parseCommandLineOptions( EvoSimInstance * esi,
                                     int argC, char * * argV )
{
    int result = parseCommandLineOptions( esi, argC, argV );

    #ifndef NoGetopt

    // Reset getopt so that more calls to options_parseCommandLineOptions
    // can be made. An alternative would be to set optind to 1 at the start
    // of parseCommandLineOptions. Best would be though to dump getopt
    // altogether and write something good instead!!
    optind = 1;  // Is this enough???

    #endif // #ifndef NoGetopt

    return result;
}
