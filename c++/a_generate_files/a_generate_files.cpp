#include "../zlib1211/zlib.h"
#include "fct_valid_lines.hpp"
#include "fct_generate_files.hpp"

#define CHUNK_SIZE 1024

using namespace std;
using namespace std::chrono;
		
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
bool generate_file(string& large_filename, string& path_to_output,
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
void generate_file_handler(vector<string>& filenames, string& path_to_output,
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
    fprintf(stderr, 
	"\t%s - Generate treated ngrams files.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] [.ini file]\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, 
    "\tGenerate treated ngrams files like this from .gz file :\n\
\t<ngram> <nb of year> <total occurrences> <total volumes> \
<mean ponderated year by the total of occurrences> \
<mean ponderated year by the total of volumes> \
<year max> <year min> <occurrences max> <occurences min> \
<nb volume max> <nb volume min>\n\
Take into account only ngrams with words tagged with \
NOUN, VERB, ADJ, ADV, PRON, DET, ADP, CONJ, PRT and words different \
than ',', '.', '?', '!', '...', ';', ':', '\"', ' ', '''\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tPrint this message on stderr.\n\n");
    fprintf(stderr, 
    "\t .ini file\n\t\tFile containing arguments for this program.\
 If not specified, take config.ini in the c++ folder.\n\n");
}

int main(int argc, char** argv)
{
	// print usage if the user has written the command incorrectly
	if( argc > 2 || (argc > 1 && !strcmp(argv[1],"-h")) )
	{
		print_usage(argv[0]);
		return 0;
	}
	auto start = high_resolution_clock::now();
	
	// Read ini file to find args
	string path_to_gz, path_to_output;
	unsigned nb_ngrams, min_year_defined;
	const char* ini_filename = argv[0];
	if( argc <= 1 )
		ini_filename = NULL;
	if( read_ini_file(ini_filename, path_to_gz, path_to_output, 
		nb_ngrams, min_year_defined) )
	{
		cout << path_to_gz << endl;
		cout << path_to_output << endl;
		cout << nb_ngrams << endl;
		cout << min_year_defined << endl;
	}
	else
		return 0;
	
	// Generate treated files
	vector<string> filenames;
	vector<string> forbidden_characters {",",".","?","!","...",";",
		":","\""," ","","'"};
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT"};
	collect_filenames(filenames, path_to_gz, ".gz");
    generate_file_handler(filenames, path_to_output, forbidden_characters, 
		accepted_tags, nb_ngrams, min_year_defined);
	
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}
