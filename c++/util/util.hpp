#ifndef UTIL_HPP
#define UTIL_HPP

#include <dirent.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <thread>
#include "QueueSafe.hpp"

bool has_suffix(const char* name, std::string& suffix);

void collect_filenames(QueueSafe<std::string>& queue_filenames, std::string& path_to_files, std::string suffix);

void collect_filenames(std::vector<std::string>& filenames, std::string& path_to_files, std::string suffix);

void print_filenames(std::vector<std::string>& filenames);

void print_message(std::string message, const char* cut_filename);

void print_message(std::string message, std::string cut_filename);

FILE* get_file(int thread_id, std::string filename, 
	std::string& path_directory, std::string filename_end, std::string new_filename_end);
	
bool is_number(const std::string& s);

bool valid_min_year(const std::string& year);

bool valid_nb_ngram(const std::string& nb_ngram);

void init_arg(std::ifstream& file_ini, std::string& line, std::string& arg);

void print_message_safe(std::mutex& print_mutex, unsigned thread_id, std::string message, std::string filename);

void print_message_safe(std::mutex& print_mutex, unsigned thread_id, std::string message, const char* m);

bool get_total_occurrences(const char* filename, unsigned long long& total_match,
	unsigned long long& total_volume);

#endif
