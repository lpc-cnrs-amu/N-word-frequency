#include "../util/util.hpp"
#include "../zlib1211/zlib.h"
#include "fct_valid_lines.hpp"

#define CUT_SIZE 100000
//#define CHUNK_SIZE 1048576
#define CHUNK_SIZE 1024

using namespace std;
using namespace std::chrono;

	
bool generate_file(string large_filename, const char* path_to_output,
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
{	
	// open the .gz file
	gzFile large_file = gzopen(large_filename.c_str(), "rb");
	if( large_file == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	
	// open the output file
	FILE* output = get_file(-1, large_filename, path_to_output, ".gz", "_treated");
	if( output == NULL )
		return false;
	
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
				cout << "\nFINI\n";
                break;
            }
            // ... because of an error
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
					one_valid_line = true;
							
					// find a new ngram so we write the precedent (except for the 1st line of the file)
					if(word_tag != precedent_word_tag && precedent_word_tag != "")
					{
						fprintf(output, "%s\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t %d\t%d\t%d\n", 
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
		cout << "WARNING -- didn't read the entire file: " <<
			"has left :["<< word_tag <<"]\n"; 
		cerr << "WARNING -- didn't read the entire file: " <<
			"has left :["<< word_tag <<"]\n"; 
	}
	
	// write the last treated line
	else
	{
		if(one_valid_line)
		{
			fprintf(output, "%s\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t %d\t%d\t%d\n", 
				word_tag.c_str(), somme_year, somme_nb_match, somme_nb_volume, 
				mean_pondere_match/static_cast<float>(somme_nb_match), mean_pondere_volume/static_cast<float>(somme_nb_volume), 
				year_max, year_min, nb_match_max, nb_match_min, nb_volume_max, nb_volume_min);
		}
	}
	gzclose(large_file);
	fclose(output);
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
	return true;
}



void generate_file_handler(vector<string>& filenames, const char* path_to_output,
	vector<string>& forbidden_characters, vector<string>& accepted_tags)
{
	for(unsigned i=0; i<filenames.size(); ++i)
	{
		if( !generate_file(filenames[i], path_to_output, forbidden_characters, accepted_tags) )
		{
			cout << "didn't process the file " << filenames[i] << "\n";
			cerr << "didn't process the file " << filenames[i] << "\n";
		}
	}
}

void print_usage(const char* exename)
{
    fprintf(stderr, "NOM \n");
    fprintf(stderr, "\t%s - Génère des fichiers de ngrams traités.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] chemin_fichiers_gz chemin_fichiers_sorties\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tGénère des fichiers de ngrams traités comme ceci à partir des fichiers .gz :\n\
    \t<ngram> <nb d'années> <total occurrences> <total volume> <moyenne des années pondérés par le total d'occurrences> <moyenne des années pondérés par le total de volume> <année max> <année min> <occurrences max> <occurences min> <nb volume max> <nb volume min>\
    Prends en compte uniquement les ngrams taggés avec NOUN, VERB, ADJ, ADV, PRON, DET, ADP, CONJ, PRT et les ngrams différents de ',', '.', '?', '!', '...', ';', ':', '\"', ' ', '''\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tAffiche un message d'aide sur la sortie d'erreur et termine normalement.\n\n");
    fprintf(stderr, "\t chemin_fichiers_gz\n\t\tLe chemin pour accéder aux fichier .gz.\n\n");
    fprintf(stderr, "\t chemin_fichiers_sorties\n\t\tLe chemin du répertoire où se trouveront les fichiers de sorties _treated.\n\n");
}

int main(int argc, char** argv)
{
	// print usage if the user has written the command incorrectly
	if( argc != 3 || (argc > 1 && !strcmp(argv[1],"-h")) )
	{
		print_usage(argv[0]);
		return 0;
	}
	auto start = high_resolution_clock::now();
	
	// Generate treated files
	vector<string> filenames;
	vector<string> forbidden_characters {",",".","?","!","...",";",
		":","\""," ","","'"};
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT"};
	collect_filenames(filenames, argv[1], ".gz");
    generate_file_handler(filenames, argv[2], forbidden_characters, accepted_tags);

	
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl;
	
    return 0;
}
