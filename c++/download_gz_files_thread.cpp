#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <regex>
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

#define LINE_SIZE 10000

mutex print_mutex;
mutex read_mutex;

bool error_curl(unsigned thread_id, const char* message, const char* arg, CURL *curl, 
	FILE* fp, const char* output_filename)
{
	std::lock_guard<std::mutex> guard(print_mutex);
	if(arg != NULL)
		fprintf(stderr, message, arg);
	else
		fprintf(stderr, message);
		
	fprintf(stderr, "Didn't download %s\n", output_filename);
	curl_easy_cleanup(curl);
	fclose(fp);
	return false;	
}

void print_safe(unsigned thread_id, const char* message, const char* arg)
{
	std::lock_guard<std::mutex> guard(print_mutex);
	if( arg != NULL )
		fprintf(stderr, message, arg);
	else
		fprintf(stderr, message);
}

/* download html page (url) in output_filename */
bool download_file(unsigned thread_id, const char* output_filename, const char* url)
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
			print_safe(thread_id, "Impossible to open %s\n", output_filename);
			return false;
		}
		print_safe(thread_id, "Downloading %s...\n", url);
		
		code = curl_easy_setopt(curl, CURLOPT_URL, url);
		if(code != CURLE_OK) 
			return error_curl(thread_id, "Failed to set URL %s\n", url, curl, 
				fp, output_filename);
		
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
		if(code != CURLE_OK) 
			return error_curl(thread_id, "Failed to set CURLOPT_WRITEFUNCTION\n", 
				NULL, curl, fp, output_filename);
		
		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if(code != CURLE_OK) 
			return error_curl(thread_id, "Failed to set output file %s\n", 
				output_filename, curl, fp, output_filename);
		
		code = curl_easy_perform(curl);
		if(code != CURLE_OK) 
			return error_curl(thread_id, "Failed to perform curl\n", NULL, curl, 
				fp, output_filename);
		
		curl_easy_cleanup(curl);
		fclose(fp);
	}  
	else
		return error_curl(thread_id, "Failed to create CURL connection\n", 
			NULL, curl, fp, output_filename);

	return true;
}

bool read_one_line(FILE* fp, char* buffer)
{
	std::lock_guard<std::mutex> guard(read_mutex);
	if ( fgets(buffer, sizeof(buffer), fp) == NULL )
		return false;
	return true;
}

bool end_of_file(FILE* fp)
{
	std::lock_guard<std::mutex> guard(read_mutex);
	return feof(fp);
}

bool search_urls(unsigned thread_id, FILE* fp, 
	string output_folder, string lang, string nb_ngrams)
{	
    regex pattern_url("http://storage\\.googleapis\\.com/books/ngrams/books/googlebooks-"+
		lang+"-all-"+nb_ngrams+"gram-20120701-.*\\.gz");

    regex pattern_filename("googlebooks-"+lang+"-all-"+nb_ngrams+"gram-20120701-.*\\.gz");
		
	smatch match_url;
	smatch match_filename;
	
	char buffer[LINE_SIZE];
	string line;
	string output_filename;
	string url;
	
	while( !end_of_file(fp) )
	{
		line = buffer;
		
		if( !read_one_line(fp, buffer) )
			break;
		
		// find a correct url
		if( regex_search(line, match_url, pattern_url) )
		{
			url = match_url[0];
			if( regex_search(url, match_filename, pattern_filename) )
			{
				output_filename = match_filename[0];
				output_filename = output_folder + output_filename;
				if( !download_file(thread_id, output_filename.c_str(), url.c_str()) )
					continue;
				print_safe(thread_id, "Finish %s\n", url.c_str());
			}
			else
				fprintf(stderr, "Didn't find the output filename in match %s\n", url.c_str());
		}
	}
	
}

int main(void)
{
	const char* html_filename = "downloaded_dataset.html";
	if( !download_file(1, html_filename, 
		"http://storage.googleapis.com/books/ngrams/books/datasetsv2.html") )
		return -1;
	
	FILE *fp = fopen(html_filename, "r");
	if( fp == NULL )
	{
		fprintf(stderr, "Impossible to open %s\n", html_filename);
		return -1;
	}
	
	
	unsigned nb_cores = std::thread::hardware_concurrency();
	vector<thread> threads;
	
	for(unsigned i=0; i<nb_cores; ++i)
		threads.emplace_back( [&]{search_urls( i+1, fp, "", "fre", "4"); } );
		
	for(auto& t: threads)
		t.join();
	threads.clear();
	
    return 0;
}
