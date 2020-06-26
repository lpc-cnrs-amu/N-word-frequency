#include <dirent.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <thread>
#include <climits>
#include <mutex>
#include <queue>
#include "zlib1211/zlib.h"
#include "util/QueueSafe.hpp"
std::mutex add_mutex;
using namespace std;
#define CHUNK_SIZE 1024


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


bool valid_ngram(string line, string& ngram, unsigned nb_ngram,
	vector<string>& forbidden_characters, 
	vector<string>& accepted_tags)
{
	size_t pos = line.find("\t");
	ngram = line.substr(0, pos);

	vector<string> words_tags;
	collect_words_tags(ngram, words_tags);
	
	if( words_tags.size() != nb_ngram )
		return false;
		
	string word;
	string tag;
	string delimiter = "_";
	pos = 0;
	for (unsigned i=0; i<words_tags.size(); ++i)
	{		
		if ( words_tags[i].find(delimiter) == std::string::npos )
			return false;
		if(words_tags[i] == "_ROOT_" || words_tags[i] == "_START_" || words_tags[i] == "_END_")
			continue;
		
		while ( (pos = words_tags[i].find(delimiter)) != std::string::npos )
		{
			word = words_tags[i].substr(0, pos);
			words_tags[i].erase(0, pos + delimiter.length());
		}
		tag = words_tags[i];
		
		if( std::find(forbidden_characters.begin(), forbidden_characters.end(), word) != forbidden_characters.end() )
			return false;
		if( std::find(accepted_tags.begin(), accepted_tags.end(), tag) == accepted_tags.end() )
			return false;
	}
	return true;		
}


void treat_file(int thread_id, gzFile large_file, string large_filename, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags, 
	unsigned nb_ngrams, unsigned long long& cpt_unique_ngrams)
{
	// to treat the lines
	unsigned char buffer[CHUNK_SIZE];
	unsigned int unzipped_bytes = 1;
	string ngram = "";
	string precedent_ngram = "";
	stringstream token("");
	int err;
							
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
				if( valid_ngram(token.str(), ngram, nb_ngrams, forbidden_characters, accepted_tags) )
				{
					if(precedent_ngram != ngram)
						++ cpt_unique_ngrams;
					precedent_ngram = ngram;
				}
				token.str(std::string());
				token.clear();
			}
		}
	}
}



void add_to_cpt_ngrams(unsigned long long& cpt_unique_ngrams, unsigned long long cpt_unique_ngrams_file)
{
	std::lock_guard<std::mutex> guard(add_mutex);
	cpt_unique_ngrams += cpt_unique_ngrams_file;
}


void generate_file(unsigned thread_id, QueueSafe<string>& queue_filenames, vector<string>& forbidden_characters, 
	vector<string>& accepted_tags, unsigned nb_ngrams, unsigned long long& cpt_unique_ngrams)
{		
	string large_filename;
	while( !queue_filenames.empty() )
	{	
		if( !queue_filenames.try_pop(large_filename) )
			continue;
		
		gzFile large_file = gzopen(large_filename.c_str(), "rb");
		if( large_file == NULL )
		{
			cout << thread_id << " Impossible to open the file " << large_filename;
			continue;	
		}
		unsigned long long cpt_unique_ngrams_file = 0;
		treat_file(thread_id, large_file, large_filename, forbidden_characters, 
			accepted_tags, nb_ngrams, cpt_unique_ngrams_file);
		
		add_to_cpt_ngrams(cpt_unique_ngrams, cpt_unique_ngrams_file);
		
		gzclose(large_file);
	}
}

bool has_suffix(const char* name, string& suffix)
{
	string str = name;
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void collect_filenames(QueueSafe<string>& queue_filenames, string& path_to_files, string suffix)
{
	DIR *pdir = NULL; //pointeur vers un dossier
    struct dirent *pent = NULL; //structure nécessaire a la lecture de répertoire, elle contiendra le nom du/des fichier
    string filename("");
                                           
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

int main(int argc, char** argv)
{

	// args
	string path_to_gz = "/mnt/j/american_eng_gram/2gram/";
	unsigned nb_ngrams = 2;

	// Generate treated files
	QueueSafe<string> queue_filenames;
	vector<string> forbidden_characters {" ",""};
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT", "NUM", ".", "X"};

	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;

	collect_filenames(queue_filenames, path_to_gz, ".gz"); 
	
	unsigned long long cpt_unique_ngrams = 0;
	for(unsigned i=0; i<nb_cores; ++i)
	{
		threads.emplace_back( [&]{generate_file( i+1, queue_filenames, 
			forbidden_characters, accepted_tags, 
			nb_ngrams, cpt_unique_ngrams ); } );
	}
	for(auto& t: threads)
		t.join();
	threads.clear();

	cout << "For " << path_to_gz << ", ngrams = " << nb_ngrams << endl;
	cout << "Nb unique ngrams = " << cpt_unique_ngrams << endl;
    return 0;
}
