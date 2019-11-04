#include "fct_calcul_frequences.hpp"

using namespace std;
using namespace std::chrono;


bool calcul_freq(string large_filename, string& path_to_output,
	unsigned long long& total_match, unsigned long long& total_volume)
{					
	FILE* input = fopen(large_filename.c_str(), "r");
	if( input == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	print_message("start", large_filename);
	
	// open the output file
	FILE* output = get_file(-1, large_filename, path_to_output, "_treated", "_frequences");
	if( output == NULL )
	{
		fclose(input);
		return false;
	}
				
	treat_freq(input, output, large_filename, total_match, total_volume);
	
	fclose(input);
	fclose(output);
	print_message("finish", large_filename);
	return true;
}
	
void print_usage(const char* exename)
{
    fprintf(stderr, "NOM \n");
    fprintf(stderr, "\t%s - Calcul la fréquence de chaque ngrams.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] fichier_total_occurrences\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tCalcule la fréquence de chaque ngrams des fichiers finissant par '_treated' contenus dans le répertoire de votre choix. \
    Utilise le fichier fichier_total_occurrences contenant le nombre total d'occurrences et de volumes. Ce fichier est obtenu avec le programme b_calcul_total_occurrences.cpp\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tAffiche un message d'aide sur la sortie d'erreur et termine normalement.\n\n");
    fprintf(stderr, "\t fichier_total_occurrences\n\t\tContient le nombre total d'occurrences et de volumes. Ce fichier est obtenu avec le programme b_calcul_total_occurrences.cpp\n\n");
    fprintf(stderr, "\t chemin_fichiers_treated\n\t\tLe chemin pour accéder aux fichier _treated.\n\n");
    fprintf(stderr, "\t chemin_fichiers_sorties\n\t\tLe chemin du répertoire où se trouveront les fichiers de sorties _frequences.\n\n");
}

void calcul_handler(vector<string>& filenames,string& path_to_output, 
	unsigned long long& total_match, unsigned long long& total_volume)
{
	for(unsigned i=0; i<filenames.size(); ++i)
		if( !calcul_freq(filenames[i], path_to_output, total_match, total_volume) )
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
	string total_occurrences_files, path_to_treated_files, path_to_output_files;
	const char* ini_filename = argv[0];
	if( argc <= 1 )
		ini_filename = NULL;
	if( read_ini_file(ini_filename, total_occurrences_files, 
		path_to_treated_files, path_to_output_files) )
	{
		cout << total_occurrences_files << endl;
		cout << path_to_treated_files << endl;
		cout << path_to_output_files << endl;
	}
	else
		return -1;
	
	// Get the total occurrences and the total nb of volumes
	unsigned long long total_match, total_volume;
	if( !get_total_occurrences(total_occurrences_files.c_str(), 
		total_match, total_volume) )
		return -1;

	// Calculate the frequences for each file
	vector<string> filenames;
	collect_filenames(filenames, path_to_treated_files, "_treated");
	calcul_handler( filenames, path_to_output_files, total_match, total_volume );

	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}

