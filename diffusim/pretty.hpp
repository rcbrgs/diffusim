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
#ifndef PRETTY
#define PRETTY

enum verbosity_levels { verbosity_silent = 0,
			verbosity_error = 1,
			verbosity_warning = 2,
			verbosity_status = 3,
			verbosity_information = 4,
			verbosity_debug = 5 };

class pretty
{
public:
  unsigned int indentation;
  unsigned int current_verbosity_level;
  pretty ();
  void f ( unsigned int verbosity_level, const char* format, ... );
};

#endif
