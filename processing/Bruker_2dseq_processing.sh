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
\t -b number_of_b_values
\t -d number_of_directions (including trace, if any)
\t -e experiment_name (equals directory name for files)
\t -o output_pixel_size (in bytes)
\t -p input_pixel_size (in bytes)
\t -s number_of_slices
\t -t number_of_traces
\t -x size_x_dimension (in pixels)
\t -y size_y_dimension (in pixels)
\t -z number_of_b0_components
EOF
)

while getopts "b:d:e:o:p:s:t:x:y:z:" OPTION; do
    case $OPTION in
	b) 
	    number_of_b_values=$OPTARG
	    ;;
	d) 
	    number_of_directions=$OPTARG
	    ;;
	e)
	    experiment=$OPTARG
	    ;;
	o)  
	    output_pixel_size=$OPTARG
	    ;;
	p) 
	    input_pixel_size=$OPTARG
	    ;;
	s) 
	    number_of_slices=$OPTARG
	    ;;
	t) 
	    number_of_traces=$OPTARG
	    ;;
	x) 
	    size_x_dimension=$OPTARG
	    ;;
	y) 
	    size_y_dimension=$OPTARG
	    ;;
	z) 
	    number_of_b0_components=$OPTARG
	    ;;
	*) 
	    echo "Unrecognized option."
	    echo -e "$usage"
	    exit 1
	    ;;
    esac
done

if [ -z "$number_of_b_values" -o \
    -z "$number_of_directions" -o \
    -z "$experiment" -o \
    -z "$input_pixel_size" -o \
    -z "$output_pixel_size" -o \
    -z "$number_of_slices" -o \
    -z "$number_of_traces" -o \
    -z "$size_x_dimension" -o \
    -z "$size_y_dimension" -o \
    -z "$number_of_b0_components" ]; then
    echo "Missing parameters."
    echo -e "$usage"
    exit 1
fi

refined_directions=$(($number_of_directions-$number_of_traces))

echo "Supposing that $experiment/directions.txt and $experiment/directions.dat exist and are ok."

echo "Spliting data in b0 and remainder... "
./Bruker_split_b0_remainder.exe $experiment/${experiment}_acquisition.2dseq $number_of_b_values $number_of_directions $number_of_slices $input_pixel_size $number_of_b0_components $size_x_dimension $size_y_dimension $experiment/${experiment}_b0.raw $experiment/${experiment}_b0_remainder.raw $output_pixel_size
echo "Done."

echo "Splitting remainder in trace(s) and remainder... " 
./remove_trace.exe ${experiment}/${experiment}_b0_remainder.raw $number_of_b_values $number_of_directions $number_of_traces $number_of_slices $output_pixel_size $size_x_dimension $size_y_dimension ${experiment}/${experiment}_traces.raw ${experiment}/${experiment}_traces_remainder.raw
echo "Done."

echo "Splitting remainder in b-value files... "
./split_raw_by_b-value.exe ${experiment}/${experiment}_traces_remainder.raw $number_of_b_values $refined_directions $number_of_slices $output_pixel_size $size_x_dimension $size_y_dimension ${experiment}/${experiment}_b_value_series-
echo "Done."

echo "Appending the b0 data as a prefix in each b-value file... "
files=`ls -1 --color=never $experiment/${experiment}_b_value_series-*`
for file in $files; do
    file_name=`echo $file | sed 's/_b_value_series-\(.*\)/_b_value_series_result-\1/'`
    ./append_b0_in_front.exe ${experiment}/${experiment}_b0.raw $file $file_name
done

echo "Done. Final files have name prefix $experiment/${experiment}_b_value_series_result-."

exit

