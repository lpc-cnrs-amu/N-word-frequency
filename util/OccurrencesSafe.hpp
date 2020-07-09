#ifndef OCCURRENCES_SAFE_HPP
#define OCCURRENCES_SAFE_HPP

#include <mutex>

class OccurrencesSafe
{
	private:
		unsigned long long _total_match = 0;
		std::mutex _mutex;
	public:
		void add_match(unsigned long long match)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			_total_match += match;
		}
		
		unsigned long long get_total_match() 
		{
			std::lock_guard<std::mutex> guard(_mutex);
			return _total_match; 
		}
		
		void print_total_match()
		{
			std::lock_guard<std::mutex> guard(_mutex);
			std::cout << "total match = " << _total_match << std::endl;
		}
};

#endif
