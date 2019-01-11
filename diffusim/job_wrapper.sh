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

if [ -z "$2" ]; then
    echo "You must give a name for this project!"
    exit 1
fi
slice=$1
batch=$2
steps_per_second=$3

echo "Submitting job for slice $slice."
cd ~/latest/$batch

test=1
while [ ! $test -eq 0 ]; do
    qsub -q griper -v batch=$batch,slice=$slice,steps_per_second=$steps_per_second /sampa/home/rborges/latest/$batch/job.sh >> qsub_output.txt
    test=$?
    if [ ! $test -eq 0 ]; then
	sleep 1
    fi
done
