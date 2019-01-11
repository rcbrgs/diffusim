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

mkdir /tmp/$$
cd /tmp/$$
number=`echo $slice | awk '{ printf ("%03d", $1) }'`
sample=`ls -1 --color=never /sampa/home/rborges/latest/$batch/sample_adc_z$number.bin | sed 's/^.*\///'`
if [ ! -e /tmp/$$/$sample ]; then
    cp /sampa/home/rborges/latest/$batch/$sample .
fi
if [ ! -e stejskal_clustered.exe ]; then
    cp /sampa/home/rborges/latest/$batch/stejskal_clustered.exe .
fi
if [ ! -e stejskal_clustered.sh ]; then
    cp /sampa/home/rborges/latest/$batch/stejskal_clustered.sh .
fi
if [ ! -e directions.txt ]; then
    cp /sampa/home/rborges/latest/$batch/directions.txt .
fi

# calculations
while read line; do
    grad_x=`echo $line | awk '{ print $1 }'`
    grad_y=`echo $line | awk '{ print $2 }'`
    grad_z=`echo $line | awk '{ print $3 }'`
    /tmp/$$/stejskal_clustered.sh -i $sample -o ${batch}_${number}_${grad_x}_${grad_y}_${grad_z} -x $grad_x -y $grad_y -z $grad_z -s $steps_per_second
    mv *.raw /sampa/home/rborges/latest
done < /tmp/$$/directions.txt

# cleanup
rm -rf /tmp/$$
exit 0
