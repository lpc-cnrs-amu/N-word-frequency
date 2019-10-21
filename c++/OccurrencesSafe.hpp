#include <mutex>
#include <condition_variable>
#include <queue>

class OccurrencesSafe
{
	private:
		unsigned total_match = 0;
		unsigned total_volume = 0;
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
		
		unsigned get_total_match() { return total_match; }
		unsigned get_total_volume() { return total_volume; }
};

