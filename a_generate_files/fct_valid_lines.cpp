#include "../util/util.hpp"
#include "fct_valid_lines.hpp"

using namespace std;

/*! \brief Puts each word_tag from a ngram in a vector. 
 *	Used in valid_ngram().
 *
 * \param ngram The ngram to treat, 
 * ex : Aaron_NOUN represented_VERB the_DET Lord_NOUN
 * \param words_tags The vector to fill with word_tag
 */
void collect_words_tags(string ngram, vector<string>& words_tags)
{
	string delimiter = " ";
	size_t pos = 0;
	while ( (pos = ngram.find(delimiter)) != std::string::npos ) 
	{
		words_tags.push_back(ngram.substr(0, pos));
		ngram.erase(0, pos + delimiter.length());	
	}
	if( ngram != "" )
		words_tags.push_back(ngram);
}

/*! \brief Says if a ngram is valid or not. Used in valid_line().
 *
 * A ngram is valid if all words are correctly tagged and all words are 
 * correct, and if there is "nb_ngram" words.
 * 
 * \param ngram The ngram to treat, 
 * ex : Aaron_NOUN represented_VERB the_DET Lord_NOUN
 * \param forbidden_characters Vector containing forbidden words in 
 * case of bad tagging
 * \param accepted_tags Vector containing the accepted tags
 * \param nb_ngram Number of word_tag you want
 * \return True if the ngram is valid, else false.
 */
bool valid_ngram(string ngram, vector<string>& forbidden_characters, 
	vector<string>& accepted_tags, unsigned nb_ngram,
	regex& regex_numeric, bool no_number)
{
	vector<string> words_tags;
	collect_words_tags(ngram, words_tags);
	
	if( words_tags.size() != nb_ngram )
		return false;
	
	std::smatch match_numeric;
	string word;
	string tag;
	string delimiter = "_";
	size_t pos = 0;
	for (unsigned i=0; i<words_tags.size(); ++i)
	{		
		if ( words_tags[i].find(delimiter) == std::string::npos )
			return false;
		
		while ( (pos = words_tags[i].find(delimiter)) != std::string::npos )
		{
			word = words_tags[i].substr(0, pos);
			if( std::find(forbidden_characters.begin(), forbidden_characters.end(), word) != forbidden_characters.end() )
				return false;
			if( no_number && std::regex_search(word, match_numeric, regex_numeric) )
				return false;
			words_tags[i].erase(0, pos + delimiter.length());
		}
		tag = words_tags[i];
		if( std::find(accepted_tags.begin(), accepted_tags.end(), tag) == accepted_tags.end() )
			return false;
	}
	return true;		
}

/*! \brief Says if a line is valid or not. Fills all the elements (ngram, year, occurrence, volume).
 *
 * A line is valid if : 
 * - The ngram is valid : all words are correctly tagged and all words are correct, and there is n words.
 * - The year is valid : it is superior or equal than the min year you defined
 * - There is 4 elements : the ngram, the year, the occurrence, the number of volume
 * 
 * \param line The line to treat, ex : 
 * \param ngram The ngram to fill
 * \param year The year to fill
 * \param nb_match The occurrence to fill
 * \param nb_volume The number of volume to fill
 * \param forbidden_characters Vector containing forbidden words in case of bad tagging
 * \param accepted_tags Vector containing the accepted tags
 * \param nb_ngram Number of word_tag you want
 * \param min_year_defined The minimum year you want
 * \return True if the line is valid, else false.
 */
bool valid_line(string line, string& ngram, 
	unsigned long long& year, unsigned long long& nb_match, unsigned long long& nb_volume, 
	vector<string>& forbidden_characters, 
	vector<string>& accepted_tags, 
	unsigned nb_ngram, unsigned min_year_defined,
	regex& regex_numeric, bool no_number)
{
	string token;
	string delimiter = "\t";
	unsigned position = 0;
	size_t pos = 0;
	
	// cut by \t
	while ((pos = line.find(delimiter)) != std::string::npos) 
	{
		++ position;
		token = line.substr(0, pos);
		line.erase(0, pos + delimiter.length());
		
		//ngram
		if(position == 1)
		{
			if( !valid_ngram(token, forbidden_characters, accepted_tags, nb_ngram, regex_numeric, no_number) )
				return false;
			ngram = token;
		}
		//year
		else if(position == 2)
		{
			year = stoull( token );
			if(year < min_year_defined)
				return false;
		}
		else if(position == 3)
			nb_match = stoull( token );
	}
	if(line != "")
	{
		++ position;
		nb_volume = stoull( line );
	}
	
	return position==4;
}
