#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <regex>
#include <iostream>

using namespace std;

#define LINE_SIZE 10000

bool error_curl(const char* message, const char* arg, CURL *curl, 
	FILE* fp, const char* output_filename)
{
	if(arg != NULL)
		fprintf(stderr, message, arg);
	else
		fprintf(stderr, message);
		
	fprintf(stderr, "Didn't download %s\n", output_filename);
	curl_easy_cleanup(curl);
	fclose(fp);
	return false;	
}

/* download html page (url) in output_filename */
bool download_file(const char* output_filename, const char* url)
{
	CURL *curl;
	CURLcode code;
	FILE *fp;
	
	curl = curl_easy_init();                                                                                                                                                                                                                                                           
	if (curl)
	{   
		fp = fopen(output_filename, "wb");
		if( fp == NULL )
		{
			fprintf(stderr, "Impossible to open %s\n", output_filename);
			return false;
		}
		fprintf(stdout, "Downloading %s...\n", url);
		
		code = curl_easy_setopt(curl, CURLOPT_URL, url);
		if(code != CURLE_OK) 
			return error_curl("Failed to set URL %s\n", url, curl, 
				fp, output_filename);
		
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
		if(code != CURLE_OK) 
			return error_curl("Failed to set CURLOPT_WRITEFUNCTION\n", 
				NULL, curl, fp, output_filename);
		
		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if(code != CURLE_OK) 
			return error_curl("Failed to set output file %s\n", 
				output_filename, curl, fp, output_filename);
		
		code = curl_easy_perform(curl);
		if(code != CURLE_OK) 
			return error_curl("Failed to perform curl\n", NULL, curl, 
				fp, output_filename);
		
		curl_easy_cleanup(curl);
		fclose(fp);
	}  
	else
		return error_curl("Failed to create CURL connection\n", 
			NULL, curl, fp, output_filename);

	return true;
}

bool search_urls(const char* html_filename, string output_folder, string lang, 
	string nb_ngrams)
{
	FILE *fp = fopen(html_filename, "r");
	if( fp == NULL )
	{
		fprintf(stderr, "Impossible to open %s\n", html_filename);
		return false;
	}
	
    regex pattern_url("http://storage\\.googleapis\\.com/books/ngrams/books/googlebooks-"+
		lang+"-all-"+nb_ngrams+"gram-20120701-.*\\.gz");

    regex pattern_filename("googlebooks-"+lang+"-all-"+nb_ngrams+"gram-20120701-.*\\.gz");
		
	smatch match_url;
	smatch match_filename;
	
	char buffer[LINE_SIZE];
	string line;
	string output_filename;
	string url;
	
	while( fgets(buffer, sizeof(buffer), fp) )
	{
		line = buffer;
		
		// find a correct url
		if( regex_search(line, match_url, pattern_url) )
		{
			url = match_url[0];
			if( regex_search(url, match_filename, pattern_filename) )
			{
				output_filename = match_filename[0];
				output_filename = output_folder + output_filename;
				if( !download_file(output_filename.c_str(), url.c_str()) )
					continue;	
			}
			else
				fprintf(stderr, "Didn't find the output filename in match %s\n", url.c_str());
		}
	}
	fclose(fp);
	return true;
}

int main(void)
{
	const char* html_filename = "downloaded_dataset.html";
	if( !download_file(html_filename, 
		"http://storage.googleapis.com/books/ngrams/books/datasetsv2.html") )
		return -1;
	
	if( !search_urls(html_filename, "/mnt/j/ENG_2GRAMS_thread/", "eng", "2") )
		return -1;
	
    return 0;
}
