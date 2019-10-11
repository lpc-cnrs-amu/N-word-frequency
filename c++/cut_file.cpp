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
#include "zlib1211/zlib.h"

#define CUT_SIZE 100000
//#define CHUNK_SIZE 1048576
#define CHUNK_SIZE 1024
#define NB_NGRAM 4
#define YEAR 1970

using namespace std;
using namespace std::chrono;

void print_filenames(vector<char*>& filenames);
bool has_suffix(const char* name, string &suffix);
void collect_filenames(vector<char*>& filenames);


void cut_large_file_into_smaller_files(char* large_filename, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);
void collect_words_tags(string line, vector<string>& words_tags);
FILE* get_file(char* large_filename, unsigned long long num_cut_files);
void print_error(string message, char* cut_filename);
void cut_large_file_into_smaller_files(char* large_filename, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);
bool valid_word_tag(string word_tag, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);	
bool valid_line(string line, string& words_tags, unsigned& year, 
	unsigned& nb_match, unsigned& nb_volume, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);
void destroy_filenames(vector<char*>& filenames);
void cut_large_file_into_smaller_files_handler(vector<char*>& filenames,
	vector<string>& forbidden_characters, vector<string>& accepted_tags);
void collect_words_tags(string line, vector<string>& words_tags);
	
	
void print_filenames(vector<char*>& filenames)
{
	for(unsigned i=0; i<filenames.size(); ++i)
		cout << filenames[i] << endl;
}

bool has_suffix(const char* name, string &suffix)
{
	string str = name;
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void collect_filenames(vector<char*>& filenames)
{
	DIR *pdir = NULL; //pointeur vers un dossier
    struct dirent *pent = NULL; //structure nécessaire a la lecture de répertoire, elle contiendra le nom du/des fichier
    char const* path_to_files = "/mnt/c/Users/Marjorie/Documents/git_repo/freqNwords/Files_gz/";
    char* filename = NULL;
    string suffix = ".gz";
                                           
    pdir = opendir (path_to_files); 
                                           
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
        if( has_suffix(pent->d_name, suffix) )
        {
			filename = new char[strlen(path_to_files) + strlen(pent->d_name) + 1];
			strcpy(filename, path_to_files);
			strcat(filename, pent->d_name);
			filenames.push_back(filename);
		}
    }
    closedir (pdir);	
}	

FILE* get_file(char* large_filename, unsigned long long num_cut_files)
{
	char* cut_filename = NULL;
	FILE* output = NULL;
	
	// change num_cut_files to char*
	char const* num_cut_filename = to_string(num_cut_files).c_str();
	
	// concat large_filename + num_cut_files
	cut_filename = new char[strlen(large_filename)-3 + strlen(num_cut_filename) + 1];
	strcpy(cut_filename, large_filename);
	cut_filename[strlen(large_filename)-3] = '\0';
	strcat(cut_filename, num_cut_filename);

	// open output file
	output = fopen(cut_filename, "w");
	if( output == NULL )
		print_error("Impossible to open the file ", cut_filename);
	cout << "\tDoing: " << cut_filename << "\n";
	delete[] cut_filename;
	return output;
}
	
void print_error(string message, char* cut_filename)
{
	cerr << message << cut_filename << "\n";
	exit(EXIT_FAILURE);
}	
	
void cut_large_file_into_smaller_files(char* large_filename, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
{
	unsigned long long num_cut_files = 1;
	FILE* output = get_file(large_filename, num_cut_files);
	
	gzFile large_file = gzopen(large_filename, "rb");
	if( large_file == NULL )
		print_error("Impossible to open the file ", large_filename);	
	
	unsigned char buffer[CHUNK_SIZE];
	unsigned int unzipped_bytes = 1;
	
	string word_tag;
	stringstream token("");
	unsigned year, nb_match, nb_volume;
	unsigned nb_lines = 0;
	int err;
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		unzipped_bytes = gzread(large_file, buffer, CHUNK_SIZE-1);
		if(unzipped_bytes <= 0)
		{
            if (gzeof (large_file))
            {
				cout << "\nFINI\n";
                break;
            }
            else 
			{
				const char * error_string;
				error_string = gzerror (large_file, &err);
				if (err) 
				{
					fprintf (stderr, "Error: %s.\n", error_string);
					break;
				}
			}
		}
		buffer[CHUNK_SIZE-1] = '\0';
		for(unsigned i=0; i<strlen((char*)buffer); ++i)
		{
			
			if(nb_lines >= CUT_SIZE)
			{
				nb_lines = 0;
				++num_cut_files;
				fclose(output);
				output = get_file(large_filename, num_cut_files);
			}
			if( buffer[i] != '\n' )
				token << buffer[i];
			else
			{
				if( valid_line(token.str(), word_tag, year, nb_match, 
					nb_volume, forbidden_characters, accepted_tags) )
				{
					fprintf(output, "%s\t%d\t%d\t%d\n", 
						word_tag.c_str(), year, nb_match, nb_volume);
					++ nb_lines;
				}
				token.str(std::string());
				token.clear();
			}
			
		}
	}
	bool not_empty = static_cast<bool>(token >> word_tag);
	if(not_empty)
		cerr << "WARNING -- didn't read the entire file: " <<
			"has left :["<< word_tag <<"]\n"; 

	gzclose(large_file);
	fclose(output);
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

void cut_large_file_into_smaller_files_handler(vector<char*>& filenames,
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
{
	for(unsigned i=0; i<filenames.size(); ++i)
		cut_large_file_into_smaller_files(filenames[i], 
			forbidden_characters, accepted_tags);
}

void destroy_filenames(vector<char*>& filenames)
{
	for(unsigned i=0; i<filenames.size(); ++i)
	{
		free ((void*) filenames[i]);
		filenames[i] = NULL;
	}
}

int main(int argc, char** argv)
{
	vector<char*> filenames;
	vector<string> forbidden_characters {",",".","?","!","...",";",":","\""," ","","'"};
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT"};
		
	auto start = high_resolution_clock::now();

	collect_filenames(filenames);
    cut_large_file_into_smaller_files_handler(filenames, 
		forbidden_characters, accepted_tags);
    destroy_filenames(filenames);

	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	auto duration_seconds = duration_cast<std::chrono::seconds>(stop - start);
	auto duration_ms = duration_cast<std::chrono::milliseconds>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl; 
	cout << "(so : " << duration_seconds.count() << " seconds, " << duration_ms.count() << " ms)" << endl; 
	
    return 0;
}



