// Ex how to launch it : ./frequences_thread ../files_treated/eng_test.txt ../files_treated/ ../files_frequences/


#include "../util/util.hpp"

#define LINE_SIZE 1024
#define NB_NGRAM 4

using namespace std;
using namespace std::chrono;

mutex print_mutex;

void calcul_freq(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	const char* path_to_output, unsigned long long& total_match, 
	unsigned long long& total_volume)
{	
	char buffer[LINE_SIZE];
	string line;
	string token;
	string delimiter = "\t";
	string large_filename;
	unsigned position;
	unsigned cpt_line;
	float freq_match;
	float freq_volume;
	size_t pos;
			
	while( !queue_filenames.empty() )
	{
		if( !queue_filenames.try_pop(large_filename) )
			continue;
			
		FILE* input = fopen(large_filename.c_str(), "r");
		if( input == NULL )
		{
			print_message("Impossible to open the file ", large_filename);
			break;	
		}
		
		print_ok_safe(print_mutex, thread_id, "start", large_filename);
		
		// open the output file
		FILE* output = get_file(thread_id, large_filename, path_to_output, "_treated", "_frequences");
		if( output == NULL )
		{
			fclose(input);
			break;
		}
		
		freq_match = 0;
		freq_volume = 0;		
		cpt_line = 0;		
		while( fgets(buffer, sizeof(buffer), input) )
		{
			++cpt_line;
			line = buffer;
			position = 0;
			pos = 0;
						
			// cut by \t
			while ((pos = line.find(delimiter)) != std::string::npos) 
			{
				++ position;
				token = line.substr(0, pos);
				line.erase(0, pos + delimiter.length());
			
				if(position == 3)
					freq_match = stoi( token ) / (total_match*0.1);
				else if(position == 4)
					freq_volume = stoi( token ) / (total_volume*0.1);
			}
			if( line != "" )
				++ position;
			
			if( position != 12 )
			{
				cout << "WARNING bad line (" << cpt_line 
					 << ") on file " << large_filename << " : " << buffer << "\n";
				cerr << "WARNING bad line (" << cpt_line 
					 << ") on file " << large_filename << " : " << buffer << "\n";
			}
			else
			{
				fprintf(output, "%s\t%.8e\t%.8e\n", strtok(buffer, "\n"), freq_match, freq_volume);
			}
			memset(buffer, 0, sizeof(buffer));
		}
		fclose(input);
		fclose(output);
		print_ok_safe(print_mutex, thread_id, "finish", large_filename);
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
	if( argc != 4 || (argc > 1 && !strcmp(argv[1],"-h")) )
	{
		print_usage(argv[0]);
		return 0;
	}	
	auto start = high_resolution_clock::now();
	
	// Get the total occurrences and the total nb of volumes
	unsigned long long total_match, total_volume;
	if( !get_total_occurrences(argv[1], total_match, total_volume) )
		return -1;

	// Calculate the frequences for each file in the queue_filenames
	QueueSafe<string> queue_filenames;
	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;	
	collect_filenames(queue_filenames, argv[2], "_treated");
	for(unsigned i=0; i<nb_cores; ++i)
	{
		//cout << "create thread " << i+1 << endl;
		threads.emplace_back( [&]{calcul_freq( i+1, queue_filenames, argv[3], total_match, total_volume ); } );
	}
	for(auto& t: threads)
		t.join();
	threads.clear();
	

	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl;
	
    return 0;
}

