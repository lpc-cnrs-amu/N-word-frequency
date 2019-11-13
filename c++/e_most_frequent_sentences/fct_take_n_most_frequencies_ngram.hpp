#ifndef FCT_TAKE_N_MOST_FREQUENCIES_HPP
#define FCT_TAKE_N_MOST_FREQUENCIES_HPP

#include "../util/util.hpp"
#include <map>

void update_args(std::ifstream& file_ini, std::string& line, std::string& output_file_name, 
	std::string& path_to_frequences_files, unsigned& nb_sentences);

bool read_ini_file(const char* ini_filename, std::string& output_file_name, 
	std::string& path_to_frequences_files, unsigned& nb_sentences);

void def_new_min(std::map<std::string, long double>& most_frequent_ngrams,
	std::string& key_min, long double& freq_min);

void compare_freq(std::map<std::string, long double>& most_frequent_ngrams, std::string new_key, 
	long double freq, unsigned nb_sentences, std::string& key_min, long double& freq_min);

void treat_most_freq(FILE* input, std::string large_filename, 
	std::map<std::string, long double>& most_frequent_ngrams, unsigned nb_sentences,
	std::string& key_min, long double& freq_min);
	
void update_most_frequent_ngrams_safe(std::mutex& map_mutex, 
	std::map<std::string, long double>& most_frequent_ngrams, 
	std::map<std::string, long double>& most_frequent_ngrams_tmp, 
	unsigned nb_sentences, std::string& key_min, long double& freq_min);
	
bool write_output(const char* filename, 
	std::map<std::string, long double>& most_frequent_ngrams);

#endif
