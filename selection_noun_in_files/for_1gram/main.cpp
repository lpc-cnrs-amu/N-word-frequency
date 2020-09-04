#include <iostream>
#include <iomanip>
#include "../../util/OccurrencesSafe.hpp"
#include "../../util/util.hpp"
#include "Tokeniseur.hpp"
#include <regex>

#define LINE_SIZE 10000

using namespace std;
using namespace std::chrono;

mutex print_mutex;

bool is_upper(const std::wstring& token_wstr)
{
	unsigned i = 0;
	while( token_wstr[i] != END_OF_WORD )
	{
		if( iswupper(token_wstr[i]) )
			return true;
		++ i;
	}
	return false;
}

/*  - un des Ngrams doit être “item_NOUN”
	- pas de “_DET” à la fin
	- pas d’apostrophe au tout début ni à la toute fin (“‘“ à la premiere lettre du GRAM1 et à la dernière lettre du GRAM5)
	- pas de lettre majuscule dans tous les Ngrams
	- pas de signe de ponctuation dans tous les Ngrams (sauf les apostrophe)
	- pas de nombre dans tous les Ngrams
*/
void treat_file(FILE* input, FILE* output, Tokeniseur& arbre, 
	string& large_filename, unsigned nb_ngram, regex& regex_ponctuation)
{
	char buffer[LINE_SIZE];
	unsigned position;
	size_t pos;
	unsigned cpt_line = 0;
	string delimiter("\t");
	string token;
	wstring token_wstr;
	string line("");
	bool words_ok = true;
	unsigned count_POS = 0;
	unsigned count_word = 0;
	while( fgets(buffer, sizeof(buffer), input) )
	{
		++cpt_line;
		line = buffer;
		position = 0;
		pos = 0;
		words_ok = true;
		count_POS = 0;
		count_word = 0;
		while (words_ok && (pos = line.find(delimiter)) != std::string::npos) 
		{
			++ position;
			token = line.substr(0, pos);
			line.erase(0, pos + delimiter.length());
			std::smatch match_ponctuation;
			if( position <= nb_ngram )
			{
				token_wstr = str_to_wstr( token );
				if( is_upper(token_wstr) || std::regex_search(token, match_ponctuation, regex_ponctuation) || !arbre.search(token_wstr) )
					words_ok = false;
				else
					++ count_word;
			}
			else if( position > nb_ngram && position <= nb_ngram*2 )
				++ count_POS;
		}
		if(words_ok)
		{
			if( line != "" )
				++ position;
			if( position != nb_ngram*2+13 || count_POS != 1 || count_word != 1 )
			{
				cerr << "WARNING bad line (" << cpt_line 
					 << ") on file " << large_filename << " : " << buffer << "\n";
			}
			else
				fprintf(output, "%s", buffer);
		}
		memset(buffer, 0, sizeof(buffer));
	}
}

void generate_file(unsigned thread_id, Tokeniseur& arbre, 
	QueueSafe<string>& queue_filenames, string& path_to_output,
	unsigned nb_ngram)
{		
	string large_filename;
	std::regex regex_ponctuation("(\\.|;|\\?|,|/|:|!|§|\\*|\\$|£|\\^|\"|#|\\(|\\[|\\]|\\)|\\\\|/|-|\\+|&|~|<|>|«|»|\\{|\\})+");
	while( !queue_filenames.empty() )
	{	
		if( !queue_filenames.try_pop(large_filename) )
			continue;
		FILE* input = fopen(large_filename.c_str(), "r");
		if( input == NULL )
		{
			print_message_safe(print_mutex, thread_id, "Impossible to open the file", large_filename);
			continue;	
		}
		FILE* output = get_file(thread_id, large_filename, path_to_output, "_frequences", "_frequencies");
		if( output == NULL )
			continue;
		print_message_safe(print_mutex, thread_id, "start", large_filename);
		treat_file(input, output, arbre, large_filename, nb_ngram, regex_ponctuation);
		fclose(input);
		fclose(output);
		print_message_safe(print_mutex, thread_id, "finish", large_filename);
	}
}

int main(int argc, char **argv)
{
	/*
	if (argc-1!=2)
	{
		cerr << "syntaxe : ./tokenisation fichier_lexique fichier_corpus \n" << endl;
		return -1;
	}
	*/
	
	// ./arbre LEX1GRAM.csv path_to_file path_to_output nb_ngram

	auto start = high_resolution_clock::now();
	
	// Construct prefix tree
	Tokeniseur arbre;
	arbre.load(argv[1]);
	cout << "The tree has been successfuly build\n";
	
	// Number of threads
	int nb_cores = std::thread::hardware_concurrency() - 1;
	if( nb_cores <= 0 )
		nb_cores = 5;
	vector<thread> threads;

	// Launch threads
	string path_to_output = argv[3];
	string path_to_files = argv[2];
	unsigned nb_ngram = stoul(argv[4]);
	QueueSafe<string> queue_filenames;
	collect_filenames(queue_filenames, path_to_files, "_frequences");
	
	for(unsigned i=0; i<(unsigned)nb_cores; ++i)
	{
		threads.emplace_back( [&]{generate_file( i+1, arbre, 
			queue_filenames, path_to_output, nb_ngram ); } );
	}
	
	// Wait for all threads to finish
	for(auto& t: threads)
		t.join();
	threads.clear();

	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << duration.count() << " minutes" << endl;
	
	return 0;
}
