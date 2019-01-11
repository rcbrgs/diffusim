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

usage=$(
cat <<EOF 
USAGE: $0 <PARAMETERS>
PARAMETERS (all are obligatory):
\t -i input_file
\t -o output_filename_prefix 
\t -s number of steps for longest integration
\t -x gradient_direction_x 
\t -y gradient_direction_y 
\t -z gradient_direction_z
EOF
)

while getopts "i:o:s:x:y:z:" OPTION; do
    case $OPTION in
	i)
	    input_file=$OPTARG
	    ;;
	o)
	    output_filename_prefix=$OPTARG
	    ;;
	s)
	    steps=$OPTARG
	    ;;
	x)
	    gradient_direction_x=$OPTARG
	    ;;
	y)
	    gradient_direction_y=$OPTARG
	    ;;
	z)
	    gradient_direction_z=$OPTARG
	    ;;
	*) 
	    echo "Unrecognized option."
	    echo -e "$usage"
	    exit 1
	    ;;
    esac
done

parameters="input_file output_filename_prefix gradient_direction.x gradient_direction.y gradient_direction.z steps"
for param in $parameters; do
    eval content=\$$param
    if [ -z "$content" ]; then
	echo "ERROR: Missing parameters."
	echo -e "$usage"
	exit 1
    fi
done

./stejskal_clustered.exe $input_file $output_filename_prefix $gradient_direction_x $gradient_direction_y $gradient_direction_z $steps

exit 0
