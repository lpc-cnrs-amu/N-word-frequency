#include "../util/util.hpp"
#include "../util/OccurrencesSafe.hpp"

#define LINE_SIZE 1024
#define NB_NGRAM 4
#define YEAR 1970

using namespace std;
using namespace std::chrono;

mutex print_mutex;


void calcul_occurrences(unsigned thread_id, QueueSafe<string>& queue_filenames, 
	OccurrencesSafe& occurrences)
{	
	char buffer[LINE_SIZE];
	string line;
	string token;
	string delimiter = "\t";
	string large_filename;
	unsigned position;
	unsigned cpt_line;
	unsigned long long total_match;
	unsigned match;
	size_t pos;
			
	while( !queue_filenames.empty() )
	{
		if( !queue_filenames.try_pop(large_filename) )
			continue;
			
		FILE* input = fopen(large_filename.c_str(), "r");
		if( input == NULL )
		{
			print_message("Impossible to open the file ", large_filename);
			break;	
		}
		print_ok_safe(print_mutex, thread_id, "start", large_filename);
		
		total_match = 0;
		match = 0;
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
					match = stoi( token );
			}
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
				total_match += match;
			}
			memset(buffer, 0, sizeof(buffer));
		}
		occurrences.add_match( total_match );
		fclose(input);
		print_ok_safe(print_mutex, thread_id, "finish", large_filename);
	}
}

bool get_total_volume(const char* filename,
	unsigned long long& total_volume)
{
	ifstream file(filename);
	if( !file )
	{
		cout << "Cannot open file "<< filename << endl;
		return false;
	}
	char tab;
	unsigned year;
	unsigned long long nb_1gram, nb_pages, nb_volumes;
	total_volume = 0;
	
	file.get(tab); // read the first space
	file.get(tab);
	while( tab == '\t'  )
	{
		if (file >> year)
		{
			file.get(tab);
			file >> nb_1gram;
			file.get(tab);
			file >> nb_pages;
			file.get(tab);
			file >> nb_volumes;
			if( year >= YEAR )
				total_volume += nb_volumes;
			file.get(tab);
		}
		else
			tab = ' ';
	}
	
	file.close();	
	return true;	
}

void print_usage(const char* exename)
{
    fprintf(stderr, "NOM \n");
    fprintf(stderr, "\t%s - Calcul le nombre total de ngrams (somme des occurrences des ngrams).\
     Calcul le nombre total de volumes avec le fichier totalcounts. Ecrit ces deux résultats dans un fichier de sortie.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] nom_du_fichier_de_sortie fichier_totalcount chemin_fichiers_treated\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tCalcule le nombre total de ngrams à l'aide des fichiers finissant par '_treated' contenus dans le répertoire de votre choix.\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tAffiche un message d'aide sur la sortie d\"erreur et termine normalement.\n\n");
    fprintf(stderr, "\t nom_du_fichier_de_sortie\n\t\tContiendra le nombre total d'occurrences et de volumes.\n\n");
    fprintf(stderr, "\t fichier_totalcount\n\t\tSert à calculer le nombre total de volumes.\n\n");
    fprintf(stderr, "\t chemin_fichiers_treated\n\t\tLe chemin pour accéder aux fichier _treated.\n\n");
}

bool write_output(const char* filename, unsigned long long total_match, 
	unsigned long long total_volume)
{
	FILE* output = fopen(filename, "w");
	if( output == NULL )
	{
		print_message("Cannot open file ", filename);
		return false;
	}
	fprintf(output, "Total match\tTotal volume\n%llu\t%llu\n", 
		total_match, total_volume);
	fclose(output);
	return true;
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
	
	// Calculate the total nb of volumes with the totalcount file
	unsigned long long total_volume;
	get_total_volume(argv[2], total_volume);
	
	// Calculate the total nb of occurrences with thread (one file per thread)
	QueueSafe<string> queue_filenames;
	OccurrencesSafe occurrences;
	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;
	collect_filenames(queue_filenames, argv[3], "_treated");
	for(unsigned i=0; i<nb_cores; ++i)
	{
		//cout << "create thread " << i+1 << endl;
		threads.emplace_back( [&]{calcul_occurrences( i+1, queue_filenames, occurrences ); } );
	}
	for(auto& t: threads)
		t.join();
	threads.clear();
	
	// Write in output file	
	write_output(argv[1], occurrences.get_total_match(), total_volume);
	
	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl;
	
    return 0;
}

