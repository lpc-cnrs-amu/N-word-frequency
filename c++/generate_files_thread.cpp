#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <thread>
#include "QueueSafe.hpp"
#include "zlib1211/zlib.h"

//#define CHUNK_SIZE 1048576
#define CHUNK_SIZE 1024
#define NB_NGRAM 4
#define YEAR 1970

using namespace std;
using namespace std::chrono;

bool has_suffix(const char* name, string &suffix);

void collect_filenames(QueueSafe<string>& queue_filenames);

void generate_file(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);

FILE* get_file(unsigned thread_id, string filename);

void print_error(string message, char* cut_filename);
void print_error(string message, string cut_filename);

void generate_file(QueueSafe<string>& queue_filenames, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);

void collect_words_tags(string line, vector<string>& words_tags);
	
bool valid_word_tag(string line, vector<string>& forbidden_characters, vector<string>& accepted_tags);

bool valid_line(string line, string& words_tags, unsigned& year, 
	unsigned& nb_match, unsigned& nb_volume, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);
	
	


bool has_suffix(const char* name, string &suffix)
{
	string str = name;
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void collect_filenames(QueueSafe<string>& queue_filenames)
{
	DIR *pdir = NULL; //pointeur vers un dossier
    struct dirent *pent = NULL; //structure nécessaire a la lecture de répertoire, elle contiendra le nom du/des fichier
    string path_to_files("/mnt/c/Users/Marjorie/Documents/git_repo/freqNwords/files_gz/");
    string filename("");
    string suffix(".gz");
                                           
    pdir = opendir (path_to_files.c_str()); 
                                           
    if (pdir == NULL) //si il y a eu un problème pour l'ouverture du répertoire
    {
        cout << "Erreur d'ouverture du repertoire" << endl; 
        return ;                                  
    }
    
    while (pent = readdir (pdir)) //tant qu'il reste quelque chose qu'on a pas lister
    {
        if (pent == NULL) //si il y a eu un probleme dans l'ouverture d'un fichier du repertoire
        {
			cout << "Erreur d'ouverture d'un fichier contenu dans le repertoire" << endl;     
			return ; 
        }
        if(	pent->d_name != NULL && has_suffix(pent->d_name, suffix) )
        {
			filename = pent->d_name;
			filename = path_to_files + filename;
			queue_filenames.push_front(filename);
		}
    }
    closedir (pdir);	
}	

FILE* get_file(unsigned thread_id, string filename)
{	
	// concat large_filename + num_cut_files
	string delimiter = "/";
	size_t pos = 0;
	while ((pos = filename.find(delimiter)) != std::string::npos) 
		filename.erase(0, pos + delimiter.length());
	filename.pop_back();
	filename.pop_back();
	filename.pop_back();
	filename += "_treated";
	filename = "/mnt/c/Users/Marjorie/Documents/git_repo/freqNwords/files_treated/" + filename;

	// open output file
	FILE* output = fopen(filename.c_str(), "w");
	if( output == NULL )
		print_error("Impossible to open the file ", filename);
	cout << "\tThread " << thread_id << " : " << filename << "\n";
	//cerr << "\tThread " << thread_id << " : " << filename << "\n";

	return output;
}
	
void print_error(string message, char* cut_filename)
{
	cout << message << cut_filename << "\n";
	//cerr << message << cut_filename << "\n";
}	
void print_error(string message, string cut_filename)
{
	cout << message << cut_filename << "\n";
	//cerr << message << cut_filename << "\n";
}		
	
void generate_file(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
{	
	// to treat the lines
	unsigned char buffer[CHUNK_SIZE];
	unsigned int unzipped_bytes = 1;
	string word_tag;
	string precedent_word_tag = "";
	stringstream token("");
	unsigned year, nb_match, nb_volume;
	int err;
	
	// for the operations on the lines
	unsigned somme_year = 0;
	unsigned somme_nb_match = 0;
	unsigned somme_nb_volume = 0;
	float mean_pondere_match = 0;
	float mean_pondere_volume = 0; 
	unsigned year_max = 0;
	unsigned year_min = 3000;
	unsigned nb_match_max = 0;
	unsigned nb_match_min = 100000;
	unsigned nb_volume_max = 0;
	unsigned nb_volume_min = 100000;
	bool one_valid_line = false;
	
	string large_filename;
	while( !queue_filenames.empty() )
	{	
		
		if( !queue_filenames.try_pop(large_filename) )
			continue;
		
		// open the .gz file
		gzFile large_file = gzopen(large_filename.c_str(), "rb");
		if( large_file == NULL )
		{
			print_error("Impossible to open the file ", large_filename);
			break;	
		}
			
		// open the output file
		FILE* output = get_file(thread_id, large_filename);
		if( output == NULL )
			break;
								
		while(1)
		{
			memset(buffer, 0, sizeof(buffer));
			unzipped_bytes = gzread(large_file, buffer, CHUNK_SIZE-1);
			
			// read nothing...
			if(unzipped_bytes <= 0)
			{
				// ... because of end of file
				if (gzeof (large_file))
				{
					//cout << "Thread " << thread_id << " finish " << large_filename << "\n";
					break;
				}
				// ... because of an error
				else 
				{
					const char * error_string;
					error_string = gzerror (large_file, &err);
					if (err) 
					{
						fprintf (stderr, "On %s : Error : %s.\n", 
							large_filename.c_str(), error_string);
						break;
					}
				}
			}
			buffer[unzipped_bytes] = '\0';
			for(unsigned i=0; i<unzipped_bytes; ++i)
			{
				if( buffer[i] != '\n' )
					token << buffer[i];
					
				// find a line
				else
				{
					if( valid_line(token.str(), word_tag, year, nb_match, 
						nb_volume, forbidden_characters, accepted_tags) )
					{		
						// find a new ngram so we write the precedent (except for the 1st line of the file)
						if(word_tag != precedent_word_tag && precedent_word_tag != "")
						{
							fprintf(output, "%s\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t %d\t%d\t%d\t\n", 
								precedent_word_tag.c_str(), somme_year, somme_nb_match, somme_nb_volume, 
								mean_pondere_match/static_cast<float>(somme_nb_match), mean_pondere_volume/static_cast<float>(somme_nb_volume), 
								year_max, year_min, nb_match_max, nb_match_min, nb_volume_max, nb_volume_min);
								
							somme_year = 1;
							somme_nb_match = nb_match;
							somme_nb_volume = nb_volume;
							mean_pondere_match = year * nb_match;
							mean_pondere_volume = year * nb_volume; 
							year_max = year;
							year_min = year;
							nb_match_max = nb_match;
							nb_match_min = nb_match;
							nb_volume_max = nb_volume;
							nb_volume_min = nb_volume;					
						}
						// find the same ngram as the precedent
						else
						{
							++ somme_year;
							somme_nb_match += nb_match;
							somme_nb_volume += nb_volume;
							mean_pondere_match = mean_pondere_match + year * nb_match;
							mean_pondere_volume = mean_pondere_volume + year * nb_volume;
							if( year > year_max )
								year_max = year;
							if( year < year_min )
								year_min = year;

							if( nb_match > nb_match_max )
								nb_match_max = nb_match;
							if( nb_match < nb_match_min )
								nb_match_min = nb_match;
								
							if( nb_volume > nb_volume_max )
								nb_volume_max = nb_volume;
							if( nb_volume < nb_volume_min )
								nb_volume_min = nb_volume;
						}
							
						precedent_word_tag = word_tag;
						
					}
					token.str(std::string());
					token.clear();
				}
			}
		}
		bool not_empty = static_cast<bool>(token >> word_tag);
		if(not_empty)
		{
			cout << "WARNING -- didn't read the entire file " 
				 << large_filename << " : has left :[" 
				 << word_tag <<"]\n"; 
			/*cerr << "WARNING -- didn't read the entire file " 
				 << large_filename << " : has left :[" 
				 << word_tag <<"]\n"; */
		}
		
		// write the last treated line
		else
		{
			if(one_valid_line)
			{
				fprintf(output, "%s\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t %d\t%d\t%d\t\n", 
					word_tag.c_str(), somme_year, somme_nb_match, somme_nb_volume, 
					mean_pondere_match/static_cast<float>(somme_nb_match), mean_pondere_volume/static_cast<float>(somme_nb_volume), 
					year_max, year_min, nb_match_max, nb_match_min, nb_volume_max, nb_volume_min);
			}
		}
		gzclose(large_file);
		fclose(output);
		cout << "Thread " << thread_id << " finish " << large_filename << "\n";
		/*
		int supp = remove( large_filename );
		if( supp != 0 )
		{
			cout << "Error deleting " << large_filename << "\n";
			cerr << "Error deleting " << large_filename << "\n"; 
		}
		else
		{
			cout << "Delete " << large_filename << "\n";
			cerr << "Delete " << large_filename << "\n"; 		
		}
		*/
	}
}

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


int main(int argc, char** argv)
{
	QueueSafe<string> queue_filenames;
	vector<string> forbidden_characters {",",".","?","!","...",";",
		":","\""," ","","'"};
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT"};
	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;

	collect_filenames(queue_filenames);
	auto start = high_resolution_clock::now();
	for(unsigned i=0; i<nb_cores; ++i)
	{
		cout << "create thread " << i+1 << endl;
		threads.emplace_back( [&]{generate_file( i+1, queue_filenames, forbidden_characters, accepted_tags ); } );
	}
	for(auto& t: threads)
		t.join();
	threads.clear();

	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl;
	
    return 0;
}
