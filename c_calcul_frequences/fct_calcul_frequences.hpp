#ifndef FCT_CALCUL_FREQUENCES_HPP
#define FCT_CALCUL_FREQUENCES_HPP

#include "../util/util.hpp"

void treat_freq(FILE* input, FILE* output, std::string& large_filename, 
	unsigned long long& total_match, unsigned long long& total_volume, unsigned nb_ngram);
	
void update_args(std::ifstream& file_ini, std::string& line, std::string& total_occurrences_files, 
	std::string& path_to_treated_files, std::string& path_to_output_files, unsigned& nb_ngram);

bool read_ini_file(const char* ini_filename, std::string& total_occurrences_files, 
	std::string& path_to_treated_files, std::string& path_to_output_files, unsigned& nb_ngram);

#endif
