#include <map>
#include "../util/util.hpp"
#include "../util/Data.hpp"
#include "../a_generate_files/fct_valid_lines.hpp"

#define LINE_SIZE 1024
#define NB_NGRAM 4
#define YEAR 1970

using namespace std;
using namespace std::chrono;

bool collect_tags(string token, string& tag, vector<string>& accepted_tags)
{
	// put every word_tag in vector words_tags
	vector<string> words_tags;
	collect_words_tags(token, words_tags);
	if(words_tags.size() != NB_NGRAM)
		return false;
	
	string word;
	tag = "";
	string delimiter = "_";
	size_t pos = 0;
	
	for (unsigned i=0; i<words_tags.size(); ++i)
	{
		// pas de "_" présent
		if (words_tags[i].find(delimiter) == std::string::npos)
			return false;
			
		while ((pos = words_tags[i].find(delimiter)) != std::string::npos)
		{
			word = words_tags[i].substr(0, pos);
			words_tags[i].erase(0, pos + delimiter.length());
		}
		
		// now words_tags[i] contains only the tag
		if( find(accepted_tags.begin(), accepted_tags.end(), words_tags[i]) == accepted_tags.end() )
			return false;
		tag = tag + words_tags[i] + " ";
	}
	tag.pop_back();
	return true;
}


bool calcul_occurrences(string large_filename, 
	map<std::string, Data*>& tags_to_data, vector<string>& accepted_tags)
{	
	char buffer[LINE_SIZE];
	string line;
	string token;
	string tags;
	string delimiter = "\t";
	unsigned position = 0, cpt_line = 0;
	size_t pos = 0;
	bool good_tags = false;
	
	unsigned nb_year = 0;
	unsigned nb_match = 0;
	unsigned nb_volume = 0;
	float mean_pondere_match = 0;
	float mean_pondere_volume = 0;
	unsigned year_max = 0;
	unsigned year_min = 3000;
	unsigned nb_match_max = 0;
	unsigned nb_match_min = 100000;
	unsigned nb_volume_max = 0;
	unsigned nb_volume_min = 100000;
			
	FILE* input = fopen(large_filename.c_str(), "r");
	if( input == NULL )
	{
		print_message("Impossible to open the file ", large_filename);
		return false;	
	}
	print_message("start", large_filename);
							
	while( fgets(buffer, sizeof(buffer), input) )
	{
		++cpt_line;
		line = buffer;
		position = 0;
		pos = 0;
		//cout << buffer << endl;
					
		// cut by \t
		while ((pos = line.find(delimiter)) != std::string::npos) 
		{
			++ position;
			token = line.substr(0, pos);
			line.erase(0, pos + delimiter.length());
			
			switch(position)
			{
				case 1:
					good_tags = collect_tags(token, tags, accepted_tags);
					break;
				case 2:
					nb_year = stoi( token );
					break;
				case 3:
					nb_match = stoi( token );
					break;
				case 4:
					nb_volume = stoi( token );
					break;
				case 5:
					mean_pondere_match = stof( token );
					break;
				case 6:
					mean_pondere_volume = stof( token );
					break;
				case 7:
					year_max = stoi( token );
					break;
				case 8:
					year_min = stoi( token );
					break;
				case 9:
					nb_match_max = stoi( token );
					break;
				case 10:
					nb_match_min = stoi( token );
					break;
				case 11:
					nb_volume_max = stoi( token );
					break;
				default:
					good_tags = false;
					cout << "WARNING bad line (" << cpt_line 
						 << ") on file " << large_filename << " : " << buffer << "\n";
					cerr << "WARNING bad line (" << cpt_line 
						 << ") on file " << large_filename << " : " << buffer << "\n";
					break;
			}
		}
		if( line != "" )
		{
			nb_volume_min = stoi( line );
			++ position;
		}
		if( position != 12 )
		{
			cout << "WARNING bad line (" << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
			cerr << "WARNING bad line (" << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
		}
		else
		{
			if(good_tags)
			{
				if( tags_to_data.find(tags) == tags_to_data.end() )
					tags_to_data[tags] = new Data();
				
				tags_to_data[tags]->add_somme_year(nb_year);
				tags_to_data[tags]->add_somme_nb_match(nb_match);
				tags_to_data[tags]->add_somme_nb_volume(nb_volume);
				tags_to_data[tags]->add_mean_pondere_match(mean_pondere_match);
				tags_to_data[tags]->add_mean_pondere_volume(mean_pondere_volume);
				tags_to_data[tags]->try_and_change_year_max(year_max);
				tags_to_data[tags]->try_and_change_year_min(year_min);
				tags_to_data[tags]->try_and_change_match_max(nb_match_max);
				tags_to_data[tags]->try_and_change_match_min(nb_match_min);
				tags_to_data[tags]->try_and_change_volume_max(nb_volume_max);
				tags_to_data[tags]->try_and_change_volume_min(nb_volume_min);
			}
		}
		memset(buffer, 0, sizeof(buffer));
	}
	fclose(input);
	print_message("finish", large_filename);
	return true;
	
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
    fprintf(stderr, "\t%s - Calcule la fréquences des tag-grams vu dans les fichiers _treated.\n\n", exename);
    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "\t%s [-h] nom_du_fichier_de_sortie fichier_totalcount chemin_fichiers_treated\n\n", exename);
    fprintf(stderr, "DESCRIPTION \n");
    fprintf(stderr, "\tCalcule la fréquences des tag-grams vu dans les fichiers _treated contenus dans le répertoire de votre choix.\
    Chaque ligne du fichier de sortie sera comme ceci :\n\
    <tag-gram> <nb d'année> <nb d'années> <total occurrences> <total volume> <moyenne des années pondérés par le total d'occurrences> \
    <moyenne des années pondérés par le total de volume> <année max> <année min> <occurrences max> <occurences min> <nb volume max> <nb volume min> <freq match> <freq volume>\n\n");
    fprintf(stderr, "ARGUMENTS\n");
    fprintf(stderr, "\t -h\n\t\tAffiche un message d'aide sur la sortie d\"erreur et termine normalement.\n\n");
    fprintf(stderr, "\t nom_du_fichier_de_sortie\n\t\tContiendra la fréquence pour chaque tag-grams.\n\n");
    fprintf(stderr, "\t fichier_totalcount\n\t\tSert à calculer le nombre total de volumes.\n\n");
    fprintf(stderr, "\t chemin_fichiers_treated\n\t\tLe chemin pour accéder aux fichier _treated.\n\n");
}

void calcul_handler(vector<string>& filenames, map<std::string, Data*>& tags_to_data, 
	vector<string>& accepted_tags)
{
	for(unsigned i=0; i<filenames.size(); ++i)
	{
		if( !calcul_occurrences(filenames[i], tags_to_data, accepted_tags) )
		{
			cout << "didn't process the file " << filenames[i] << "\n";
			cerr << "didn't process the file " << filenames[i] << "\n";
		}
	}
}

bool write_output_frequences_tags_grams(const char* filename, unsigned long long total_match, 
	unsigned long long total_volume, map<std::string, Data*>& tags_to_data)
{
	FILE* output = fopen(filename, "w");
	if( output == NULL )
	{
		print_message("Cannot open file ", filename);
		return false;
	}

	for(auto it=tags_to_data.begin(); it != tags_to_data.end(); ++it)
	{
		fprintf(output, 
			"%s\t%llu\t%llu\t%llu\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%.8e\t%.8e\n", 
			it->first.c_str(), 
			it->second->get_somme_year(), 
			it->second->get_somme_nb_match(), 
			it->second->get_somme_nb_volume(), 
			it->second->calcul_mean_pondere_match(),
			it->second->calcul_mean_pondere_volume(), 
			it->second->get_year_max(),
			it->second->get_year_min(), 
			it->second->get_nb_match_max(), 
			it->second->get_nb_match_min(),
			it->second->get_nb_volume_max(), 
			it->second->get_nb_volume_min(), 
			it->second->get_freq_match(total_match), 
			it->second->get_freq_volume(total_volume) );
	}	
	
	fclose(output);
	return true;	
}

// calcul total occurrences 

void calcul_total_occurrences(
	map<std::string, Data*>& tags_to_data, unsigned long long& total_match)
{
	total_match = 0;
	for(auto it=tags_to_data.begin(); it != tags_to_data.end(); ++it)
		total_match += it->second->get_somme_nb_match();
}


void destroy_data(map<std::string, Data*>& tags_to_data)
{
	for(auto it=tags_to_data.begin(); it != tags_to_data.end(); ++it)
	{
		it->second->~Data();
	}	
	tags_to_data.clear();
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
	unsigned long long total_volume, total_match = 0;
	get_total_volume(argv[2], total_volume);
	
	// Calculate the total nb of occurrences of tags_grams with treated files
	vector<string> filenames;
	vector<string> accepted_tags {"NOUN", "VERB", "ADJ", "ADV", 
		"PRON", "DET", "ADP", "CONJ", "PRT"};
	map<std::string, Data*> tags_to_data;
	collect_filenames(filenames, argv[3], "_treated");
	calcul_handler(filenames, tags_to_data, accepted_tags);
	
	// Write in output file	frequences for each tags-grams
	calcul_total_occurrences(tags_to_data, total_match);
	write_output_frequences_tags_grams(argv[1], total_match, total_volume, tags_to_data);
	
	destroy_data(tags_to_data);
	
	// Calcul time taken
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<std::chrono::minutes>(stop - start);
	cout << "Time taken : " << endl;
	cout << duration.count() << " minutes" << endl;
	
    return 0;
}

