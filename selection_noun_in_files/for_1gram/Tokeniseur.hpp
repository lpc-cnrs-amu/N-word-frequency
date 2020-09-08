#ifndef TOKENISEUR_HPP
#define TOKENISEUR_HPP

#include <iostream>
#include <fstream>
#include <string>
#include "Node.hpp"

std::wstring str_to_wstr(std::string& s);

class Tokeniseur
{
	private:
		Node* m_tree;
		std::string m_filename;
		bool construct_tree(std::ifstream& f);
		bool add_first_word(std::wstring& mot);
		bool add_word(std::wstring& mot);
		
	public:
		Tokeniseur();
		~Tokeniseur();
		bool load(char* filename);
		bool search(std::wstring& w_word);
		Node* get_root();
		void delete_tree(Node* node);
};
#endif
