#!/bin/bash
array=('a_generate_files' 'b_calcul_total_occurrences' 'c_calcul_frequences')
for i in "${array[@]}";do
	cd $i;make;cd ..;
done
