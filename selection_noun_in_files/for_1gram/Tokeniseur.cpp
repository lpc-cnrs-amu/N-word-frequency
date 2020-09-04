#include "Tokeniseur.hpp"
#include <vector>

using namespace std;

Tokeniseur::Tokeniseur(): m_tree(NULL), m_filename(" ")
{

}

Tokeniseur::~Tokeniseur()
{
 
}

void Tokeniseur::delete_tree(Node* node)
{
	if (node == NULL) 
		return;
    delete_tree(node->get_child());  
    delete_tree(node->get_sibling());  
    delete node;
}

Node* Tokeniseur::get_root(){return m_tree;}

bool Tokeniseur::load(char* filename)
{
    ifstream file(filename);
    if(!file)
    {
        cerr << "Error|Ligne 32|Tokeniseur.cpp|Error: Impossible to open the file " << filename << endl;
        return false;
    }
    m_filename = filename;
    return construct_tree(file);
}

wstring str_to_wstr(string& s) 
{
    string curLocale = setlocale(LC_ALL, ""); 
    const char* source = s.c_str();
    size_t dest_size = mbstowcs(NULL, source, 0) + 1;
    wchar_t* dest = new wchar_t[dest_size];
    wmemset(dest, 0, dest_size);
    mbstowcs(dest, source, dest_size);
    wstring result = dest;
    delete []dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

bool Tokeniseur::construct_tree(ifstream& f)
{
    string token;
    wstring word;
    while( getline(f, token) )
    {
		word = str_to_wstr(token);
        if( !add_word(word) )
        {
            cerr << "Error|Ligne 62|Tokeniseur.cpp|Adding word " << token << " return false" << endl;
            return false;
        }
    }
    return true;
}

bool Tokeniseur::add_first_word(wstring& mot)
{
	m_tree = new Node(mot[0]);
	if(m_tree == NULL)
	{
		cerr << "Error|Ligne 74|Tokeniseur.cpp|Error when creating a node" << endl;
		return false;
	}
    Node* current = NULL;
    Node* parent = m_tree;
	for(unsigned i=1; mot[i] != END_OF_WORD; ++i)
	{
		current = new Node(mot[i]);
		if(current == NULL)
		{
			cerr << "Error|Ligne 84|Tokeniseur.cpp|Error when creating a node" << endl;
			return false;
		}
		parent->add_child(current);
		parent = current;
	}
	parent->set_end_word();
	return true;
}

bool Tokeniseur::add_word(wstring& mot)
{
    Node* parent = NULL;
    Node* current = NULL;
    unsigned i = 0;
    bool sibling_root = false;
    bool tmp = true;
    
    if(m_tree == NULL)
		return add_first_word(mot);
		
    current = m_tree;
    parent = current;
    while(current != NULL)
    {
        if(mot[i] == END_OF_WORD)
        {
            parent->set_end_word();
            return true;
        }
        if(current->get_val() == mot[i])
        {
            parent = current;
            current = current->get_child();
            i++;
            tmp = false;
        }
        else
        {
            current = current->get_sibling();
            if(tmp)
				sibling_root = true;
        }
    }
    // The word is already in the tree
    if(mot[i] == END_OF_WORD)
    {
		parent->set_end_word();		
		return true;
	}
    current = new Node(mot[i]);
    if(current == NULL)
    {
        cerr << "Error|Ligne 137|Tokeniseur.cpp|Error when creating a node" << endl;
        return false;
    }
    if(parent == m_tree && sibling_root) // adding sibling to the root
        m_tree->add_sibling(current);
    else
    {
        parent->add_child(current);
        if(current->get_child() != NULL)
			cout << "pas normal" << endl;
    }
    i++;
    while(mot[i] != END_OF_WORD)
    {
        parent = current;
        current = new Node(mot[i]);
        if(current == NULL)
        {
            cerr << "Error|Ligne 155|Tokeniseur.cpp|Error when creating a node" << endl;
            return false;
        }
        parent->add_child(current);
        i++;
    }
	current->set_end_word();
    return true;
}

bool Tokeniseur::search(wstring& w_word)
{
	return m_tree->search_and_get(w_word);
}
