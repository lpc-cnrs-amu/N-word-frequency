#ifndef FCT_GENERATE_FILES_HPP
#define FCT_GENERATE_FILES_HPP

#include "../zlib1211/zlib.h"
#include "../util/util.hpp"

void update_args(std::ifstream& file_ini, std::string& line, std::string& path_to_gz, 
	std::string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined);
	
bool read_ini_file(const char* ini_filename, std::string& path_to_gz, 
	std::string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined);
	
void treat_line(FILE* output, std::string& ngram, std::string& precedent_ngram, unsigned& somme_year,
	unsigned& somme_nb_match, unsigned& somme_nb_volume, float& mean_pondere_match,
	float& mean_pondere_volume,	unsigned& year_max,	unsigned& year_min,
	unsigned& nb_match_max,	unsigned& nb_match_min,	unsigned& nb_volume_max,
	unsigned& nb_volume_min, unsigned year, unsigned nb_match, unsigned nb_volume);
	
bool file_not_entirely_read(std::stringstream& token);

void treat_file(int thread_id, gzFile large_file, FILE* output, std::string large_filename,
	std::vector<std::string>& forbidden_characters, std::vector<std::string>& accepted_tags, 
	unsigned nb_ngrams, unsigned min_year_defined);

#endif
