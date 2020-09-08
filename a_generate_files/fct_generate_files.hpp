#ifndef FCT_GENERATE_FILES_HPP
#define FCT_GENERATE_FILES_HPP

#include "../zlib-1.2.11/zlib.h"
#include "../util/util.hpp"
#include <regex>

void update_args(std::ifstream& file_ini, std::string& line, std::string& path_to_gz, 
	std::string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined, bool& no_number);
	
bool read_ini_file(const char* ini_filename, std::string& path_to_gz, 
	std::string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined, bool& no_number);
	
bool file_not_entirely_read(std::stringstream& token, std::string large_filename);

void treat_file(int thread_id, gzFile large_file, FILE* output, std::string large_filename,
	std::vector<std::string>& forbidden_characters, std::vector<std::string>& accepted_tags, 
	unsigned nb_ngrams, unsigned min_year_defined, bool no_number);

#endif
