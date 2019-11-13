#include "../a_generate_files/fct_valid_lines.hpp"
#include "fct_calcul_frequences_tag_grams.hpp"

#define LINE_SIZE 1024

using namespace std;

bool write_output_frequences_tags_grams(const char* filename, unsigned long long total_match, 
	unsigned long long total_volume, map<std::string, Data*>& tags_to_data)
{
	FILE* output = fopen(filename, "w");
	if( output == NULL )
	{
		print_message("Cannot open file ", filename);
		return false;
	}

	for(auto it=tags_to_data.begin(); it != tags_to_data.end(); ++it)
	{
		fprintf(output, 
			"%s\t%llu\t%llu\t%llu\t%.2lf\t%.2lf\t%d\t%d\t%llu\t%llu\t%llu\t%llu\t%.8Le\t%.8Le\n", 
			it->first.c_str(), 
			it->second->get_somme_year(), 
			it->second->get_somme_nb_match(), 
			it->second->get_somme_nb_volume(), 
			it->second->calcul_mean_pondere_match(),
			it->second->calcul_mean_pondere_volume(), 
			it->second->get_year_max(),
			it->second->get_year_min(), 
			it->second->get_nb_match_max(), 
			it->second->get_nb_match_min(),
			it->second->get_nb_volume_max(), 
			it->second->get_nb_volume_min(), 
			it->second->get_freq_match(total_match), 
			it->second->get_freq_volume(total_volume) );
	}	
	
	fclose(output);
	return true;	
}


bool collect_tags(string token, string& tag, 
	vector<string>& accepted_tags, unsigned nb_ngrams)
{
	// put every word_tag in vector words_tags
	vector<string> words_tags;
	collect_words_tags(token, words_tags);
	if(words_tags.size() != nb_ngrams)
		return false;
	
	string word;
	tag = "";
	string delimiter = "_";
	size_t pos = 0;
	
	for (unsigned i=0; i<words_tags.size(); ++i)
	{
		// no "_"
		if (words_tags[i].find(delimiter) == std::string::npos)
			return false;
			
		while ((pos = words_tags[i].find(delimiter)) != std::string::npos)
		{
			word = words_tags[i].substr(0, pos);
			words_tags[i].erase(0, pos + delimiter.length());
		}
		
		// now words_tags[i] contains only the tag
		if( find(accepted_tags.begin(), accepted_tags.end(), words_tags[i]) == accepted_tags.end() )
			return false;
		tag = tag + words_tags[i] + " ";
	}
	tag.pop_back();
	return true;
}

bool update_elements(unsigned position, bool& good_tags, string token, 
	string& tags, vector<string>& accepted_tags, unsigned long long& nb_year,
	unsigned long long& nb_match, unsigned long long& nb_volume, double& mean_pondere_match,
	double& mean_pondere_volume, unsigned& year_max, unsigned& year_min,
	unsigned long long& nb_match_max, unsigned long long& nb_match_min, 
	unsigned long long& nb_volume_max, unsigned nb_ngrams)
{
	switch(position)
	{
		case 1:
			good_tags = collect_tags(token, tags, accepted_tags, nb_ngrams);
			return true;
			break;
		case 2:
			nb_year = stoull( token );
			return true;
			break;
		case 3:
			nb_match = stoull( token );
			return true;
			break;
		case 4:
			nb_volume = stoull( token );
			return true;
			break;
		case 5:
			mean_pondere_match = stod( token );
			return true;
			break;
		case 6:
			mean_pondere_volume = stod( token );
			return true;
			break;
		case 7:
			year_max = stoi( token );
			return true;
			break;
		case 8:
			year_min = stoi( token );
			return true;
			break;
		case 9:
			nb_match_max = stoull( token );
			return true;
			break;
		case 10:
			nb_match_min = stoull( token );
			return true;
			break;
		case 11:
			nb_volume_max = stoull( token );
			return true;
			break;
		default:
			good_tags = false;
			return false;
			break;
	}
}

void update_tag_grams(map<std::string, Data*>& tags_to_data, string tags, 
	unsigned long long nb_year, unsigned long long nb_match, unsigned long long nb_volume, double mean_pondere_match,
	double mean_pondere_volume, unsigned year_max, unsigned year_min,
	unsigned long long nb_match_max, unsigned long long nb_match_min, 
	unsigned long long nb_volume_max, unsigned long long nb_volume_min)
{
	if( tags_to_data.find(tags) == tags_to_data.end() )
		tags_to_data[tags] = new Data();
	
	tags_to_data[tags]->add_somme_year(nb_year);
	tags_to_data[tags]->add_somme_nb_match(nb_match);
	tags_to_data[tags]->add_somme_nb_volume(nb_volume);
	tags_to_data[tags]->add_mean_pondere_match(mean_pondere_match);
	tags_to_data[tags]->add_mean_pondere_volume(mean_pondere_volume);
	tags_to_data[tags]->try_and_change_year_max(year_max);
	tags_to_data[tags]->try_and_change_year_min(year_min);
	tags_to_data[tags]->try_and_change_match_max(nb_match_max);
	tags_to_data[tags]->try_and_change_match_min(nb_match_min);
	tags_to_data[tags]->try_and_change_volume_max(nb_volume_max);
	tags_to_data[tags]->try_and_change_volume_min(nb_volume_min);	
}

void update_tag_grams(map<string, Data*>& tags_to_data, string tags, 
	unsigned long long nb_year, unsigned long long nb_match, 
	unsigned long long nb_volume, unsigned year_max, unsigned year_min,
	unsigned long long nb_match_max, unsigned long long nb_match_min, 
	unsigned long long nb_volume_max, unsigned long long nb_volume_min)
{
	if( tags_to_data.find(tags) == tags_to_data.end() )
		tags_to_data[tags] = new Data();
	
	tags_to_data[tags]->add_somme_year(nb_year);
	tags_to_data[tags]->add_somme_nb_match(nb_match);
	tags_to_data[tags]->add_somme_nb_volume(nb_volume);
	tags_to_data[tags]->try_and_change_year_max(year_max);
	tags_to_data[tags]->try_and_change_year_min(year_min);
	tags_to_data[tags]->try_and_change_match_max(nb_match_max);
	tags_to_data[tags]->try_and_change_match_min(nb_match_min);
	tags_to_data[tags]->try_and_change_volume_max(nb_volume_max);
	tags_to_data[tags]->try_and_change_volume_min(nb_volume_min);	
}

void update_tag_grams_safe(mutex& map_mutex, map<std::string, Data*>& tags_to_data,
	map<std::string, Data*>& tags_to_data_file)
{
	std::lock_guard<std::mutex> guard(map_mutex);
	unsigned i=0;
	unsigned size_match, size_volume;
	for(auto it = tags_to_data_file.begin(); it != tags_to_data_file.end(); ++it)
	{
		update_tag_grams(tags_to_data, it->first, 
			it->second->get_somme_year(), it->second->get_somme_nb_match(), 
			it->second->get_somme_nb_volume(), it->second->get_year_max(), it->second->get_year_min(),
			it->second->get_nb_match_max(), it->second->get_nb_match_min(), 
			it->second->get_nb_volume_max(), it->second->get_nb_volume_min());
			
		size_match = it->second->get_nb_mean_pondere_match();
		size_volume = it->second->get_nb_mean_pondere_volume();
		for(i=0; i<size_match; ++i)
			tags_to_data[it->first]->add_mean_pondere_match(it->second->get_mean_pondere_match(i));
			
		for(i=0; i<size_volume; ++i)
			tags_to_data[it->first]->add_mean_pondere_volume(it->second->get_mean_pondere_volume(i));
	}
}

void treat_occurrences_tag_grams(map<std::string, Data*>& tags_to_data, vector<string>& accepted_tags, 
	FILE* input, unsigned nb_ngrams, string& large_filename)
{
	char buffer[LINE_SIZE];
	string line;
	string token;
	string tags;
	string delimiter = "\t";
	unsigned position, cpt_line = 0;
	size_t pos;
	bool good_tags = false;
	
	unsigned long long nb_year = 0;
	unsigned long long nb_match = 0;
	unsigned long long nb_volume = 0;
	double mean_pondere_match = 0;
	double mean_pondere_volume = 0;
	unsigned year_max = 0;
	unsigned year_min = 3000;
	unsigned long long nb_match_max = 0;
	unsigned long long nb_match_min = ULLONG_MAX;
	unsigned long long nb_volume_max = 0;
	unsigned long long nb_volume_min = ULLONG_MAX;

	while( fgets(buffer, sizeof(buffer), input) )
	{
		++cpt_line;
		line = buffer;
		position = 0;
		pos = 0;
					
		// cut by \t
		while ((pos = line.find(delimiter)) != std::string::npos) 
		{
			++ position;
			token = line.substr(0, pos);
			line.erase(0, pos + delimiter.length());
			
			if( !update_elements(position, good_tags, token, 
				tags, accepted_tags,nb_year,
				nb_match, nb_volume, mean_pondere_match,
				mean_pondere_volume, year_max, year_min,
				nb_match_max, nb_match_min, nb_volume_max, nb_ngrams) )
			{
				cerr << "WARNING - ELEMENTS MUST BE SEPARATED BY TABULATION - bad line (" << cpt_line 
					 << ") on file " << large_filename << " : " << buffer << "\n";
			}
		}
		if( line != "" )
		{
			nb_volume_min = stoull( line );
			++ position;
		}
		if( position != 12 )
			cerr << "WARNING - THERE ARE NOT 12 ELEMENTS - bad line (" << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
		else
		{
			if(good_tags)
				update_tag_grams(tags_to_data, tags, nb_year, nb_match, nb_volume, 
					mean_pondere_match, mean_pondere_volume, year_max, year_min,
					nb_match_max, nb_match_min, nb_volume_max, nb_volume_min);
		}
		memset(buffer, 0, sizeof(buffer));
	}
}
 
void calcul_total_occurrences(
	map<std::string, Data*>& tags_to_data, unsigned long long& total_match)
{
	total_match = 0;
	for(auto it=tags_to_data.begin(); it != tags_to_data.end(); ++it)
		total_match += it->second->get_somme_nb_match();
}

void destroy_data(map<std::string, Data*>& tags_to_data)
{
	for(auto it=tags_to_data.begin(); it != tags_to_data.end(); ++it)
	{
		it->second->~Data();
	}	
	tags_to_data.clear();
}

void update_args(ifstream& file_ini, string& line, string& output_file_name, 
	string& totalcount_file, string& path_to_treated_files,
	unsigned& nb_ngrams, unsigned& min_year_defined)
{ 
	string tmp("");
	if( line == "output_file_name" || line == "output_file_name=" )
		init_arg(file_ini, line, output_file_name);
	else if( line == "totalcount_file" || line == "totalcount_file=" )
		init_arg(file_ini, line, totalcount_file);
	else if( line == "path_to_treated_files" || line == "path_to_treated_files=" )
		init_arg(file_ini, line, path_to_treated_files);
	else if( line == "nb_ngram" || line == "nb_ngram=" )
	{
		init_arg(file_ini, line, tmp);
		if( valid_nb_ngram(tmp) )
			nb_ngrams = stoul(tmp);
		else
			cerr << "WARNING invalid entry for nb_ngram." 
				 << " nb_ngram is now set to 1\n";
	}
	else if( line == "min_year" || line == "min_year=" )
	{
		init_arg(file_ini, line, tmp);
		if( valid_min_year(tmp) )
			min_year_defined = stoul(tmp);
		else
			cerr << "WARNING invalid entry for min_year."
				 << " min_year is now set to 0\n";
	}
	else
	{
		cerr << "WARNING don't recognize this variable : " << line
			 << "\nVariables for d_calcul_frequences_tag_grams should be : "
			 << "output_file_name, totalcount_file, "
			 << "path_to_treated_files, nb_ngram, min_year\n";
	}
}

bool read_ini_file(const char* ini_filename, string& output_file_name, 
	string& totalcount_file, string& path_to_treated_files,
	unsigned& nb_ngrams, unsigned& min_year_defined)
{
	ifstream file_ini;
	string line("");
	output_file_name = "";
	totalcount_file = "";
	path_to_treated_files = "";
	nb_ngrams = 1;
	min_year_defined = 0;

	if( ini_filename == NULL )
		file_ini.open("../config.ini");
	else
		file_ini.open(ini_filename);
	
	if( !file_ini )
	{
		cerr << "Impossible to open the ini file or didn't find it\n";
		return false;
	}
	
    while( line != "d_calcul_frequences_tag_grams:" && line != "d_calcul_frequences_tag_grams" )
    {
		if( !getline(file_ini, line) )
			break;
		cout << line << endl;
	}
	
	if( line == "d_calcul_frequences_tag_grams:" || line == "d_calcul_frequences_tag_grams" )
	{
		file_ini >> line;
		read_comment(file_ini, line);
		while( line != "END" && line != "" )
		{
			update_args(file_ini, line, output_file_name, 
				totalcount_file, path_to_treated_files,
				nb_ngrams, min_year_defined);
			file_ini >> line;
			read_comment(file_ini, line);
		}
	}
	else
	{
		cerr << "Didn't find the line \"d_calcul_frequences_tag_grams:\" or "
			 << "\"d_calcul_frequences_tag_grams\". Stop.\n";
		return false;
	}
	file_ini.close();
	return true;
}
