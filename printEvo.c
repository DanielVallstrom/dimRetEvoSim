// For printing an evolution simulation. Uses OpenGL and Freeglut.

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


#include "printEvo.h"
#include "common.h"

#ifdef PrintEvo
#include <GL/freeglut.h>
#endif

#include <stdio.h>


// The width of what's ascii-printed.
#define AsciiWidth 160


// Prints an evolution simulation using ascii.
void asciiPrintEvo( EvoSimInstance * esi )
{
    Evo * e = esi->evo;

    return;
}


// Prints an evolution simulation using Freeglut and OpenGL.
//   Returns true iff something went wrong.
bool printEvo( EvoSimInstance * esi )
{
    #ifdef PrintEvo

    Evo * e = esi->evo;

    #else

    printf( "Graphically printing the evolution failed. Make sure OpenGL and \n"
            "Freeglut are installed, and compile with printing enabled \n"
            "(have PRINTEVO defined to 'true'; do e.g.\n"
            "make clean; make gccprint).\n"
            "OpenGL printing isn't yet implemented though!\n" );

    #endif

    return false;
}
