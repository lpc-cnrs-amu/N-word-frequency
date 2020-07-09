#include "fct_generate_files.hpp"

using namespace std;
using namespace std::chrono;

mutex print_mutex;

void generate_file(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	string& path_to_output, vector<string>& forbidden_characters, 
	vector<string>& accepted_tags, unsigned nb_ngrams, 
	unsigned min_year_defined, bool no_number)
{		
	string large_filename;
	while( !queue_filenames.empty() )
	{	
		if( !queue_filenames.try_pop(large_filename) )
			continue;
		
		gzFile large_file = gzopen(large_filename.c_str(), "rb");
		if( large_file == NULL )
		{
			print_message_safe(print_mutex, thread_id, "Impossible to open the file", large_filename);
			continue;	
		}
			
		FILE* output = get_file(thread_id, large_filename, path_to_output, ".gz", "_treated");
		if( output == NULL )
			continue;
		
		print_message_safe(print_mutex, thread_id, "start", large_filename);
		treat_file(thread_id, large_file, output, large_filename, forbidden_characters, 
			accepted_tags, nb_ngrams, min_year_defined, no_number);
		
		gzclose(large_file);
		fclose(output);
		print_message_safe(print_mutex, thread_id, "finish", large_filename);
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
	"\t%s - Generate treated ngrams files with threads.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] [.ini file]\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, 
    "\tGenerate treated ngrams files like this from .gz file :\n\
\t<ngram> <nb of year> <total occurrences> <total volumes>\n \
\t<mean ponderated year by the total of occurrences> \
<mean ponderated year by the total of volumes>\n\
\t<year max> <year min> <occurrences max> <occurences min> \
<nb volume max> <nb volume min>\n\n\
\tTake into account only ngrams with words tagged with :\n\
\tNOUN, VERB, ADJ, ADV, PRON, DET, ADP, CONJ, PRT and words different \
than:\n\t',', '.', '?', '!', '...', ';', ':', '\"', ' ', '''\n\n");
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
	bool no_number;
	const char* ini_filename = argv[1];
	if( read_ini_file(ini_filename, path_to_gz, path_to_output, 
		nb_ngrams, min_year_defined, no_number) )
	{
		cout << path_to_gz << endl;
		cout << path_to_output << endl;
		cout << nb_ngrams << endl;
		cout << min_year_defined << endl;
		cout << no_number << endl;
	}
	else
		return -1;
	
	// Generate treated files
	QueueSafe<string> queue_filenames;
	vector<string> forbidden_characters {",",".","?","!","...",";",
		":","\""," ","","'"};
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT", "NUM"};
	int nb_cores = std::thread::hardware_concurrency() - 1;
	if( nb_cores <= 0 )
		nb_cores = 5;
	vector<thread> threads;

	collect_filenames(queue_filenames, path_to_gz, ".gz");
	for(unsigned i=0; i<(unsigned)nb_cores; ++i)
	{
		threads.emplace_back( [&]{generate_file( i+1, queue_filenames, 
			path_to_output, forbidden_characters, accepted_tags, 
			nb_ngrams, min_year_defined, no_number ); } );
	}
	for(auto& t: threads)
		t.join();
	threads.clear();

	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
    return 0;
}
