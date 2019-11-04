#!/bin/bash
array=('a_generate_files' 'b_calcul_total_occurrences' 'c_calcul_frequences' 'd_calcul_frequences_tag_grams')
for i in "${array[@]}";do
	for file in "$i"/*;do
		if [[ -f $file ]]; then
		    dos2unix $file
		fi
	done
done

for file in *;do
	if [[ -f $file ]]; then
		dos2unix $file
	fi
done
