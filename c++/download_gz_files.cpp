#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <regex>

using namespace std;

int main(void)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    string url_begining = "http://storage.googleapis.com/books/ngrams/books/googlebooks-"
    string output_folder = "/";
	string lang = "fre";
	string nb_ngrams = "4";
    
    string url = "fre-all-4gram-20120701-zz.gz";
    string outfilename = "downloaded_googlebooks-fre-all-4gram-20120701-zz.gz";
    bool ok=true;
    
    
    regex pattern("http://storage.googleapis.com/books/ngrams/books/googlebooks-"+
		lang+"-all-"+nb_ngrams+"gram-20120701-*.gz"); 
    
    while(ok)
    {
		url = url_begining + lang + "-all-" + nb_ngrams + "gram-20120701-" + + ".gz";
		output_filename = output_folder + lang + "-all-" + nb_ngrams + "gram-20120701-" + + ".gz";
		
		curl = curl_easy_init();                                                                                                                                                                                                                                                           
		if (curl)
		{   
			fp = fopen(outfilename.c_str(),"wb");
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			fclose(fp);
		}   
	}
    return 0;
}
