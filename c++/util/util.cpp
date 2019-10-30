#include "util.hpp"

using namespace std;

bool has_suffix(const char* name, string &suffix)
{
	string str = name;
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void collect_filenames(QueueSafe<string>& queue_filenames, const char* path_to_files, string suffix)
{
	DIR *pdir = NULL; //pointeur vers un dossier
    struct dirent *pent = NULL; //structure nécessaire a la lecture de répertoire, elle contiendra le nom du/des fichier
    string path_to_files_str(path_to_files);
    string filename("");
                                           
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
        if(	pent->d_name != NULL && has_suffix(pent->d_name, suffix) )
        {
			filename = pent->d_name;
			filename = path_to_files_str + filename;
			queue_filenames.push_front(filename);
		}
    }
    closedir (pdir);	
}
void collect_filenames(vector<string>& filenames, const char* path_to_files, string suffix)
{
	DIR *pdir = NULL; //pointeur vers un dossier
    struct dirent *pent = NULL; //structure nécessaire a la lecture de répertoire, elle contiendra le nom du/des fichier
    string path_to_files_str(path_to_files);
    string filename("");
                                           
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
        if(	pent->d_name != NULL && has_suffix(pent->d_name, suffix) )
        {
			filename = pent->d_name;
			filename = path_to_files_str + filename;
			filenames.push_back(filename);
		}
    }
    closedir (pdir);	
}
void print_filenames(vector<string>& filenames)
{
	for(unsigned i=0; i<filenames.size(); ++i)
		cout << filenames[i] << endl;
}

void print_message(string message, char* cut_filename)
{
	cout << message << cut_filename << "\n";
	cerr << message << cut_filename << "\n";
}	
void print_message(string message, string cut_filename)
{
	cout << message << cut_filename << "\n";
	cerr << message << cut_filename << "\n";
}

FILE* get_file(int thread_id, string filename, 
	const char* path_directory, string filename_end, string new_filename_end)
{	
	// concat large_filename + num_cut_files
	string delimiter = "/";
	string path_directory_str(path_directory);
	size_t pos = 0;
	while ((pos = filename.find(delimiter)) != std::string::npos) 
		filename.erase(0, pos + delimiter.length());

	pos = filename.find(filename_end);
	if (pos != std::string::npos)
		filename.erase(pos, filename.length());		
	filename += new_filename_end;
	filename = path_directory_str + filename; 
	
	cout << filename << endl;

	// open output file
	FILE* output = fopen(filename.c_str(), "w");
	if( output == NULL )
		print_message("Impossible to open the file ", filename);
	
	if(thread_id == -1)
	{
		cout << "\tDoing : " << filename << "\n";
		cerr << "\tDoing : " << filename << "\n";
	}
	return output;
}


void print_ok_safe(mutex& print_mutex, unsigned thread_id, string message, string filename)
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
		print_message("Cannot open file ", filename);
		return false;
	}
	char buffer[1000];
	if( fgets(buffer, sizeof(buffer), file) == NULL )
	{
		print_message("Error reading the first line. This file is not in a good format : ", filename);
		return false;		
	}
	if( fscanf(file, "%llu\t%llu", &total_match, &total_volume) != 2 )
	{
		print_message("Error reading the second line. This file is not in a good format : ", filename);
		return false;
	}
	fclose(file);
	return true;	
}
