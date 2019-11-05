#ifndef FCT_TAKE_N_MOST_FREQUENCIES_HPP
#define FCT_TAKE_N_MOST_FREQUENCIES_HPP

#include "../util/util.hpp"

void update_args(std::ifstream& file_ini, std::string& line, std::string& output_file_name, 
	std::string& path_to_frequences_files, unsigned& nb_sentences);

bool read_ini_file(const char* ini_filename, std::string& output_file_name, 
	std::string& path_to_frequences_files, unsigned& nb_sentences);

void def_new_min(std::map<string, float>& most_frequent_ngrams,
	std::string& key_min, float& freq_min);

void compare_freq(std::map<std::string, float>& most_frequent_ngrams, char* buffer, 
	float freq_match, unsigned nb_sentences, std::string& key_min, float& freq_min);

void treat_most_freq(FILE* input, std::string large_filename, 
	std::map<std::string, float>& most_frequent_ngrams, unsigned nb_sentences);

#endif
