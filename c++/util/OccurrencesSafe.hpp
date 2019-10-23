#ifndef OCCURRENCES_SAFE_HPP
#define OCCURRENCES_SAFE_HPP

#include <mutex>
#include <condition_variable>
#include <queue>

class OccurrencesSafe
{
	private:
		unsigned long long total_match = 0;
		unsigned long long total_volume = 0;
		std::mutex _mutex;
		std::condition_variable _condition_variable;
	public:
		void add_match(unsigned match)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			total_match += match;
		}
		
		void add_volume(unsigned volume)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			total_volume += volume;
		}
		
		unsigned long long get_total_match() { return total_match; }
		unsigned long long get_total_volume() { return total_volume; }
		
		void print_total_match()
		{
			std::lock_guard<std::mutex> guard(_mutex);
			std::cout << "total match = " << total_match << std::endl;
		}
		void print_total_volume()
		{
			std::lock_guard<std::mutex> guard(_mutex);
			std::cout << "total volume = " << total_volume << std::endl;
		}
};

#endif
