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

source=`pwd`

usage=$(
cat <<EOF 
USAGE: $0 <PARAMETERS>
PARAMETERS:
\t -c name of file containg cluster information for computing slices in parallel
\t -d number of gradient directions (*)
\t -e experiment_name (*)
\t -s number of slices (*)
\t -t number of steps per second (*)
\t -u direction uncertainty percentage
(*) Indicates an obligatory option.
EOF
)

direction_uncertainty_percentage=0

while getopts "c:d:e:s:t:u:" OPTION; do
    case $OPTION in
	c)  cluster_info=$OPTARG
	    ;;
	d)  gradient_directions=$OPTARG
	    ;;
	e)  experiment_name=$OPTARG
	    ;;
	s)  slices=$OPTARG
	    ;;
	t)  steps_per_second=$OPTARG
	    ;;
	u)  direction_uncertainty_percentage=$OPTARG
	    ;;
	*) 
	    echo "Unrecognized option."
	    echo -e "$usage"
	    exit 1
	    ;;
    esac
done

parameters="gradient_directions experiment_name slices steps_per_second"
for param in $parameters; do
    eval content=\$$param
    if [ -z "$content" ]; then
	echo "ERROR: Missing parameters."
	echo -e "$usage"
	exit 1
    fi
done

clusterize=0
if [ ! "$cluster_info" == "" ]; then
    clusterize=1
    cluster_host=`grep cluster_host $cluster_info | sed 's/^.*cluster_host.*=//'`
fi

if [ -e ~/latest/$experiment_name ]; then
    rm -rf ~/latest/$experiment_name
fi

mkdir -p ~/latest/$experiment_name
cd ~/latest/$experiment_name

cp $source/$experiment_name.xml .
cp $source/b0.raw .
cp $source/mask_generator.exe .

./mask_generator.exe b0.raw $experiment_name.xml $direction_uncertainty_percentage

rm $experiment_name.xml
rm b0.raw
rm buffer.raw
rm mask_generator.exe
rm mask_x.raw
rm mask_y.raw
rm mask_z.raw

cp $source/batch_wrapper.sh .
cp $source/directions.txt .
cp $source/job.sh .
cp $source/job_wrapper.sh .
cp $source/stejskal_clustered.exe .
cp $source/stejskal_clustered.sh .

if [ $clusterize -eq 1 ]; then
    echo "Running in clusterized mode."
    echo -e "#!/bin/bash\ncd ~/latest/$experiment_name\n./batch_wrapper.sh -e $experiment_name -s $slices -t $steps_per_second \necho 'Press enter after all jobs have run.'\nread\nmv ~/latest/*.raw ~/latest/$experiment_name/\ncat job.sh.e*\necho 'Press enter if no errors or no important errors.'\nread\nrm job.sh.*" > run_me_on_cluster.sh
    chmod 700 run_me_on_cluster.sh
    
    rsync -avz ~/latest/$experiment_name/ $cluster_host:latest/$experiment_name/
    ssh $cluster_host "cd ~/latest/$experiment_name; ./run_me_on_cluster.sh"
    rsync -avz $cluster_host:latest/$experiment_name/ ~/latest/$experiment_name/
    ssh $cluster_host "rm -rf ~/latest/$experiment_name"
    rm batch_wrapper.sh
    rm job.sh
    rm job_wrapper.sh
    rm stejskal_clustered.exe
    rm stejskal_clustered.sh
    rm run_me_on_cluster.sh
else
    echo "Running in local, single mode."
    slice0_time_start=`date +"%s"`
    slice0_time_end=0
    slices_times_dirs_to_go=$(($gradient_directions * $slices))
    sample_files=`ls -1 --color=never sample_adc*.bin | sed 's/^.*\///'`
    for sample in $sample_files; do
	number=`echo $sample | sed 's/sample_adc_z//' | sed 's/.bin//'`
	while read line; do
	    if [ ! $slice0_time_end -eq 0 ]; then
		seconds_to_go=$(($slices_times_dirs_to_go * $time_cost))
		hours_to_go=$(($seconds_to_go / 3600))
		seconds_to_go=$(($seconds_to_go % 3600))
		minutes_to_go=$(($seconds_to_go / 60))
		seconds_to_go=$(($seconds_to_go % 60))
		printf "ETA for $slices_times_dirs_to_go chunks: %02d:%02d:%02d. " $hours_to_go $minutes_to_go $seconds_to_go
	    fi
	    grad_x=`echo $line | awk '{ print $1 }'`
	    grad_y=`echo $line | awk '{ print $2 }'`
	    grad_z=`echo $line | awk '{ print $3 }'`
	    echo "Synthesizing slice $number of direction $grad_x $grad_y $grad_z."
	    ./stejskal_clustered.sh -i $sample -o ${experiment_name}_${number}_${grad_x}_${grad_y}_${grad_z} -x $grad_x -y $grad_y -z $grad_z -s $steps_per_second
	    if [ $slice0_time_end -eq 0 ]; then
		slice0_time_end=`date +"%s"`
		time_cost=$(($slice0_time_end - $slice0_time_start))
		if [ $time_cost -eq 0 ]; then
		    time_cost=1
		fi
	    fi
	    slices_times_dirs_to_go=$(($slices_times_dirs_to_go - 1))
	done < directions.txt
    done
fi

cp $source/000_merged.raw .
cp $source/directions.dat .
cp $source/merge_clustered.exe .
cp $source/merge_wrapper.sh .

./merge_wrapper.sh -d $gradient_directions -n $experiment_name -s $slices

echo "Press ENTER to cleanup local temporary directory."
read
rm 000_merged.raw
rm 001_merged.raw
rm -rf adc_files
rm -rf components
rm -rf dir_files
rm merge_clustered.exe
rm merge_wrapper.sh

if [ -e $source/$experiment_name ]; then
    rm -rf $source/$experiment_name
fi

mv ~/latest/$experiment_name $source/

exit 0
