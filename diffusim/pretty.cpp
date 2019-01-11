/*
# Diffusynth - Synthesize DWI images from T2 images.
# Copyright (C) 2013 Renato Callado Borges
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author can be reached at rborges@if.usp.br
*/
#include <iostream>
#include <cstdarg>
#include <stdio.h>

#include "pretty.hpp"

using namespace std;

pretty::pretty ()
{
  indentation = 0;
  current_verbosity_level = verbosity_debug;
}

void pretty::f ( unsigned int verbosity_level, const char* format, ... )
{
  char    buffer[256];
  va_list args;

  if ( verbosity_level > current_verbosity_level )
    return;

  va_start ( args, format );
  vsprintf ( buffer, format, args );
  for ( unsigned int count = 0; count < indentation; count++ )
    cout << "\t";
  cout << buffer;
  va_end ( args );
}
