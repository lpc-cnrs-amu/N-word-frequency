#ifndef DATA_HPP
#define DATA_HPP

#include <mutex>
#include <condition_variable>
#include <map>

// dans le main.cpp : map<std::string, DataSafe> tags_to_data;

class DataSafe
{
	private:
		unsigned long somme_year = 0;
		unsigned long somme_nb_match = 0;
		unsigned long somme_nb_volume = 0;
		float mean_pondere_match = 0;
		float mean_pondere_volume = 0;
		unsigned year_max = 0;
		unsigned year_min = 3000;
		unsigned nb_match_max = 0;
		unsigned nb_match_min = 100000;
		unsigned nb_volume_max = 0;
		unsigned nb_volume_min = 100000;
		
		std::mutex _mutex;

	public:
		void add_somme_year(unsigned year)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			somme_year += year;
		}
		void add_somme_nb_match(unsigned nb_match)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			somme_nb_match += nb_match;
		}
		void add_somme_nb_volume(unsigned nb_volume)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			somme_nb_volume += nb_volume;
		}
		void add_mean_pondere_match(float mult)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			mean_pondere_match += mult;
		}
		void add_mean_pondere_volume(float mult)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			mean_pondere_volume += mult;
		}
		void try_and_change_year_max(unsigned year)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			if( year > year_max )
				year_max = year;
		}
		void try_and_change_year_min(unsigned year)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			if( year < year_min )
				year_min = year;
		}
		void try_and_change_match_max(unsigned nb_match)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			if( nb_match > nb_match_max )
				nb_match_max = nb_match;
		}
		void try_and_change_match_min(unsigned nb_match)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			if( nb_match < nb_match_min )
				nb_match_min = nb_match;
		}
		void try_and_change_volume_max(unsigned nb_volume)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			if( nb_volume > nb_volume_max )
				nb_volume_max = nb_volume;
		}
		void try_and_change_volume_min(unsigned nb_volume)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			if( nb_volume < nb_volume_min )
				nb_volume_min = nb_volume;
		}
};

#endif
