#ifndef DATA_SAFE_HPP
#define DATA_SAFE_HPP

#include <vector>

class Data
{
	private:
		unsigned long long somme_year = 0;
		unsigned long long somme_nb_match = 0;
		unsigned long long somme_nb_volume = 0;
		std::vector<float> mean_pondere_match;
		std::vector<float> mean_pondere_volume;
		unsigned year_max = 0;
		unsigned year_min = 3000;
		unsigned nb_match_max = 0;
		unsigned nb_match_min = 100000;
		unsigned nb_volume_max = 0;
		unsigned nb_volume_min = 100000;
		
		std::mutex _mutex

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
		void add_mean_pondere_match(float match)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			mean_pondere_match.push_back(match);
		}
		void add_mean_pondere_volume(float volume)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			mean_pondere_volume.push_back(volume);
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
		
		float calcul_mean_pondere_match()
		{
			std::lock_guard<std::mutex> guard(_mutex);
			float result = 0;
			for(unsigned i=0; i<mean_pondere_match.size(); ++i)
				result += mean_pondere_match[i];
			return result / static_cast<float>(mean_pondere_match.size());
		}
		
		float calcul_mean_pondere_volume()
		{
			std::lock_guard<std::mutex> guard(_mutex);
			float result = 0;
			for(unsigned i=0; i<mean_pondere_volume.size(); ++i)
				result += mean_pondere_volume[i];
			return result / static_cast<float>(mean_pondere_volume.size());
		}
		

		
		float get_freq_match(unsigned long long total_match)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			return somme_nb_match / (total_match*0.1);
		}
		
		float get_freq_volume(unsigned long long total_volume)
		{
			std::lock_guard<std::mutex> guard(_mutex);
			return somme_nb_volume / (total_volume*0.1);
		}
		unsigned long long get_somme_year() { return somme_year; }
		unsigned long long get_somme_nb_match() { return somme_nb_match; }
		unsigned long long get_somme_nb_volume() { return somme_nb_volume; }
		unsigned get_year_max() { return year_max; }
		unsigned get_year_min() { return year_min; }
		unsigned get_nb_match_max() { return nb_match_max; }
		unsigned get_nb_match_min() { return nb_match_min; }
		unsigned get_nb_volume_max() { return nb_volume_max; }
		unsigned get_nb_volume_min() { return nb_volume_min; }
};

#endif
