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

void calcul_freq(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	unsigned long long& total_match, unsigned long long& total_volume);


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

FILE* get_file(unsigned thread_id, string filename)
{	
	// concat large_filename + num_cut_files
	string delimiter = "/";
	size_t pos = 0;
	while ((pos = filename.find(delimiter)) != std::string::npos) 
		filename.erase(0, pos + delimiter.length());

	pos = filename.find("_treated");
	if (pos != std::string::npos)
		filename.erase(pos, filename.length());		
	filename += "_frequences";
	filename = "/mnt/j/eng_4grams_frequences_thread/" + filename;
	
	cout << filename << endl;

	// open output file
	FILE* output = fopen(filename.c_str(), "w");
	if( output == NULL )
		print_error("Impossible to open the file ", filename);
	cout << "\tThread " << thread_id << " : " << filename << "\n";
	cerr << "\tThread " << thread_id << " : " << filename << "\n";

	return output;
}


void calcul_freq(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	unsigned long long& total_match, unsigned long long& total_volume)
{	
	char buffer[LINE_SIZE];
	string line;
	string token;
	string delimiter = "\t";
	string large_filename;
	unsigned position = 0;
	unsigned nb_match = 0;
	unsigned nb_volume = 0;
	unsigned cpt_line = 0;
	float freq_match = 0;
	float freq_volume = 0;
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
		
		// open the output file
		FILE* output = get_file(thread_id, large_filename);
		if( output == NULL )
			break;
					
		print_ok_safe(thread_id, "start", large_filename);
		
		cpt_line = 0;		
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
				
				if(position > 4)
					break;
				else if(position == 3)
					freq_match = stoi( token ) / (total_match*0.1);
				else if(position == 4)
					freq_volume = stoi( token ) / (total_volume*0.1);
			}
			
			if( position < 4 )
			{
				cout << "WARNING bad line on file " << large_filename << ", line " << cpt_line << " : " << buffer << "\n";
				cerr << "WARNING bad line on file " << large_filename << ", line " << cpt_line << " : " << buffer << "\n";
			}
			else
			{
				// warning : write "%s\t%.10e\t%.10e\n" if we fix the _treated files (\t in last position)
				fprintf(output, "%s%.10e\t%.10e\n", strtok(buffer, "\n"), freq_match, freq_volume);
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

bool get_total_occurrences(const char* filename, 
	unsigned long long& total_match,
	unsigned long long& total_volume)
{
	FILE* file = fopen(filename, "r");
	if( file == NULL )
	{
		print_error("Cannot open file ", filename);
		return false;
	}
	char buffer[1000];
	fgets(buffer, sizeof(buffer), file);
	fscanf(file, "%llu\t%llu", &total_match, &total_volume);
	fclose(file);
	return true;	
}

void print_usage(const char* exename)
{
    fprintf(stderr, "NOM \n");
    fprintf(stderr, "\t%s - Calcul la fréquence de chaque ngrams.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] fichier_total_occurrences\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tCalcule la fréquence de chaque ngrams des fichiers finissant par '_treated' contenus dans le répertoire de votre choix. \
    Utilise le fichier fichier_total_occurrences contenant le nombre total d'occurrences et de volumes. Ce fichier est obtenu avec le programme b_calcul_total_occurrences.cpp\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tAffiche un message d'aide sur la sortie d'erreur et termine normalement.\n\n");
    fprintf(stderr, "\t fichier_total_occurrences\n\t\tContient le nombre total d'occurrences et de volumes. Ce fichier est obtenu avec le programme b_calcul_total_occurrences.cpp\n\n");
}

int main(int argc, char** argv)
{
	// print usage if the user has written the command incorrectly
	if( argc != 2 || (argc > 1 && !strcmp(argv[1],"-h")) )
	{
		print_usage(argv[0]);
		return 0;
	}	
	
	// Get the total occurrences and the total nb of volumes
	unsigned long long total_match, total_volume;
	if( !get_total_occurrences(argv[1], total_match, total_volume) )
		return -1;

	// Calculate the frequences for each file in the queue_filenames
	QueueSafe<string> queue_filenames;
	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;	
	collect_filenames(queue_filenames);
	auto start = high_resolution_clock::now();
	for(unsigned i=0; i<nb_cores; ++i)
	{
		cout << "create thread " << i+1 << endl;
		threads.emplace_back( [&]{calcul_freq( i+1, queue_filenames, total_match, total_volume ); } );
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

