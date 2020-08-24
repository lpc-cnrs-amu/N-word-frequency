#include "fct_generate_files.hpp"
#include "fct_valid_lines.hpp"

#define CHUNK_SIZE 1024

using namespace std;

void update_args(ifstream& file_ini, string& line, string& path_to_gz, 
	string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined,
	bool& no_number)
{ 
	string tmp("");
	if( line == "path_to_gz_files" || line == "path_to_gz_files=" )
	{
		init_arg(file_ini, line, path_to_gz);
	}
	else if( line == "path_to_output_files" || line == "path_to_output_files=" )
	{
		init_arg(file_ini, line, path_to_output);
	}
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
	else if( line == "no_number" || line == "no_number=" )
	{
		init_arg(file_ini, line, tmp);
		no_number = (tmp != "0");
	}
	else
	{
		cerr << "WARNING don't recognize this variable : " << line
			 << "\nVariables for a_generate_file should be : "
			 << "path_to_gz_files, path_to_output_files, "
			 << "nb_ngram, min_year, no_number\n";
	}
}

bool read_ini_file(const char* ini_filename, string& path_to_gz, 
	string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined,
	bool& no_number)
{
	ifstream file_ini;
	string line("");
	path_to_gz = "";
	path_to_output = "";
	nb_ngrams = 1;
	min_year_defined = 0;
	no_number = true;

	if( ini_filename == NULL )
		file_ini.open("../config.ini");
	else
		file_ini.open(ini_filename);
	
	if( !file_ini )
	{
		cerr << "Impossible to open the ini file or didn't find it\n";
		return false;
	}
	
    while( line != "a_generate_file:" && line != "a_generate_file" )
    {
		if( !getline(file_ini, line) )
			break;
	}
	if( line == "a_generate_file:" || line == "a_generate_file" )
	{
		file_ini >> line;
		read_comment(file_ini, line);
		while( line != "END" && line != "" )
		{
			update_args(file_ini, line, path_to_gz, 
				path_to_output, nb_ngrams, min_year_defined, no_number);
			file_ini >> line;
			read_comment(file_ini, line);
		}
	}
	else
	{
		cerr << "Didn't find the line \"a_generate_file:\" or "
			 << "\"a_generate_file\". Stop.\n";
		file_ini.close();
		return false;
	}
	file_ini.close();
	return true;
}

void explode_ngram(string& precedent_ngram, vector<string>& words, 
	vector<string>& tags)
{
	vector<string> words_tags;
	collect_words_tags(precedent_ngram, words_tags);
	
	string delimiter = "_";
	size_t pos = 0;
	for (unsigned i=0; i<words_tags.size(); ++i)
	{		
		pos = words_tags[i].find_last_of(delimiter);
		words.push_back( words_tags[i].substr(0, pos) );
		words_tags[i].erase(0, pos + delimiter.length());
		tags.push_back( words_tags[i] );
	}
}

void write_in_file(FILE* output, string& ngram, unsigned& somme_year,
	unsigned long long& somme_nb_match, unsigned long long& somme_nb_volume, 
	unsigned long long& mean_pondere_match, unsigned long long& mean_pondere_volume, 
	unsigned& year_max, unsigned& year_min, unsigned long long& nb_match_max, 
	unsigned long long& nb_match_min, unsigned long long& nb_volume_max,
	unsigned long long& nb_volume_min)
{
	unsigned i;
	vector<string> words;
	vector<string> tags;
	explode_ngram(ngram, words, tags);
	for(i=0; i<words.size(); ++i)
		fprintf(output, "%s\t", words[i].c_str());
	for(i=0; i<tags.size(); ++i)
		fprintf(output, "%s\t", tags[i].c_str());
	
	fprintf(output, 
		"%d\t%llu\t%llu\t%.2lf\t%.2lf\t%d\t%d\t%llu\t%llu\t%llu\t%llu\n", 
		somme_year, somme_nb_match, 
		somme_nb_volume, mean_pondere_match/static_cast<double>(somme_nb_match), 
		mean_pondere_volume/static_cast<double>(somme_nb_volume), 
		year_max, year_min, nb_match_max, nb_match_min, 
		nb_volume_max, nb_volume_min);	
}


/*! \brief Says if the .gz file has been entirely read or not.
 * 
 * \param token Is empty if the .gz file has been entirely read
 * \return False if the file has been entirely read, else true
 */
bool file_not_entirely_read(stringstream& token)
{
	string buff;
	bool not_empty = static_cast<bool>(token >> buff);
	if(not_empty)
		print_message("WARNING didn't read the entire file, has left : ", buff);
	return not_empty;
}		

void treat_file(int thread_id, gzFile large_file, FILE* output, string large_filename, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags, 
	unsigned nb_ngrams, unsigned min_year_defined, bool no_number)
{
	int err;
	std::regex regex_numeric("[0-9]+");
	unsigned char buffer[CHUNK_SIZE];
	unsigned int unzipped_bytes = 1;
	string ngram;
	stringstream token("");
	unsigned year;
	unsigned long long nb_match, nb_volume;
	unsigned long long mean_pondere_match = 0;
	unsigned long long mean_pondere_volume = 0; 
	unsigned year_max = 0;
	unsigned year_min = 3000;
	unsigned long long match_max = 0;
	unsigned long long match_min = ULLONG_MAX;
	unsigned long long volume_max = 0;
	unsigned long long volume_min = ULLONG_MAX;
							
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		unzipped_bytes = gzread(large_file, buffer, CHUNK_SIZE-1);
		
		// read nothing...
		if(unzipped_bytes <= 0)
		{
			// ... because of end of file
            if (gzeof (large_file))
                break;
            // ... because of an error
            else 
			{
				const char * error_string = gzerror (large_file, &err);
				if (err) 
				{
					if( thread_id != -1 )
						cerr << "[Thread " << thread_id << "] File " 
							 << large_filename << ", Error : " 
							 << error_string << "\n";
					else
						cerr << "File " 
							 << large_filename << ", Error : " 
							 << error_string << "\n"; 
					break;
				}
			}
		}
		buffer[unzipped_bytes] = '\0';
		for(unsigned i=0; i<unzipped_bytes; ++i)
		{
			if( buffer[i] != '\n' )
				token << buffer[i];
			else
			{
				if( valid_line(token.str(), ngram, year, nb_match, 
					nb_volume, forbidden_characters, accepted_tags, 
					nb_ngrams, min_year_defined, regex_numeric, no_number,
					year_max, year_min, mean_pondere_match, mean_pondere_volume,
					match_max, match_min, volume_max, volume_min) )
				{
					write_in_file(output, ngram, year,
						nb_match, nb_volume, mean_pondere_match, 
						mean_pondere_volume, year_max, year_min,
						match_max, match_min, volume_max, volume_min);
				}
				token.str(std::string());
				token.clear();
			}
		}
	}
	// last line
	//pour tester s'il y a bien un saut de ligne a la fin
	file_not_entirely_read(token);
	/*
	if( !file_not_entirely_read(token) && valid_line(token.str(), ngram, year, nb_match, 
		nb_volume, forbidden_characters, accepted_tags, 
		nb_ngrams, min_year_defined, regex_numeric, no_number,
		year_max, year_min, mean_pondere_match, mean_pondere_volume,
		match_max, match_min, volume_max, volume_min) )
	{
		write_in_file(output, ngram, year,
			nb_match, nb_volume, mean_pondere_match, 
			mean_pondere_volume, year_max, year_min,
			match_max, match_min, volume_max, volume_min);
	}
	*/
}
