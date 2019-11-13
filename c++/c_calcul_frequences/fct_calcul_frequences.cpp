#include "fct_calcul_frequences.hpp"

#define LINE_SIZE 1024

using namespace std;

void treat_freq(FILE* input, FILE* output, string& large_filename, 
	unsigned long long& total_match, unsigned long long& total_volume)
{
	char buffer[LINE_SIZE];
	unsigned position;
	size_t pos;
	string delimiter("\t");
	string token;
	string line("");
	long double freq_match = 0;
	long double freq_volume = 0;
	unsigned cpt_line = 0;		
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
				freq_match = stoull( token ) / static_cast<long double>(total_match);
			else if(position == 4)
				freq_volume = stoull( token ) / static_cast<long double>(total_volume);
		}
		if( line != "" )
			++ position;
		
		if( position != 12 )
			cerr << "WARNING bad line (num " << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
		else
			fprintf(output, "%s\t%.8Le\t%.8Le\n", strtok(buffer, "\n"), freq_match, freq_volume);
		memset(buffer, 0, sizeof(buffer));
	}
}

void update_args(ifstream& file_ini, string& line, string& total_occurrences_files, 
	string& path_to_treated_files, string& path_to_output_files)
{ 
	string tmp("");
	if( line == "total_occurrences_files" || line == "total_occurrences_files=" )
		init_arg(file_ini, line, total_occurrences_files);
	else if( line == "path_to_treated_files" || line == "path_to_treated_files=" )
		init_arg(file_ini, line, path_to_treated_files);
	else if( line == "path_to_output_files" || line == "path_to_output_files=" )
		init_arg(file_ini, line, path_to_output_files);
	else
	{
		cerr << "WARNING don't recognize this variable : " << line
			 << "\nVariables for c_calcul_frequences should be : "
			 << "total_occurrences_files, path_to_treated_files, "
			 << "path_to_output_files\n";
	}
}

bool read_ini_file(const char* ini_filename, string& total_occurrences_files, 
	string& path_to_treated_files, string& path_to_output_files)
{
	ifstream file_ini;
	string line("");
	total_occurrences_files = "";
	path_to_treated_files = "";
	path_to_output_files = "";

	if( ini_filename == NULL )
		file_ini.open("../config.ini");
	else
		file_ini.open(ini_filename);
	
	if( !file_ini )
	{
		cerr << "Impossible to open the ini file or didn't find it\n";
		return false;
	}
	
    while( line != "c_calcul_frequences:" && line != "c_calcul_frequences" )
    {
		if( !getline(file_ini, line) )
			break;
		cout << line << endl;
	}
	
	if( line == "c_calcul_frequences:" || line == "c_calcul_frequences" )
	{
		file_ini >> line;
		read_comment(file_ini, line);
		while( line != "END" && line != "" )
		{
			update_args(file_ini, line, total_occurrences_files, 
				path_to_treated_files, path_to_output_files);
			file_ini >> line;
			read_comment(file_ini, line);
		}
	}
	else
	{
		cerr << "Didn't find the line \"c_calcul_frequences:\" or "
			 << "\"c_calcul_frequences\". Stop.\n";
		return false;
	}
	file_ini.close();
	return true;
}
