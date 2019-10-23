#include "../util/util.hpp"
#include "fct_valid_lines.hpp"

using namespace std;

void collect_words_tags(string line, vector<string>& words_tags)
{
	string token;
	string delimiter = " ";
	size_t pos = 0;
	while ((pos = line.find(delimiter)) != std::string::npos) 
	{
		words_tags.push_back(line.substr(0, pos));
		line.erase(0, pos + delimiter.length());	
	}
	words_tags.push_back(line);
}

bool valid_word_tag(string line, vector<string>& forbidden_characters, vector<string>& accepted_tags)
{
	vector<string> words_tags;
	collect_words_tags(line, words_tags);
	
	if(words_tags.size() != NB_NGRAM)
		return false;
		
	string word;
	string tag;
	string delimiter = "_";
	size_t pos = 0;
	for (unsigned i=0; i<words_tags.size(); ++i)
	{		
		// pas de "_" présent
		if (words_tags[i].find(delimiter) == std::string::npos)
			return false;
		
		while ((pos = words_tags[i].find(delimiter)) != std::string::npos)
		{
			word = words_tags[i].substr(0, pos);
			words_tags[i].erase(0, pos + delimiter.length());
		}
		tag = words_tags[i];
		
		// mot = caractère interdit
		if( std::find(forbidden_characters.begin(), forbidden_characters.end(), word) != forbidden_characters.end() )
			return false;
		// tag = pas un tag accepté
		if( std::find(accepted_tags.begin(), accepted_tags.end(), tag) == accepted_tags.end() )
			return false;
	}
	return true;		
}

bool valid_line(string line, string& words_tags, unsigned& year, 
	unsigned& nb_match, unsigned& nb_volume, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
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
		
		//words + tags
		if(position == 1)
		{
			if( !valid_word_tag(token, forbidden_characters, accepted_tags) )
				return false;
			words_tags = token;
		}
		//year
		else if(position == 2)
		{
			year = stoi( token );
			if(year < YEAR)
				return false;
		}
		else if(position == 3)
			nb_match = stoi( token );
	}
	if(line != "") // ce qu'il reste (le dernier élément de la ligne)
	{
		++ position;
		nb_volume = stoi( line );
	}
	
	return position==4;
}
