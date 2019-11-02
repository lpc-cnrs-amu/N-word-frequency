#ifndef FCT_CALCUL_TOTAL_OCCURRENCES_HPP
#define FCT_CALCUL_TOTAL_OCCURRENCES_HPP

#include "../util/util.hpp"

void update_args(std::ifstream& file_ini, std::string& line, std::string& output_file_name, 
	std::string& totalcount_file, std::string& path_to_treated_files, 
	unsigned& nb_ngrams, unsigned& min_year_defined);

bool read_ini_file(const char* ini_filename, std::string& output_file_name, 
	std::string& totalcount_file, std::string& path_to_treated_files, 
	unsigned& nb_ngrams, unsigned& min_year_defined);
	
bool write_output(const char* filename, unsigned long long total_match, 
	unsigned long long total_volume);
	
void treat_occurrences(FILE* input, std::string& large_filename, 
	unsigned long long& total_match);
	
void treat_occurrences(FILE* input, std::string& large_filename, 
	unsigned long long& total_match, unsigned nb_ngrams, unsigned min_year_defined);
	
#endif
