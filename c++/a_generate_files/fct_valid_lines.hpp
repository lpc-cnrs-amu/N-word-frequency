#ifndef FCT_VALID_LINES_HPP
#define FCT_VALID_LINES_HPP

#include <string>
#include <vector>

#define NB_NGRAM 4
#define YEAR 1970

void collect_words_tags(std::string line, std::vector<std::string>& words_tags);

bool valid_word_tag(std::string line, std::vector<std::string>& forbidden_characters, std::vector<std::string>& accepted_tags);

bool valid_line(std::string line, std::string& words_tags, unsigned& year, 
	unsigned& nb_match, unsigned& nb_volume, 
	std::vector<std::string>& forbidden_characters, std::vector<std::string>& accepted_tags);

#endif
