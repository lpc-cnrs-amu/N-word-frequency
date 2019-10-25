#include "../util/util.hpp"

#define LINE_SIZE 1024
#define NB_NGRAM 4

using namespace std;
using namespace std::chrono;


bool calcul_freq(string large_filename, const char* path_to_output,
	unsigned long long& total_match, unsigned long long& total_volume)
{	
	char buffer[LINE_SIZE];
	string line;
	string token;
	string delimiter = "\t";
	unsigned position = 0;
	unsigned cpt_line = 0;
	float freq_match = 0;
	float freq_volume = 0;
	size_t pos = 0;
				
	FILE* input = fopen(large_filename.c_str(), "r");
	if( input == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	print_message("start", large_filename);
	
	// open the output file
	FILE* output = get_file(-1, large_filename, path_to_output, "_treated", "_frequences");
	if( output == NULL )
	{
		fclose(input);
		return false;
	}
				
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
		
			if(position == 3)
				freq_match = stoi( token ) / (total_match*0.1);
			else if(position == 4)
				freq_volume = stoi( token ) / (total_volume*0.1);
		}
		/* si on retire le dernier \t alors on décommente*/
		if( line != "" )
			++ position;
		
		if( position != 12 )
		{
			cout << "WARNING bad line (" << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
			cerr << "WARNING bad line (" << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
		}
		else
		{
			// warning : write "%s\t%.10e\t%.10e\n" if we fix the _treated files (\t in last position)
			fprintf(output, "%s\t%.8e\t%.8e\n", strtok(buffer, "\n"), freq_match, freq_volume);
		}
		memset(buffer, 0, sizeof(buffer));
	}
	fclose(input);
	fclose(output);
	print_message("finish", large_filename);
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
    fprintf(stderr, "\t chemin_fichiers_treated\n\t\tLe chemin pour accéder aux fichier _treated.\n\n");
    fprintf(stderr, "\t chemin_fichiers_sorties\n\t\tLe chemin du répertoire où se trouveront les fichiers de sorties _frequences.\n\n");
}

void calcul_handler(vector<string>& filenames, const char* path_to_output, 
	unsigned long long& total_match, unsigned long long& total_volume)
{
	for(unsigned i=0; i<filenames.size(); ++i)
	{
		if( !calcul_freq(filenames[i], path_to_output, total_match, total_volume) )
		{
			cout << "didn't process the file " << filenames[i] << "\n";
			cerr << "didn't process the file " << filenames[i] << "\n";
		}
	}
}

int main(int argc, char** argv)
{
	// print usage if the user has written the command incorrectly
	if( argc != 4 || (argc > 1 && !strcmp(argv[1],"-h")) )
	{
		print_usage(argv[0]);
		return 0;
	}
	auto start = high_resolution_clock::now();
	
	// Get the total occurrences and the total nb of volumes
	unsigned long long total_match, total_volume;
	if( !get_total_occurrences(argv[1], total_match, total_volume) )
		return -1;

	// Calculate the frequences for each file
	vector<string> filenames;
	collect_filenames(filenames, argv[2], "_treated");
	calcul_handler( filenames, argv[3], total_match, total_volume );

	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl;
	
    return 0;
}

