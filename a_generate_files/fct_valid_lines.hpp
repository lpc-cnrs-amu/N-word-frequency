#ifndef FCT_VALID_LINES_HPP
#define FCT_VALID_LINES_HPP

#include <string>
#include <vector>
#include <regex>

void collect_words_tags(std::string ngram, std::vector<std::string>& words_tags);

bool valid_ngram(std::string ngram, std::vector<std::string>& forbidden_characters, 
	std::vector<std::string>& accepted_tags, unsigned nb_ngram,
	std::regex& regex_numeric, bool no_number);

bool valid_line(std::string line, std::string& ngram, 
	unsigned long long& year, unsigned long long& nb_match, unsigned long long& nb_volume, 
	std::vector<std::string>& forbidden_characters, 
	std::vector<std::string>& accepted_tags, 
	unsigned nb_ngram, unsigned min_year_defined,
	std::regex& regex_numeric, bool no_number);

#endif
