#include "fct_calcul_frequences.hpp"

using namespace std;
using namespace std::chrono;

mutex print_mutex;

void calcul_freq(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	string& path_to_output, unsigned long long& total_match, 
	unsigned long long& total_volume, unsigned nb_ngram)
{			
	string large_filename;	
	while( !queue_filenames.empty() )
	{
		if( !queue_filenames.try_pop(large_filename) )
			continue;
			
		FILE* input = fopen(large_filename.c_str(), "r");
		if( input == NULL )
		{
			print_message("Impossible to open the file ", large_filename);
			continue;	
		}
		
		print_message_safe(print_mutex, thread_id, "start", large_filename);
		
		// open the output file
		FILE* output = get_file(thread_id, large_filename, path_to_output, "_treated", "_frequences");
		if( output == NULL )
		{
			fclose(input);
			break;
		}
		
		treat_freq(input, output, large_filename, total_match, total_volume, nb_ngram);
		
		fclose(input);
		fclose(output);
		print_message_safe(print_mutex, thread_id, "finish", large_filename);
	}
}

void print_usage(const char* exename)
{
    fprintf(stderr, "NOM \n");
    fprintf(stderr, "\t%s - Calcul la fréquence de chaque ngrams avec des threads.\n\n", exename);
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
	unsigned nb_ngram;
	const char* ini_filename = argv[1];
	if( read_ini_file(ini_filename, total_occurrences_files, 
		path_to_treated_files, path_to_output_files, nb_ngram) )
	{
		cout << total_occurrences_files << endl;
		cout << path_to_treated_files << endl;
		cout << path_to_output_files << endl;
		cout << nb_ngram << endl;
	}
	else
		return -1;
	
	// Get the total occurrences and the total nb of volumes
	unsigned long long total_match, total_volume;
	if( !get_total_occurrences(total_occurrences_files.c_str(), 
		total_match, total_volume) )
		return -1;

	// Calculate the frequences for each file in the queue_filenames
	QueueSafe<string> queue_filenames;
	int nb_cores = std::thread::hardware_concurrency() - 1;
	if( nb_cores <= 0 )
		nb_cores = 5;
	vector<thread> threads;	
	collect_filenames(queue_filenames, path_to_treated_files, "_treated");
	for(unsigned i=0; i<(unsigned)nb_cores; ++i)
		threads.emplace_back( [&]{calcul_freq( i+1, queue_filenames, 
			path_to_output_files, total_match, total_volume, nb_ngram ); } );
	for(auto& t: threads)
		t.join();
	threads.clear();
	
	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}

