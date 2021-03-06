#include "fct_take_n_most_frequencies_ngram.hpp"

using namespace std;
using namespace std::chrono;

bool calcul_most_frequent(string large_filename, 
	map<string, unsigned long long>& most_frequent_ngrams, unsigned long long nb_sentences,
	string& key_min, unsigned long long& freq_min, unsigned nb_ngram)
{
	FILE* input = fopen(large_filename.c_str(), "r");
	if( input == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	print_message("start", large_filename);
	
	treat_most_freq(input, large_filename, most_frequent_ngrams, 
		nb_sentences, key_min, freq_min, nb_ngram);
	
	fclose(input);
	print_message("finish", large_filename);
	return true;
}


void calcul_handler(vector<string>& filenames, 
	map<string, unsigned long long>& most_frequent_ngrams, unsigned long long nb_sentences,
	unsigned nb_ngram)
{
	string key_min("");
	unsigned long long freq_min(ULLONG_MAX);
	for(unsigned i=0; i<filenames.size(); ++i)
		if( !calcul_most_frequent(filenames[i], most_frequent_ngrams, 
			nb_sentences, key_min, freq_min, nb_ngram) )
			cerr << "didn't process the file " << filenames[i] << "\n";
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
	unsigned long long nb_sentences;
	unsigned nb_ngram;
	const char* ini_filename = argv[1];
	if( read_ini_file(ini_filename, output_file_name, 
		path_to_frequences_files, nb_sentences, nb_ngram) )
	{
		cout << output_file_name << endl;
		cout << path_to_frequences_files << endl;
		cout << nb_sentences << endl;
		cout << nb_ngram << endl;
	}
	else
		return -1;
	
	// Calculate the total nb of occurrences with treated files
	vector<string> filenames;
	map<string, unsigned long long> most_frequent_ngrams;
	collect_filenames(filenames, path_to_frequences_files, "_frequences");
	calcul_handler(filenames, most_frequent_ngrams, nb_sentences, nb_ngram);
	
	// Write in output file
	write_output(output_file_name.c_str(), most_frequent_ngrams);
	
	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}
