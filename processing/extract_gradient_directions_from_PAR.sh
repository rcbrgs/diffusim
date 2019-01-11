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

if [ $# -lt 2 ]; then
    echo "USAGE: $0 .par_input_file_name output_file_name"
    exit 1
fi

input_file_name=$1
output_file_name=$2

#grep -v "^#" $input_file_name | awk '{ print $46 " " $47 " " $48 }' | grep -v "^[[:space:]]*$" | uniq | grep -v "0.000 0.000 0.000" > /tmp/temp_file
grep -v "^#" $input_file_name | awk '{ print $48 " " $46 " " $47 }' | grep -v "^[[:space:]]*$" | uniq | grep -v "0.000 0.000 0.000" > /tmp/temp_file
number_of_lines=`wc -l /tmp/temp_file | cut -d" " -f 1`
echo "$number_of_lines" > $output_file_name
cat /tmp/temp_file >> $output_file_name
rm /tmp/temp_file
