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
\t -e experiment name 
\t -s number of slices
\t -t number of steps per second
EOF
)

while getopts "e:s:t:" OPTION; do
    case $OPTION in
	e)
	    experiment_name=$OPTARG
	    ;;
	s)
	    slices=$OPTARG
	    ;;
	t)  steps_per_second=$OPTARG
	    ;;
	*) 
	    echo "Unrecognized option."
	    echo -e "$usage"
	    exit 1
	    ;;
    esac
done

cd ~/latest/$experiment_name

i=1
while [ $i -le $slices ]; do
    echo -n "($i/$slices) "
    ./job_wrapper.sh $(($i-1)) $experiment_name $steps_per_second
    i=$(($i+1))
done

all_done=1
while [ ! $all_done -eq 0 ]; do
    sleep 29
    files=`ls -1 --color=never ~/latest/$experiment_name/job.sh.* 2>/dev/null | wc -l | cut -d" " -f 1`
    echo "files=$files"
    expected_files=$(($slices*2))
    if [ "$files" == "$expected_files" ]; then
	all_done=0
    fi
done

# How long did it take?                                                                                                                         
first_job=`head -n 1 qsub_output.txt | sed 's/\..*$//'`
last_job=`tail -n 1 qsub_output.txt | sed 's/\..*$//'`
echo "Batch started at $first_job and ended at $last_job." >> qsub_output.txt
echo "tracejob $first_job 2>/dev/null | grep enqueuing | awk '{ print \$2 }'" >> qsub_output.txt
echo "tracejob $last_job  2>/dev/null | grep dequeuing | awk '{ print \$2 }'" >> qsub_output.txt
