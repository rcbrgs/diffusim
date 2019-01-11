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
\t -e experiment_name 
EOF
)

while getopts "e:" OPTION; do
    case $OPTION in
	e)
	    experiment=$OPTARG
	    ;;
	*) 
	    echo "Unrecognized option."
	    echo -e "$usage"
	    exit 1
	    ;;
    esac
done

parameters="max_attenuation input_file output_filename_prefix gradient_direction.x gradient_direction.y gradient_direction.z steps"
for param in $parameters; do
    eval content=\$$param
    if [ -z "$content" ]; then
	echo "ERROR: Missing parameters."
	echo -e "$usage"
	exit 1
    fi
done
if [ -z "$experiment" ]; then
    echo "ERROR: Missing parameters."
    echo -e "$usage"
    exit 1
fi

here=`pwd`
mkdir /tmp/$$
cd /tmp/$$
cp $here/sample_adc_z*.bin .
samples=`ls -1 --color=never $here/sample_adc_z*.bin | sed 's/^.*\///'`
if [ ! -e stejskal_clustered.exe ]; then
    cp $here/stejskal_clustered.exe .
fi
if [ ! -e directions.txt ]; then
    cp $here/directions.txt .
fi

# calculations
for sample in $samples; do
    slice=`echo $sample | sed 's/sample_adc_z//' | sed 's/.bin//'`
    number=`echo $slice | awk '{ printf ("%03d", $1) }'`
    echo "slice $slice"
    while read line; do
	grad_x=`echo $line | awk '{ print $1 }'`
	grad_y=`echo $line | awk '{ print $2 }'`
	grad_z=`echo $line | awk '{ print $3 }'`
	echo -e "\t dir $grad_x $grad_y $grad_z"
	/tmp/$$/stejskal_clustered.exe $sample ${experiment}_${number}_${grad_x}_${grad_y}_${grad_z} $grad_x $grad_y $grad_z
	mv *.raw $here/
    done < /tmp/$$/directions.txt
done

# cleanup
rm -rf /tmp/$$
exit 0
