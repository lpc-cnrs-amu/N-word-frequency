#ifndef DATA_HPP
#define DATA_HPP

class Data
{
	private:
		unsigned long long somme_year = 0;
		unsigned long long somme_nb_match = 0;
		unsigned long long somme_nb_volume = 0;
		unsigned long long mean_pondere_match = 0;
		unsigned long long mean_pondere_volume = 0;
		unsigned year_max = 0;
		unsigned year_min = 3000;
		unsigned long long nb_match_max = 0;
		unsigned long long nb_match_min = ULLONG_MAX;
		unsigned long long nb_volume_max = 0;
		unsigned long long nb_volume_min = ULLONG_MAX;

	public:
		void add_somme_year(unsigned long long year)
		{
			somme_year += year;
		}
		void incr_somme_year()
		{
			++ somme_year;
		}
		void add_somme_nb_match(unsigned long long nb_match)
		{
			somme_nb_match += nb_match;
		}
		void add_somme_nb_volume(unsigned long long nb_volume)
		{
			somme_nb_volume += nb_volume;
		}
		void add_mean_pondere_match(unsigned long long match, unsigned long long year)
		{
			mean_pondere_match = mean_pondere_match + match * year;
		}
		void add_mean_pondere_volume(unsigned long long volume, unsigned long long year)
		{
			mean_pondere_volume = mean_pondere_volume + volume * year;
		}
		void add_mean_pondere_match(unsigned long long mpm)
		{
			mean_pondere_match = mean_pondere_match + mpm;
		}
		void add_mean_pondere_volume(unsigned long long mpv)
		{
			mean_pondere_volume = mean_pondere_volume + mpv;
		}
		void try_and_change_year_max(unsigned year)
		{
			if( year > year_max )
				year_max = year;
		}
		void try_and_change_year_min(unsigned year)
		{
			if( year < year_min )
				year_min = year;
		}
		void try_and_change_match_max(unsigned long long nb_match)
		{
			if( nb_match > nb_match_max )
				nb_match_max = nb_match;
		}
		void try_and_change_match_min(unsigned long long nb_match)
		{
			if( nb_match < nb_match_min )
				nb_match_min = nb_match;
		}
		void try_and_change_volume_max(unsigned long long nb_volume)
		{
			if( nb_volume > nb_volume_max )
				nb_volume_max = nb_volume;
		}
		void try_and_change_volume_min(unsigned long long nb_volume)
		{
			if( nb_volume < nb_volume_min )
				nb_volume_min = nb_volume;
		}
		

		long double get_freq_match(unsigned long long total_match)
		{
			return somme_nb_match / static_cast<double>(total_match);
		}
		
		long double get_freq_volume(unsigned long long total_volume)
		{
			return somme_nb_volume / static_cast<double>(total_volume);
		}
	
		unsigned long long get_somme_year() { return somme_year; }
		unsigned long long get_somme_nb_match() { return somme_nb_match; }
		unsigned long long get_somme_nb_volume() { return somme_nb_volume; }
		unsigned get_year_max() { return year_max; }
		unsigned get_year_min() { return year_min; }
		unsigned long long get_nb_match_max() { return nb_match_max; }
		unsigned long long get_nb_match_min() { return nb_match_min; }
		unsigned long long get_nb_volume_max() { return nb_volume_max; }
		unsigned long long get_nb_volume_min() { return nb_volume_min; }
		unsigned long long get_mean_pondere_match() 
		{ 
			return mean_pondere_match; 
		}
		unsigned long long get_mean_pondere_volume() 
		{ 
			return mean_pondere_volume; 
		}
		
		double calcul_mean_pondere_match() 
		{ 
			return mean_pondere_match / static_cast<double>(somme_nb_match); 
		}
		double calcul_mean_pondere_volume() 
		{ 
			return mean_pondere_volume / static_cast<double>(somme_nb_volume); 
		}
};

#endif
