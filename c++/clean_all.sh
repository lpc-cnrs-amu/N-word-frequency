#!/bin/bash
array=('a_generate_files' 'b_calcul_total_occurrences' 'c_calcul_frequences' 'd_calcul_frequences_tag_grams')
for i in "${array[@]}";do
	cd $i;make rmproper;cd ..;
done
