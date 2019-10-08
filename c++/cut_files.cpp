#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <algorithm>
#include <chrono>

#define CUT_SIZE 100000
#define NB_NGRAM 4
#define YEAR 1970

using namespace std;
using namespace std::chrono;

void print_filenames(vector<char*>& filenames);
void collect_filenames(vector<char*>& filenames);
void cut_large_file_into_smaller_files(char* large_filename, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);
void collect_words_tags(string line, vector<string>& words_tags);
bool valid_words_tags(string line, vector<string>& forbidden_characters,
	vector<string>& accepted_tags);
bool valid_line(string line, string& words_tags, unsigned& year, 
	unsigned& nb_match, unsigned& nb_volume, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags);
void cut_large_file_into_smaller_files_handler(vector<char*>& filenames);
void destroy_filenames(vector<char*>& filenames);


void print_filenames(vector<char*>& filenames)
{
	for(unsigned i=0; i<filenames.size(); ++i)
		cout << filenames[i] << endl;
}

void collect_filenames(vector<char*>& filenames)
{
	DIR *pdir = NULL; //pointeur vers un dossier
    struct dirent *pent = NULL; //structure nécessaire a la lecture de répertoire, elle contiendra le nom du/des fichier
    char const* path_to_files = "/mnt/c/Users/Marjorie/Documents/git_repo/freqNwords/Files_gz/";
    char* filename = NULL;
                                           
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
        if(strlen(pent->d_name) > 2)
        {
			filename = new char[strlen(path_to_files) + strlen(pent->d_name) + 1];
			strcpy(filename, path_to_files);
			strcat(filename, pent->d_name);
			filenames.push_back(filename);
		}
    }
    closedir (pdir);	
}

void cut_large_file_into_smaller_files(char* large_filename, 
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
{
	unsigned long long num_cut_files = 1;

	ifstream large_file(large_filename, ios::binary); //change binary
	if( !large_file )
	{
		cerr << "Impossible to open the file " << large_filename << endl;
		exit(EXIT_FAILURE);			
	}
	ofstream output;
	char* cut_filename = NULL;
	
	// change num_cut_files to char*
	char const* num_cut_filename = to_string(num_cut_files).c_str();
	
	// concat large_filename + num_cut_files
	cut_filename = new char[strlen(large_filename) + strlen(num_cut_filename) + 1];
	strcpy(cut_filename, large_filename);
	strcat(cut_filename, num_cut_filename);
	
	
	output.open(cut_filename, ios::out | ios::trunc);
	if(!output)
	{
		cerr << "Impossible to open the file " << cut_filename << endl;
		exit(EXIT_FAILURE);		
	}
	string line;
	string words_tags = "";
	unsigned year, nb_match, nb_volume;
	int nb_lines = 0;
	
	cout << "Doing " << large_filename << "..." << endl;
	cout << "\tDoing " << cut_filename << "..." << endl;
	
	while(std::getline(large_file, line))
	{
		
		if(nb_lines >= CUT_SIZE)
		{
			nb_lines = 0;
			output.close();
			++num_cut_files;
			delete[] cut_filename;
			
			// change num_cut_files to char*
			char const* num_cut_filename = to_string(num_cut_files).c_str();
			
			// concat large_filename + num_cut_files
			cut_filename = new char[strlen(large_filename) + strlen(num_cut_filename) + 1];
			strcpy(cut_filename, large_filename);
			strcat(cut_filename, num_cut_filename);
			
			output.open(cut_filename, ios::out | ios::trunc);
			if(!output)
			{
				cerr << "Impossible to open the file " << cut_filename << endl;
				exit(EXIT_FAILURE);		
			}
			cout << "\tDoing " << cut_filename << "..." << endl;	
		}
		
		if(valid_line(line, words_tags, year, nb_match, nb_volume, forbidden_characters, accepted_tags))
		{
			output << words_tags << "\t" << year << "\t" << nb_match << "\t" << nb_volume << endl;
			cout << "CORRECT: " << words_tags << "\t" << year << "\t" << nb_match << "\t" << nb_volume << endl;
			++ nb_lines;
		}
	}
	delete[] cut_filename;
	output.close();
	large_file.close();
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

// line = "mot_TAG mot_TAG mot_TAG mot_TAG"
// words_tags = {"mot_TAG", "mot_TAG", "mot_TAG", "mot_TAG"}
bool valid_words_tags(string line,
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
{
	vector<string> words_tags;
	collect_words_tags(line, words_tags);
	
	if(words_tags.size() != NB_NGRAM)
		return false;
	
	string word;
	string tag;
	string delimiter = "_";
	size_t pos = 0;
	// treat word_tag
	for (unsigned i=0; i<words_tags.size(); ++i)
	{	
		// pas de _ présent
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
			if( !valid_words_tags(token, forbidden_characters, accepted_tags) )
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



