#ifndef FCT_CALCUL_FREQUENCES_TAG_GRAMS_HPP
#define FCT_CALCUL_FREQUENCES_TAG_GRAMS_HPP

#include <map>
#include "../util/util.hpp"
#include "../util/Data.hpp"
#include "../zlib1211/zlib-1.2.11/zlib.h"

bool write_output_frequences_tags_grams(const char* filename, unsigned long long total_match, 
	unsigned long long total_volume, std::map<std::string, Data*>& tags_to_data);

bool collect_tags(std::string token, std::string& tag, 
	std::vector<std::string>& accepted_tags, unsigned nb_ngrams);

bool valid_line(std::string line, std::string& tag, 
	unsigned long long& year, unsigned long long& nb_match, unsigned long long& nb_volume, 
	std::vector<std::string>& accepted_tags, unsigned nb_ngram, unsigned min_year_defined);
	
void update_tag_grams(std::map<std::string, Data*>& tags_to_data, std::string tags, 
	unsigned long long year, unsigned long long nb_match,
	unsigned long long nb_volume);

void update_tag_grams_safe_bis(std::map<std::string, Data*>& tags_to_data, std::string tags, 
	unsigned long long nb_year, unsigned long long nb_match, 
	unsigned long long nb_volume, unsigned long long mean_pondere_match, 
	unsigned long long mean_pondere_volume, unsigned year_max, unsigned year_min,
	unsigned long long nb_match_max, unsigned long long nb_match_min, 
	unsigned long long nb_volume_max, unsigned long long nb_volume_min);
	
void update_tag_grams_safe(std::mutex& map_mutex, std::map<std::string, Data*>& tags_to_data,
	std::map<std::string, Data*>& tags_to_data_file);

void treat_occurrences_tag_grams(int thread_id, std::map<std::string, Data*>& tags_to_data, 
	std::vector<std::string>& accepted_tags, gzFile large_file, 
	unsigned nb_ngrams, unsigned min_year_defined, std::string& large_filename);
 
void calcul_total_occurrences(std::map<std::string, Data*>& tags_to_data, 
	unsigned long long& total_match);

void destroy_data(std::map<std::string, Data*>& tags_to_data);

void update_args(std::ifstream& file_ini, std::string& line, std::string& output_file_name, 
	std::string& totalcount_file, std::string& path_to_gz_pos_files,
	unsigned& nb_ngrams, unsigned& min_year_defined);

bool read_ini_file(const char* ini_filename, std::string& output_file_name, 
	std::string& totalcount_file, std::string& path_to_gz_pos_files,
	unsigned& nb_ngrams, unsigned& min_year_defined);


#endif
