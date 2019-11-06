#include "fct_take_n_most_frequencies_ngram.hpp"

#define LINE_SIZE 1024

using namespace std;

void update_args(ifstream& file_ini, string& line, string& output_file_name, 
	string& path_to_frequences_files, unsigned& nb_sentences)
{ 
	string tmp("");
	if( line == "output_file_name" || line == "output_file_name=" )
		init_arg(file_ini, line, output_file_name);
	else if( line == "path_to_frequencies_files" || line == "path_to_frequencies_files=" )
		init_arg(file_ini, line, path_to_frequences_files);
	else if( line == "n" || line == "n=" )
	{
		init_arg(file_ini, line, tmp);
		if( valid_min_year(tmp) )
			nb_sentences = stoul(tmp);
		else
			cerr << "WARNING invalid entry for n."
				 << " n is now set to 5.\n";
	}
	else
	{
		cerr << "WARNING don't recognize this variable : " << line
			 << "\nVariables for most_frequent_sentences should be : "
			 << "output_file_name, path_to_frequences_files, n.\n";
	}
}

bool read_ini_file(const char* ini_filename, string& output_file_name, 
	string& path_to_frequences_files, unsigned& nb_sentences)
{
	ifstream file_ini;
	string line("");
	output_file_name = "";
	path_to_frequences_files = "";
	nb_sentences = 5;

	if( ini_filename == NULL )
		file_ini.open("../config.ini");
	else
		file_ini.open(ini_filename);
	
	if( !file_ini )
	{
		cerr << "Impossible to open the ini file or didn't find it\n";
		return false;
	}
	
    while( line != "e_most_frequent_sentences:" && line != "e_most_frequent_sentences" )
    {
		if( !getline(file_ini, line) )
			break;
	}
	
	if( line == "e_most_frequent_sentences:" || line == "e_most_frequent_sentences" )
	{
		file_ini >> line;
		read_comment(file_ini, line);
		while( line != "END" && line != "" )
		{
			update_args(file_ini, line, output_file_name, 
				path_to_frequences_files, nb_sentences);
			file_ini >> line;
			read_comment(file_ini, line);
		}
	}
	else
	{
		cerr << "Didn't find the line \"e_most_frequent_sentences:\" or "
			 << "\"e_most_frequent_sentences\". Stop.\n";
		file_ini.close();
		return false;
	}
	file_ini.close();
	return true;
}


void def_new_min(map<string, float>& most_frequent_ngrams,
	string& key_min, float& freq_min)
{
	freq_min = 999999;
	for(auto it=most_frequent_ngrams.begin(); it != most_frequent_ngrams.end(); ++it)
	{
		if( it->second < freq_min )
		{
			freq_min = it->second;
			key_min = it->first;
		}
	}
}

void compare_freq(map<string, float>& most_frequent_ngrams, string new_key, 
	float freq, unsigned nb_sentences, string& key_min, float& freq_min)
{
	if( most_frequent_ngrams.empty() )
	{
		most_frequent_ngrams[new_key] = freq;
		freq_min = freq;
		key_min = new_key;
	}
	else if( most_frequent_ngrams.size() < nb_sentences )
	{
		most_frequent_ngrams[new_key] = freq;
		if( freq < freq_min )
		{
			freq_min = freq;
			key_min = new_key;
		}
	}
	else
	{
		if( freq > freq_min )
		{
			if( most_frequent_ngrams.find(key_min) != most_frequent_ngrams.end() )
			{
				most_frequent_ngrams.erase(key_min);
				most_frequent_ngrams[new_key] = freq;
				//cerr << "on a supp " << key_min << "(" << freq_min << ") pour mettre " << new_key << "(" << freq << ")\n";
				def_new_min(most_frequent_ngrams, key_min, freq_min);
				//cerr << "\tkey_min= " << key_min << " occ_min= "<< freq_min << "\n";
				
			}
			else
				cerr << "WARNING " << new_key << "\n";
		}
	}
}

void treat_most_freq(FILE* input, string large_filename, 
	map<string, float>& most_frequent_ngrams, unsigned nb_sentences, 
	string& key_min, float& freq_min)
{
	char buffer[LINE_SIZE];
	unsigned position;
	size_t pos;
	float freq;
	unsigned cpt_line = 0;
	string delimiter("\t");
	string token;
	string line("");
	string ngrams("");
	
	while( fgets(buffer, sizeof(buffer), input) )
	{	
		++cpt_line;
		line = buffer;
		position = 0;
		pos = 0;
		freq = 0;
				
		// cut by \t
		while ((pos = line.find(delimiter)) != std::string::npos) 
		{
			++ position;
			token = line.substr(0, pos);
			line.erase(0, pos + delimiter.length());
			
			if( position == 1 )
				ngrams = token;
			
			if( position == 13 )
				freq = stof( token );
		}
		if( line != "" )
			++ position;
		if( position != 14 )
		{
			cerr << "WARNING bad line (" << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
		}
		else
		{
			line = buffer;
			compare_freq(most_frequent_ngrams, line, freq, 
				nb_sentences, key_min, freq_min);
		}
		memset(buffer, 0, sizeof(buffer));
	}		
}

void update_most_frequent_ngrams_safe(mutex& map_mutex, 
	map<string, float>& most_frequent_ngrams, map<string, float>& most_frequent_ngrams_tmp, 
	unsigned nb_sentences, string& key_min, float& freq_min)
{
	std::lock_guard<std::mutex> guard(map_mutex);
	
	def_new_min(most_frequent_ngrams, key_min, freq_min);
	for(auto it = most_frequent_ngrams_tmp.begin(); it != most_frequent_ngrams_tmp.end(); ++it)
	{
		compare_freq(most_frequent_ngrams, it->first, 
			it->second, nb_sentences, key_min, freq_min);
	}	
}

bool write_output(const char* filename, 
	map<string, float>& most_frequent_ngrams)
{
	FILE* output = fopen(filename, "w");
	if( output == NULL )
	{
		print_message("Cannot open file ", filename);
		return false;
	}
	
	for(auto it=most_frequent_ngrams.begin(); it != most_frequent_ngrams.end(); ++it)
		fprintf(output, "%s", it->first.c_str());
			
	fclose(output);
	return true;	
}
