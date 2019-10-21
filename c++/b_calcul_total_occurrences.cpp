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
#include "OccurrencesSafe.hpp"

#define LINE_SIZE 1024
#define NB_NGRAM 4

using namespace std;
using namespace std::chrono;

mutex print_mutex;

bool has_suffix(const char* name, string &suffix);

void collect_filenames(QueueSafe<string>& queue_filenames);

void print_error(string message, char* cut_filename);
void print_error(string message, string cut_filename);
void print_ok_safe(unsigned thread_id, string message, string filename);

void calcul_occurrences(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	OccurrencesSafe& occurrences);


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
    string path_to_files("/mnt/j/eng_4grams_treated_thread/");
    string filename("");
    string suffix("_treated");
                                           
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
	
void print_error(string message, char* cut_filename)
{
	cout << message << cut_filename << "\n";
	cerr << message << cut_filename << "\n";
}	
void print_error(string message, string cut_filename)
{
	cout << message << cut_filename << "\n";
	cerr << message << cut_filename << "\n";
}		


void calcul_occurrences(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	OccurrencesSafe& occurrences)
{	
	char buffer[LINE_SIZE];
	string line;
	string token;
	string delimiter = "\t";
	string large_filename;
	unsigned position = 0;
	size_t pos = 0;
			
	while( !queue_filenames.empty() )
	{
		if( !queue_filenames.try_pop(large_filename) )
			continue;
			
		FILE* input = fopen(large_filename.c_str(), "r");
		if( input == NULL )
		{
			print_error("Impossible to open the file ", large_filename);
			break;	
		}
		print_ok_safe(thread_id, "start", large_filename);
								
		while( fgets(buffer, sizeof(buffer), input) )
		{
			line = buffer;
			position = 0;
			pos = 0;
						
			// cut by \t
			while ((pos = line.find(delimiter)) != std::string::npos) 
			{
				++ position;
				token = line.substr(0, pos);
				line.erase(0, pos + delimiter.length());
				
				if(position > 4)
					break;
				else if(position == 3)
					occurrences.add_match( stoi( token ) );
				else if(position == 4)
					occurrences.add_volume( stoi( token ) );
			}
			memset(buffer, 0, sizeof(buffer));
		}
		fclose(input);
		print_ok_safe(thread_id, "finish", large_filename);
	}
}

void print_ok_safe(unsigned thread_id, string message, string filename)
{
	lock_guard<mutex> guard(print_mutex);
	cout << "Thread " << thread_id << " " << message << " " << filename << "\n";
	cerr << "Thread " << thread_id << " " << message << " " << filename << "\n";
}


int main(int argc, char** argv)
{
	QueueSafe<string> queue_filenames;
	OccurrencesSafe occurrences;
	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;

	collect_filenames(queue_filenames);
	auto start = high_resolution_clock::now();
	for(unsigned i=0; i<nb_cores; ++i)
	{
		cout << "create thread " << i+1 << endl;
		threads.emplace_back( [&]{calcul_occurrences( i+1, queue_filenames, occurrences ); } );
	}
	for(auto& t: threads)
		t.join();
	threads.clear();
	
	FILE* output = fopen(argv[1], "w");
	if( output == NULL )
	{
		print_error("Cannot open file ", argv[1]);
		return -1;
	}
	fprintf(output, "Total match\tTotal volume\n%llu\t%llu\n", 
		occurrences.get_total_match(), occurrences.get_total_volume());

	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl;
	
    return 0;
}

