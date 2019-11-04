#include "fct_calcul_total_occurrences.hpp"

#define LINE_SIZE 1024

using namespace std;

void update_args(ifstream& file_ini, string& line, string& output_file_name, 
	string& totalcount_file, string& path_to_treated_files, 
	unsigned& min_year_defined)
{ 
	string tmp("");
	if( line == "output_file_name" || line == "output_file_name=" )
		init_arg(file_ini, line, output_file_name);
	else if( line == "totalcount_file" || line == "totalcount_file=" )
		init_arg(file_ini, line, totalcount_file);
	else if( line == "path_to_treated_files" || line == "path_to_treated_files=" )
		init_arg(file_ini, line, path_to_treated_files);
	else if( line == "min_year" || line == "min_year=" )
	{
		init_arg(file_ini, line, tmp);
		if( valid_min_year(tmp) )
			min_year_defined = stoul(tmp);
		else
			cerr << "WARNING invalid entry for min_year."
				 << " min_year is now set to 0\n";
	}
	else
	{
		cerr << "WARNING don't recognize this variable : " << line
			 << "\nVariables for b_calcul_total_occurrences should be : "
			 << "output_file_name, totalcount_file, "
			 << "path_to_treated_files, min_year\n";
	}
}

bool read_ini_file(const char* ini_filename, string& output_file_name, 
	string& totalcount_file, string& path_to_treated_files, 
	unsigned& min_year_defined)
{
	ifstream file_ini;
	string line("");
	output_file_name = "";
	totalcount_file = "";
	path_to_treated_files = "";
	min_year_defined = 0;

	if( ini_filename == NULL )
		file_ini.open("../config.ini");
	else
		file_ini.open(ini_filename);
	
	if( !file_ini )
	{
		cerr << "Impossible to open the ini file or didn't find it\n";
		return false;
	}
	
    while( line != "b_calcul_total_occurrences:" && line != "b_calcul_total_occurrences" )
    {
		if( !getline(file_ini, line) )
			break;
		cout << line << endl;
	}
	
	if( line == "b_calcul_total_occurrences:" || line == "b_calcul_total_occurrences" )
	{
		file_ini >> line;
		while( line != "END" && line != "" )
		{
			update_args(file_ini, line, output_file_name, 
				totalcount_file, path_to_treated_files, 
				min_year_defined);
			file_ini >> line;
		}
	}
	else
	{
		cerr << "Didn't find the line \"b_calcul_total_occurrences:\" or "
			 << "\"b_calcul_total_occurrences\". Stop.\n";
		return false;
	}
	file_ini.close();
	return true;
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

void treat_occurrences(FILE* input, string& large_filename, 
	unsigned long long& total_match)
{
	char buffer[LINE_SIZE];
	unsigned position;
	size_t pos;
	unsigned match;
	unsigned cpt_line = 0;
	string delimiter("\t");
	string token;
	string line("");
	
	while( fgets(buffer, sizeof(buffer), input) )
	{	
		++cpt_line;
		line = buffer;
		position = 0;
		pos = 0;
		match = 0;
				
		// cut by \t
		while ((pos = line.find(delimiter)) != std::string::npos) 
		{
			++ position;
			token = line.substr(0, pos);
			line.erase(0, pos + delimiter.length());
			
			if( position == 3 )
				match = stoi( token );
		}
		if( line != "" )
			++ position;
		if( position != 12 )
		{
			cerr << "WARNING bad line (" << cpt_line 
				 << ") on file " << large_filename << " : " << buffer << "\n";
		}
		else
			total_match += match;
		memset(buffer, 0, sizeof(buffer));
	}
}
