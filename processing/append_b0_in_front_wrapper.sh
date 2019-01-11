#!/bin/bash

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

./04-append_b0_in_front.exe 01-nylon_b0.raw 03-output-000.raw 04-b0_and_output-000.raw
./04-append_b0_in_front.exe 01-nylon_b0.raw 03-output-001.raw 04-b0_and_output-001.raw
./04-append_b0_in_front.exe 01-nylon_b0.raw 03-output-002.raw 04-b0_and_output-002.raw
./04-append_b0_in_front.exe 01-nylon_b0.raw 03-output-003.raw 04-b0_and_output-003.raw
