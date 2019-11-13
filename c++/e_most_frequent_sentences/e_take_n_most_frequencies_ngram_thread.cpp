#include "fct_take_n_most_frequencies_ngram.hpp"

// Prendre les N phrases les plus fréquentes parmi tous les fichiers de fréquences
using namespace std;
using namespace std::chrono;

mutex print_mutex;
mutex map_mutex;

void calcul_most_frequent(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	map<string, long double>& most_frequent_ngrams, unsigned nb_sentences, 
	string& key_min, long double& freq_min)
{
	string large_filename;
			
	while( !queue_filenames.empty() )
	{
		if( !queue_filenames.try_pop(large_filename) )
			continue;
			
		FILE* input = fopen(large_filename.c_str(), "r");
		if( input == NULL )
		{
			print_message_safe(print_mutex, thread_id, "Impossible to open the file", large_filename);
			continue;	
		}
		print_message_safe(print_mutex, thread_id, "start", large_filename);
		
		map<string, long double> most_frequent_ngrams_tmp;
		string key_min_tmp(""); 
		long double freq_min_tmp(999999);
		treat_most_freq(input, large_filename, most_frequent_ngrams_tmp, 
			nb_sentences, key_min_tmp, freq_min_tmp);
		
		update_most_frequent_ngrams_safe(map_mutex,
			most_frequent_ngrams, most_frequent_ngrams_tmp, nb_sentences,
			key_min, freq_min);
		most_frequent_ngrams_tmp.clear();
		fclose(input);
		print_message_safe(print_mutex, thread_id, "finish", large_filename);
	}
}


int main(int argc, char** argv)
{
	// print usage if the user has written the command incorrectly
	if( argc > 2 || (argc > 1 && !strcmp(argv[1],"-h")) )
	{
		//print_usage(argv[0]);
		return 0;
	}
	auto start = high_resolution_clock::now();
	
	// Read ini file to find args
	string output_file_name, path_to_frequences_files;
	unsigned nb_sentences;
	const char* ini_filename = argv[1];
	if( read_ini_file(ini_filename, output_file_name, 
		path_to_frequences_files, nb_sentences) )
	{
		cout << output_file_name << endl;
		cout << path_to_frequences_files << endl;
		cout << nb_sentences << endl;
	}
	else
		return -1;
	
	// Calculate the total nb of occurrences with treated files
	QueueSafe<string> queue_filenames;
	map<string, long double> most_frequent_ngrams;
	string key_min("");
	long double freq_min(999999);
	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;
	collect_filenames(queue_filenames, path_to_frequences_files, "_frequences");
	for(unsigned i=0; i<nb_cores; ++i)
		threads.emplace_back( [&]{calcul_most_frequent( i+1, 
			queue_filenames, most_frequent_ngrams, nb_sentences,
			key_min, freq_min); } );
	for(auto& t: threads)
		t.join();
	threads.clear();
	
	// Write in output file
	write_output(output_file_name.c_str(), most_frequent_ngrams);
	
	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}
