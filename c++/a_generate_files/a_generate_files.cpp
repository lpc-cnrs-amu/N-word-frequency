#include "../util/util.hpp"
#include "../zlib1211/zlib.h"
#include "fct_valid_lines.hpp"

#define CHUNK_SIZE 1024

using namespace std;
using namespace std::chrono;

/*! \brief Update each elements of the output file.
 * 
 * \param output The output file 
 * \param ngram The actual ngram read
 * \param precedent_ngram The precedent ngram read
 * \param somme_year Number of year where the ngram appear (from min year to 2009)
 * \param somme_nb_match Sum of occurrences through the years (from min year to 2009)
 * \param somme_nb_volume Sum of volumes through the years (from min year to 2009)
 * \param mean_pondere_match Mean year ponderated by somme_nb_match
 * \param mean_pondere_volume Mean year ponderated by somme_nb_volume
 * \param year_max The last year where the ngram appears (most recent year)
 * \param year_min The first year where the ngram appears (limited to min year)
 * \param nb_match_max 
 * \param nb_match_min
 * \param nb_volume_max
 * \param nb_volume_min
 * \param year
 * \param nb_match
 * \param nb_volume
 */
void treat_line(FILE* output, string& ngram, string& precedent_ngram, unsigned& somme_year,
	unsigned& somme_nb_match, unsigned& somme_nb_volume, float& mean_pondere_match,
	float& mean_pondere_volume,	unsigned& year_max,	unsigned& year_min,
	unsigned& nb_match_max,	unsigned& nb_match_min,	unsigned& nb_volume_max,
	unsigned& nb_volume_min, unsigned year, unsigned nb_match, unsigned nb_volume)
{	
	// find a new ngram so we write the precedent (except for the 1st line of the file)
	if( ngram != precedent_ngram && precedent_ngram != "" )
	{
		fprintf(output, 
			"%s\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\n", 
			precedent_ngram.c_str(), somme_year, somme_nb_match, 
			somme_nb_volume, mean_pondere_match/static_cast<float>(somme_nb_match), 
			mean_pondere_volume/static_cast<float>(somme_nb_volume), 
			year_max, year_min, nb_match_max, nb_match_min, 
			nb_volume_max, nb_volume_min);
			
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
}

/*! \brief Says if the .gz file has been entirely read or not.
 * 
 * \param token Is empty if the .gz file has been entirely read
 * \return False if the file has been entirely read, else true
 */
bool file_not_entirely_read(stringstream& token)
{
	string buff;
	bool not_empty = static_cast<bool>(token >> buff);
	if(not_empty)
		print_message("WARNING didn't read the entire file, has left : ", buff);
	return not_empty;
}		
		
/*! \brief Generates treated file from a .gz file.
 * 
 * \param large_filename File name of the .gz file
 * \param path_to_output Path where the generated file will be
 * \param forbidden_characters Forbidden words : , . ? ! ... ; : ' "
 * \param accepted_tags Accepted tags : "NOUN", "VERB", "ADJ", "ADV", 
 * "PRON", "DET", "ADP", "CONJ", "PRT"
 * \param nb_ngram Number of word_tag (the n in ngram)
 * \param min_year_defined The minimum year you defined
 * \return True if the function has been executed without error, else false
 */
bool generate_file(string large_filename, const char* path_to_output,
	vector<string>& forbidden_characters, vector<string>& accepted_tags,
	unsigned nb_ngram, unsigned min_year_defined)
{	
	// open the .gz file
	gzFile large_file = gzopen(large_filename.c_str(), "rb");
	if( large_file == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	
	// open the output file
	FILE* output = get_file(-1, large_filename, 
		path_to_output, ".gz", "_treated");
	if( output == NULL )
		return false;
	
	// to treat the lines
	unsigned char buffer[CHUNK_SIZE];
	unsigned int unzipped_bytes = 1;
	string ngram;
	string precedent_ngram = "";
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
				print_message("finish ", large_filename);
                break;
            }
            // ... because of an error
            else 
			{
				const char * error_string = gzerror (large_file, &err);
				if (err) 
				{
					print_message("Error : ", error_string);
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
				if( valid_line(token.str(), ngram, year, nb_match, 
					nb_volume, forbidden_characters, accepted_tags, 
					nb_ngram, min_year_defined) )
				{
					one_valid_line = true;
					treat_line(output, ngram, precedent_ngram, somme_year,
						somme_nb_match, somme_nb_volume, mean_pondere_match,
						mean_pondere_volume, year_max, year_min,
						nb_match_max, nb_match_min, nb_volume_max,
						nb_volume_min, year, nb_match, nb_volume);
					precedent_ngram = ngram;
				}
				token.str(std::string());
				token.clear();
			}
		}
	}
	// write the last treated line
	if( one_valid_line && !file_not_entirely_read(token) )
	{
		fprintf(output, 
			"%s\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\n", 
			ngram.c_str(), somme_year, somme_nb_match, somme_nb_volume, 
			mean_pondere_match/static_cast<float>(somme_nb_match), 
			mean_pondere_volume/static_cast<float>(somme_nb_volume), 
			year_max, year_min, nb_match_max, nb_match_min, 
			nb_volume_max, nb_volume_min);
	}
	gzclose(large_file);
	fclose(output);
	return true;
}

/*! \brief Calls generate_file() for every .gz files from the 
 * folder you defined
 * 
 * \param filenames All .gz file names from the folder you defined
 * \param path_to_output Path where the generated file will be
 * \param forbidden_characters Forbidden words : , . ? ! ... ; : ' "
 * \param accepted_tags Accepted tags : "NOUN", "VERB", "ADJ", "ADV", 
 * "PRON", "DET", "ADP", "CONJ", "PRT"
 * \param nb_ngram Number of word_tag (the n in ngram)
 * \param min_year_defined The minimum year you defined
 */
void generate_file_handler(vector<string>& filenames, const char* path_to_output,
	vector<string>& forbidden_characters, vector<string>& accepted_tags,
	unsigned nb_ngram, unsigned min_year_defined)
{
	for(unsigned i=0; i<filenames.size(); ++i)
	{
		if( !generate_file(filenames[i], path_to_output, 
			forbidden_characters, accepted_tags, nb_ngram, min_year_defined) )
			print_message("didn't process the file entirely : ", filenames[i]);
	}
}

/*! \brief Print usage of the program
 * 
 * \param exename Name of the executable
 */
void print_usage(const char* exename)
{
    fprintf(stderr, "NAME \n");
    fprintf(stderr, "\t%s - Generate treated ngrams files.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] chemin_fichiers_gz chemin_fichiers_sorties\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tGenerate treated ngrams files like this from .gz file :\n\
\t<ngram> <nb of year> <total occurrences> <total volumes> \
<mean ponderated year by the total of occurrences> <mean ponderated year by the total of volumes> \
<year max> <year min> <occurrences max> <occurences min> <nb volume max> <nb volume min>\n\
Take into account only ngrams with words tagged with NOUN, VERB, ADJ, ADV, PRON, DET, ADP, CONJ, PRT and words different than ',', '.', '?', '!', '...', ';', ':', '\"', ' ', '''\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tPrint this message on stderr.\n\n");
    fprintf(stderr, "\t chemin_fichiers_gz\n\t\tLe chemin pour accéder aux fichier .gz.\n\n");
    fprintf(stderr, "\t chemin_fichiers_sorties\n\t\tLe chemin du répertoire où se trouveront les fichiers de sorties _treated.\n\n");
}

bool read_ini_file(const char* ini_filename, char* path_to_gz, 
	char* path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined)
{
	FILE* file_ini = NULL; 
	char buffer[1024];

	if( ini_filename == NULL )
		file_ini = fopen("../config.ini", "w");
	else
		file_ini = fopen(ini_filename, "w");
	
	if( file_ini == NULL )
	{
		cout << "Impossible to open the ini file or didn't find it\n";
		cerr << "Impossible to open the ini file or didn't find it\n";
		return false;
	}
	
	while(1)
	{
		if( fgets(buffer, sizeof(buffer), file_ini) != NULL )
		{
			
		}
	}
		
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
    generate_file_handler(filenames, argv[2], forbidden_characters, 
		accepted_tags, nb_ngram, min_year_defined);
	
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}
