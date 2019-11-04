#include "fct_calcul_frequences_tag_grams.hpp"

using namespace std;
using namespace std::chrono;

bool calcul_occurrences(string large_filename, map<std::string, Data*>& tags_to_data, 
	vector<string>& accepted_tags, unsigned nb_ngrams)
{	
	
	FILE* input = fopen(large_filename.c_str(), "r");
	if( input == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	print_message("start", large_filename);
							
	treat_occurrences_tag_grams(tags_to_data, accepted_tags, 
		input, nb_ngrams, large_filename);

	fclose(input);
	print_message("finish", large_filename);
	return true;
}

void print_usage(const char* exename)
{
    fprintf(stderr, "NOM \n");
    fprintf(stderr, "\t%s - Calcule la fréquences des tag-grams vu dans les fichiers _treated.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] nom_du_fichier_de_sortie fichier_totalcount chemin_fichiers_treated\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tCalcule la fréquences des tag-grams vu dans les fichiers _treated contenus dans le répertoire de votre choix.\
    Chaque ligne du fichier de sortie sera comme ceci :\n\
    <tag-gram> <nb d'année> <nb d'années> <total occurrences> <total volume> <moyenne des années pondérés par le total d'occurrences> \
    <moyenne des années pondérés par le total de volume> <année max> <année min> <occurrences max> <occurences min> <nb volume max> <nb volume min> <freq match> <freq volume>\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tAffiche un message d'aide sur la sortie d\"erreur et termine normalement.\n\n");
    fprintf(stderr, "\t nom_du_fichier_de_sortie\n\t\tContiendra la fréquence pour chaque tag-grams.\n\n");
    fprintf(stderr, "\t fichier_totalcount\n\t\tSert à calculer le nombre total de volumes.\n\n");
    fprintf(stderr, "\t chemin_fichiers_treated\n\t\tLe chemin pour accéder aux fichier _treated.\n\n");
}

void calcul_handler(vector<string>& filenames, map<std::string, Data*>& tags_to_data, 
	vector<string>& accepted_tags, unsigned nb_ngrams)
{
	for(unsigned i=0; i<filenames.size(); ++i)
		if( !calcul_occurrences(filenames[i], tags_to_data, accepted_tags, nb_ngrams) )
			cerr << "didn't process the file " << filenames[i] << "\n";
}

int main(int argc, char** argv)
{
	// print usage if the user has written the command incorrectly
	if( argc > 2 || (argc > 1 && !strcmp(argv[1],"-h")) )
	{
		print_usage(argv[0]);
		return 0;
	}
	auto start = high_resolution_clock::now();
	
	// Read ini file to find args
	string output_file_name, totalcount_file, path_to_treated_files;
	unsigned nb_ngrams, min_year_defined;
	const char* ini_filename = argv[1];
	if( read_ini_file(ini_filename, output_file_name, totalcount_file, 
		path_to_treated_files, nb_ngrams, min_year_defined) )
	{
		cout << output_file_name << endl;
		cout << totalcount_file << endl;
		cout << path_to_treated_files << endl;
		cout << nb_ngrams << endl;
		cout << min_year_defined << endl;
	}
	else
		return -1;
	
	// Calculate the total nb of volumes with the totalcount file
	unsigned long long total_volume, total_match = 0;
	get_total_volume(totalcount_file.c_str(), total_volume, min_year_defined);
	
	// Calculate the total nb of occurrences of tags_grams with treated files
	vector<string> filenames;
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT"};
	map<std::string, Data*> tags_to_data;
	collect_filenames(filenames, path_to_treated_files, "_treated");
	calcul_handler(filenames, tags_to_data, accepted_tags, nb_ngrams);
	
	// Write in output file	frequences for each tags-grams
	calcul_total_occurrences(tags_to_data, total_match);
	write_output_frequences_tags_grams(output_file_name.c_str(), 
		total_match, total_volume, tags_to_data);
	
	destroy_data(tags_to_data);
	
	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}

