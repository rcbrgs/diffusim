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

g++ -Wall -lmxml pretty.cpp b-value_calculation.cpp -o b-value_calculation.exe
g++ -Wall -lmxml pretty.cpp mask_generator.cpp -o mask_generator.exe
g++ -Wall pretty.cpp stejskal_clustered.cpp -o stejskal_clustered.exe
