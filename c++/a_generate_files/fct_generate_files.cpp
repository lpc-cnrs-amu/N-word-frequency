#include "fct_generate_files.hpp"

using namespace std;

void update_args(ifstream& file_ini, string& line, string& path_to_gz, 
	string& path_to_output, unsigned& nb_ngram, unsigned& min_year)
{ 
	string tmp("");
	if( line == "path_to_gz_files" || line == "path_to_gz_files=" )
	{
		init_arg(file_ini, line, path_to_gz);
	}
	else if( line == "path_to_output_files" || line == "path_to_output_files=" )
	{
		init_arg(file_ini, line, path_to_output);
	}
	else if( line == "nb_ngram" || line == "nb_ngram=" )
	{
		init_arg(file_ini, line, tmp);
		if( valid_nb_ngram(tmp) )
			nb_ngram = stoul(tmp);
		else
			cerr << "WARNING invalid entry for nb_ngram." 
				 << " nb_ngram is now set to 1\n";
	}
	else if( line == "min_year" || line == "min_year=" )
	{
		init_arg(file_ini, line, tmp);
		if( valid_min_year(tmp) )
			min_year = stoul(tmp);
		else
			cerr << "WARNING invalid entry for min_year."
				 << " min_year is now set to 0\n";
	}
	else
	{
		cerr << "WARNING don't recognize this variable : " << line
			 << "\nVariables for a_generate_file should be : "
			 << "path_to_gz_files, path_to_output_files, "
			 << "nb_ngram, min_year\n";
	}
}

bool read_ini_file(const char* ini_filename, string& path_to_gz, 
	string& path_to_output, unsigned& nb_ngrams, unsigned& min_year_defined)
{
	ifstream file_ini;
	string line("");
	path_to_gz = "";
	path_to_output = "";
	nb_ngrams = 1;
	min_year_defined = 0;

	if( ini_filename == NULL )
		file_ini.open("../config.ini");
	else
		file_ini.open(ini_filename);
	
	if( !file_ini )
	{
		cout << "Impossible to open the ini file or didn't find it\n";
		cerr << "Impossible to open the ini file or didn't find it\n";
		return false;
	}
	
    while( line != "a_generate_file:" && line != "a_generate_file" )
    {
		if( !getline(file_ini, line) )
			break;
		cout << line << endl;
	}
	
	if( line == "a_generate_file:" || line == "a_generate_file" )
	{
		file_ini >> line;
		while( line != "END" && line != "" )
		{
			update_args(file_ini, line, path_to_gz, 
				path_to_output, nb_ngrams, min_year_defined);
			file_ini >> line;
		}
	}
	else
	{
		cerr << "Didn't find the line \"a_generate_file:\" or "
			 << "\"a_generate_file\". Stop.\n";
		return false;
	}
	file_ini.close();
	return true;
}

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
