#ifndef FCT_GENERATE_FILES_HPP
#define FCT_GENERATE_FILES_HPP

#include "../util/util.hpp"

void update_args(std::ifstream& file_ini, std::string& line, std::string& path_to_gz, 
	std::string& path_to_output, unsigned& nb_ngram, unsigned& min_year);
	
bool read_ini_file(const char* ini_filename, std::string& path_to_gz, 
	std::string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined);
	
void treat_line(FILE* output, std::string& ngram, std::string& precedent_ngram, unsigned& somme_year,
	unsigned& somme_nb_match, unsigned& somme_nb_volume, float& mean_pondere_match,
	float& mean_pondere_volume,	unsigned& year_max,	unsigned& year_min,
	unsigned& nb_match_max,	unsigned& nb_match_min,	unsigned& nb_volume_max,
	unsigned& nb_volume_min, unsigned year, unsigned nb_match, unsigned nb_volume);
	
bool file_not_entirely_read(std::stringstream& token);

#endif
