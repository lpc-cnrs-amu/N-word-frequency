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
std::mutex print_mutex;
using namespace std;
#define LINE_SIZE 10000

void print_message_safe(mutex& print_mutex, unsigned thread_id, string message, string filename)
{
	lock_guard<mutex> guard(print_mutex);
	cout << "Thread " << thread_id << " " << message << " " << filename << "\n";
	cerr << "Thread " << thread_id << " " << message << " " << filename << "\n";
}

FILE* get_file(int thread_id, string filename, 
	string& path_directory, string filename_end, string new_filename_end)
{	
	// concat large_filename + num_cut_files
	string delimiter = "/";
	size_t pos = 0;
	while ((pos = filename.find(delimiter)) != std::string::npos) 
		filename.erase(0, pos + delimiter.length());

	pos = filename.find(filename_end);
	if (pos != std::string::npos)
		filename.erase(pos, filename.length());		
	filename += new_filename_end;
	filename = path_directory + filename; 
	
	cout << filename << endl;

	// open output file
	FILE* output = fopen(filename.c_str(), "w");
	if( output == NULL )
		print_message_safe(print_mutex, thread_id, "Impossible to open the file ", filename);
	return output;
}

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

bool separate_words_and_tags(string ngram_original, string& words, string& tags, 
	unsigned nb_ngram, int thread_id)
{
	words = "";
	tags = "";
	
	vector<string> words_tags;
	collect_words_tags(ngram_original, words_tags);
	
	if( words_tags.size() != nb_ngram )
		return false;
		
	string delimiter = "_";
	size_t pos = 0;
	for (unsigned i=0; i<words_tags.size(); ++i)
	{		
		if ( (pos = words_tags[i].find(delimiter)) == std::string::npos )
			return false;
		
		words = words + words_tags[i].substr(0, pos) + "\t";
		words_tags[i].erase(0, pos + delimiter.length());
		tags = tags + words_tags[i] + "\t";
	}
	// remove last "\t"
	words.pop_back();
	tags.pop_back();
	return true;		
}


void treat_file(int thread_id, FILE* input, FILE* output, 
	string large_filename, unsigned nb_ngrams)
{
	// to treat the lines
	char buffer[LINE_SIZE];
	string ngram("");
	string words("");
	string tags("");
	string line("");
	string delimiter("\t");
	size_t pos;
							
	while( fgets(buffer, sizeof(buffer), input) )
	{	
		line = buffer;
		memset(buffer, 0, sizeof(buffer));
		
		// ngram = contains the ngram
		// line = contains the rest of the lines (without the ngram)
		pos = line.find(delimiter);
		ngram = line.substr(0, pos);
		line.erase(0, pos + delimiter.length());
		
		
		
		if( separate_words_and_tags(ngram, words, tags, nb_ngrams, thread_id) )
		{
			//print_message_safe(print_mutex, thread_id, "words= ", words);
			//print_message_safe(print_mutex, thread_id, "tags= ", tags);
			fprintf(output, "%s\t%s\t%s", 
				words.c_str(), tags.c_str(), line.c_str());
		}
		else
			print_message_safe(print_mutex, thread_id, "Pas le bon nb de ngram", large_filename);
	}
}

void generate_file(unsigned thread_id, 
	QueueSafe<string>& queue_filenames, unsigned nb_ngrams, string path_to_output)
{		
	string large_filename;
	while( !queue_filenames.empty() )
	{	
		if( !queue_filenames.try_pop(large_filename) )
			continue;
		
		FILE* large_file = fopen(large_filename.c_str(), "r");
		if( large_file == NULL )
		{
			print_message_safe(print_mutex, thread_id, "Impossible to open the file", large_filename);
			continue;	
		}
		
		FILE* output = get_file(thread_id, large_filename, path_to_output, "_frequences", "_frequencies");
		if( output == NULL )
			continue;		
		
		print_message_safe(print_mutex, thread_id, "start", large_filename);
		treat_file(thread_id, large_file, output, large_filename, nb_ngrams);
		
		fclose(large_file);
		fclose(output);
		print_message_safe(print_mutex, thread_id, "finish", large_filename);
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
	string path_to_files = "/mnt/c/Users/Marjorie/Documents/git_repo/freqNwords/files_frequences/";
	string path_to_output = "/mnt/c/Users/Marjorie/Documents/git_repo/freqNwords/files_frequences/";
	unsigned nb_ngrams = 4;

	// Generate treated files
	QueueSafe<string> queue_filenames;

	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;

	collect_filenames(queue_filenames, path_to_files, "_frequences"); 
	cout << "For " << path_to_files << ", ngrams = " << nb_ngrams << endl;
	for(unsigned i=0; i<nb_cores; ++i)
	{
		threads.emplace_back( [&]{generate_file( i+1, queue_filenames, 
			nb_ngrams, path_to_output); } );
	}
	for(auto& t: threads)
		t.join();
	threads.clear();

    return 0;
}
