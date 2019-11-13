#ifndef FCT_CALCUL_FREQUENCES_TAG_GRAMS_HPP
#define FCT_CALCUL_FREQUENCES_TAG_GRAMS_HPP

#include <map>
#include "../util/util.hpp"
#include "../util/Data.hpp"

bool write_output_frequences_tags_grams(const char* filename, unsigned long long total_match, 
	unsigned long long total_volume, std::map<std::string, Data*>& tags_to_data);

bool collect_tags(std::string token, std::string& tag, 
	std::vector<std::string>& accepted_tags, unsigned nb_ngrams);

bool update_elements(unsigned position, bool& good_tags, std::string token, 
	std::string& tags, std::vector<std::string>& accepted_tags, unsigned long long& nb_year,
	unsigned long long& nb_match, unsigned long long& nb_volume, double& mean_pondere_match,
	double& mean_pondere_volume, unsigned& year_max, unsigned& year_min,
	unsigned long long& nb_match_max, unsigned long long& nb_match_min, 
	unsigned long long& nb_volume_max, unsigned nb_ngrams);

void update_tag_grams(std::map<std::string, Data*>& tags_to_data, std::string tags, 
	unsigned long long nb_year, unsigned long long nb_match, unsigned long long nb_volume, double mean_pondere_match,
	double mean_pondere_volume, unsigned year_max, unsigned year_min,
	unsigned long long nb_match_max, unsigned long long nb_match_min, 
	unsigned long long nb_volume_max, unsigned long long nb_volume_min);
	
void update_tag_grams(std::map<std::string, Data*>& tags_to_data, std::string tags, 
	unsigned long long nb_year, unsigned long long nb_match, 
	unsigned long long nb_volume, unsigned year_max, unsigned year_min,
	unsigned long long nb_match_max, unsigned long long nb_match_min, 
	unsigned long long nb_volume_max, unsigned long long nb_volume_min);
	
void update_tag_grams_safe(std::mutex& map_mutex, std::map<std::string, Data*>& tags_to_data,
	std::map<std::string, Data*>& tags_to_data_file);
	
void treat_occurrences_tag_grams(std::map<std::string, Data*>& tags_to_data, 
	std::vector<std::string>& accepted_tags, FILE* input, unsigned nb_ngrams, 
	std::string& large_filename);
	
void calcul_total_occurrences(std::map<std::string, Data*>& tags_to_data, 
	unsigned long long& total_match);

void destroy_data(std::map<std::string, Data*>& tags_to_data);

void update_args(std::ifstream& file_ini, std::string& line, std::string& output_file_name, 
	std::string& totalcount_file, std::string& path_to_treated_files,
	unsigned& nb_ngrams, unsigned& min_year_defined);

bool read_ini_file(const char* ini_filename, std::string& output_file_name, 
	std::string& totalcount_file, std::string& path_to_treated_files,
	unsigned& nb_ngrams, unsigned& min_year_defined);

#endif
