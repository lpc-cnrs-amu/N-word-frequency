#ifndef NOEUD_HPP
#define NOEUD_HPP

#include <iostream>
#include <iomanip>
#include <vector>

#define END_OF_WORD 0x0000

class Noeud;

class Noeud
{
    private:
        wchar_t m_val; // une lettre
        bool m_end_word = false;
        
        Noeud* m_child;
        Noeud* m_sibling;

    public:
        ~Noeud();
        Noeud(wchar_t c);
        Noeud(Noeud* copy, unsigned num_POS);
        bool add_child(Noeud* child);
        bool add_sibling(Noeud* sibling);

        void set_val(wchar_t c);
        int get_val() const;
        
        Noeud* get_child() const;
        Noeud* get_sibling() const;
        
        bool is_end_word();
        void set_end_word();
        
        bool search_and_get(std::wstring& mot);
        
        /*Noeud* search(const std::wstring& mot, const std::string& POS);
        Noeud* search_and_get(const std::wstring& mot, const std::string& POS);
        */
        /*
        void print_info()
        {
			std::cout << "POS: ";
			for(unsigned i=0; i<m_POS.size(); ++i)
				std::cout << m_POS[i] << " ";
			std::cout << "\nphon: ";
			for(unsigned i=0; i<m_phon.size(); ++i)
				std::cout << m_phon[i] << " ";
			std::cout << "\nlemme: ";
			for(unsigned i=0; i<m_lemme.size(); ++i)
				std::cout << m_lemme[i] << " ";
			std::cout << std::endl;
		}*/
};
#endif // NOEUD_HPP
