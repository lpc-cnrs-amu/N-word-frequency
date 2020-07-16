#ifndef TOKENISEUR_HPP
#define TOKENISEUR_HPP

#include <iostream>
#include <fstream>
#include <string>
#include "Noeud.hpp"

std::wstring str_to_wstr(std::string& s);

class Tokeniseur
{
	private:
		Noeud* m_tree;
		std::string m_chemin; //seulement utile pour l'integrite
		bool construct_tree(std::ifstream& f);
		bool add_first_word(std::wstring& mot);
		bool add_word(std::wstring& mot);
		
	public:
		Tokeniseur();
		~Tokeniseur();
		bool load(char* chemin);
		bool search(std::wstring& w_word);
		Noeud* get_root();
		void delete_tree(Noeud* node);
		/*
		int search(const std::wstring& mot, const std::string& POS) const;
		bool search(std::vector<std::string> words, std::vector<std::string> POS, 
			FILE* output, std::string line_ngram);*/
};
#endif // TOKENISEUR_HPP
