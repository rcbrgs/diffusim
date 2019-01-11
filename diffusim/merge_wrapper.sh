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

# sanity
if [ ! -f directions.txt ]; then
    echo "directions.txt doesn't exist!"
    exit 1
fi
if [ ! -f 000_merged.raw ]; then
    echo "000_merged.raw doesn't exist!"
    exit 1
fi

usage=$(
cat <<EOF 
USAGE: $0 <PARAMETERS>
PARAMETERS (all are obligatory):
\t -d number of directions
\t -n experiment name
\t -s number of slices
EOF
)

while getopts "d:n:s:" OPTION; do
    case $OPTION in
	d) 
	    number_of_directions=$OPTARG
	    ;;
	n) 
	    name=$OPTARG
	    ;;
	s) 
	    number_of_slices=$OPTARG
	    ;;
	*) 
	    echo "Unrecognized option."
	    echo -e "$usage"
	    exit 1
	    ;;
    esac
done

parameters="number_of_directions name number_of_slices"
for param in $parameters; do
    eval content=\$$param
    if [ -z "$content" ]; then
	echo "ERROR: Missing parameters."
	echo -e "$usage"
	exit 1
    fi
done

count=0
cd ~/latest/$name
mkdir adc_files
mv sample_adc_z*.bin adc_files/
mkdir components
mkdir dir_files
# put files for each direction in a single folder, and merge them in a component
while read line; do
    # prepare
    grad_x=`echo $line | awk '{ print $1 }'`
    grad_y=`echo $line | awk '{ print $2 }'`
    grad_z=`echo $line | awk '{ print $3 }'`
    files=`ls -1 --color=never ${name}_???_${grad_x}_${grad_y}_${grad_z}_att.raw | grep -v "^dir"`
    mkdir "dir_files/dir_${grad_x}_${grad_y}_${grad_z}"
    mv $files dir_files/dir_${grad_x}_${grad_y}_${grad_z}/
    cd dir_files/dir_${grad_x}_${grad_y}_${grad_z}/
    rename "s/_${grad_x}_${grad_y}_${grad_z}_att\.raw/_att\.raw/" *.raw
    rename "s/${name}_//" *.raw
    # merge
    #echo -e "($count)\t Merging files for direction ${grad_x}\t ${grad_y}\t ${grad_z} "
    printf "($count)\t Merging files for direction %+01.3f %+01.3f %+01.3f\n" ${grad_x} ${grad_y} ${grad_z}
    padded=`python -c "print '%03d' % $count"`
    ../../merge_clustered.exe ${padded}_mer.raw _att $number_of_slices
    mv ${padded}_mer.raw ../../components
    count=$(($count+1))
    cd ../..
done < directions.txt

echo "Merging the b0 with the first component..."
mv components/000_mer.raw components/000_mer_original.raw
ln -s components/000_mer_original.raw 001_merged.raw
./merge_clustered.exe 000_mer.raw _merged 2
mv 000_mer.raw components/
echo "Merging the components..."
cd components
../merge_clustered.exe ${name}_merged.raw _mer $number_of_directions
cd ..
mv components/${name}_merged.raw ${name}_synthetic.raw

# clean up
cd ..
echo "Done!"
exit 0
