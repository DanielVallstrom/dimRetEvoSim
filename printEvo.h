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


#ifndef printEvo_H
#define printEvo_H


#include "evoSim.h"

#include <stdbool.h>


// Prints an evolution simulation using Freeglut and OpenGL.
//   Returns true iff something went wrong.
bool printEvo( EvoSimInstance * esi );


// Prints an evolution simulation using ascii.
void asciiPrintEvo( EvoSimInstance * esi );


#endif // printEvo_H
