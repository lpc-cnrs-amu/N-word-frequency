#include "fct_calcul_total_occurrences.hpp"

using namespace std;
using namespace std::chrono;

bool calcul_occurrences(string large_filename,
	unsigned long long& total_match, unsigned nb_ngram)
{			
	FILE* input = fopen(large_filename.c_str(), "r");
	if( input == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	print_message("start", large_filename);
							
	treat_occurrences(input, large_filename, total_match, nb_ngram);
	
	fclose(input);
	print_message("finish", large_filename);
	return true;
	
}

void print_usage(const char* exename)
{
    fprintf(stderr, "NOM \n");
    fprintf(stderr, "\t%s - Calcul le nombre total de ngrams (somme des occurrences des ngrams).\
     Calcul le nombre total de volumes avec le fichier totalcounts. Ecrit ces deux résultats dans un fichier de sortie.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] nom_du_fichier_de_sortie fichier_totalcount chemin_fichiers_treated\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tCalcule le nombre total de ngrams à l'aide des fichiers finissant par '_treated' contenus dans le répertoire de votre choix.\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tAffiche un message d'aide sur la sortie d\"erreur et termine normalement.\n\n");
    fprintf(stderr, "\t nom_du_fichier_de_sortie\n\t\tContiendra le nombre total d'occurrences et de volumes.\n\n");
    fprintf(stderr, "\t fichier_totalcount\n\t\tSert à calculer le nombre total de volumes.\n\n");
    fprintf(stderr, "\t chemin_fichiers_treated\n\t\tLe chemin pour accéder aux fichier _treated.\n\n");
}

void calcul_handler(vector<string>& filenames, 
	unsigned long long& total_match, unsigned nb_ngram)
{
	for(unsigned i=0; i<filenames.size(); ++i)
		if( !calcul_occurrences(filenames[i], total_match, nb_ngram) )
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
	unsigned min_year_defined, nb_ngram;
	const char* ini_filename = argv[1];
	if( read_ini_file(ini_filename, output_file_name, totalcount_file, 
		path_to_treated_files, min_year_defined, nb_ngram) )
	{
		cout << output_file_name << endl;
		cout << totalcount_file << endl;
		cout << path_to_treated_files << endl;
		cout << min_year_defined << endl;
		cout << nb_ngram << endl;
	}
	else
		return -1;
	
	// Calculate the total nb of volumes with the totalcount file
	unsigned long long total_volume = 0;
	get_total_volume(totalcount_file.c_str(), total_volume, min_year_defined);
	
	// Calculate the total nb of occurrences with treated files
	vector<string> filenames;
	unsigned long long total_match = 0;
	collect_filenames(filenames, path_to_treated_files, "_treated");
	calcul_handler(filenames, total_match, nb_ngram);
	
	// Write in output file
	write_output(output_file_name.c_str(), total_match, total_volume);
	
	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}

