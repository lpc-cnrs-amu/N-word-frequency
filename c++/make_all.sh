#!/bin/bash
array=('a_generate_files' 'b_calcul_total_occurrences' 'c_calcul_frequences' 'd_calcul_frequences_tag_grams' 'e_most_frequent_sentences')
for i in "${array[@]}";do
	cd $i;make;cd ..;
done
