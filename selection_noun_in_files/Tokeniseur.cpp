#include "Tokeniseur.hpp"
#include <vector>

using namespace std;

Tokeniseur::Tokeniseur(): m_tree(NULL),m_chemin(" ")
{

}

Tokeniseur::~Tokeniseur()
{
 
}

void Tokeniseur::delete_tree(Noeud* node)
{
	if (node == NULL) 
		return;  
  
    /* first delete both subtrees */
    delete_tree(node->get_child());  
    delete_tree(node->get_sibling());  
      
    /* then delete the node */
    //cout << "Deleting node: " << (char)node->get_val() << endl;  
    delete node;
}

Noeud* Tokeniseur::get_root()
{
	return m_tree;
}

bool Tokeniseur::load(char *chemin)
{
    ifstream fichier(chemin);
    if(!fichier)
    {
        cerr << "Erreur|Ligne 15 Tokeniseur.cpp|Erreur: Impossible d'ouvrir le fichier " << chemin << endl;
        return false;
    }
    m_chemin = chemin;
    return construct_tree(fichier);
}

wstring str_to_wstr(std::string& s) 
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
    
    // Read a line
    while( getline(f, token) )
    {
		
		word = str_to_wstr(token);
		//cout << "MOT: ["<< token << "]" << endl;
		//cout << str_word << " " << POS << " " << phon << " " << lemme << endl;
        if( !add_word(word) )
        {
            cerr << "Erreur|Ligne 30 Tokeniseur.cpp|L'ajout du mot "<<token<<" return false" << endl;
            return false;
        }
    }
    return true;
}

bool Tokeniseur::add_first_word(wstring& mot)
{
	m_tree = new Noeud(mot[0]);
	if(m_tree == NULL)
	{
		cerr << "Erreur|Ligne 61 Tokeniseur.cpp|Erreur création d'un noeud " << endl;
		return false;
	}
	//cout << "ajout a la racine: "<< (char)m_tree->get_val() << endl;
    Noeud* courant = NULL;
    Noeud* parent = m_tree;
	for(unsigned i=1; mot[i] != END_OF_WORD; ++i)
	{
		courant = new Noeud(mot[i]);
		if(courant == NULL)
		{
			cerr << "Erreur|Ligne 71 Tokeniseur.cpp|Erreur création d'un noeud " << endl;
			return false;
		}
		parent->add_child(courant);
		parent = courant;
	}
	parent->set_end_word();
	return true;
}

bool Tokeniseur::add_word(wstring& mot)
{
    Noeud* parent = NULL;
    Noeud* courant = NULL;
    int i = 0;
    bool brother_racine = false;
    bool tmp = true;
    
    if(m_tree == NULL)
		return add_first_word(mot);
		
    courant = m_tree;
    parent = courant;
    while(courant != NULL)
    {
        if(mot[i] == END_OF_WORD)
        {
			//cout << "fin mot: " << (char)mot[i] << endl;
            parent->set_end_word();
            return true;
        }
        if(courant->get_val() == mot[i])
        {
			/*if( courant->get_child() != NULL )
				cout << "Deplacement noeud enfant: " << (char)courant->get_child()->get_val() << endl;
			else
				cout << "Deplacement noeud enfant null" << endl;*/
            parent = courant;
            courant = courant->get_child();
            i++;
            tmp = false;
        }
        else
        {
			/*if( courant->get_sibling() != NULL )
				cout << "Deplacement noeud frere: " << (char)courant->get_sibling()->get_val() << endl;
			else
				cout << "Deplacement noeud frere null: " << endl;*/
            courant = courant->get_sibling();
            if(tmp)
				brother_racine = true;
        }
    }
    // si le mot est déjà dedans
    if(mot[i] == END_OF_WORD)
    {
		parent->set_end_word();		
		return true;
	}
    
    courant = new Noeud(mot[i]);
    if(courant == NULL)
    {
        cerr << "Erreur|Ligne 74 Tokeniseur.cpp|Erreur création d'un noeud " << endl;
        return false;
    }
    if(parent == m_tree && brother_racine) // cas ou on ajoute un frere a la racine
    {
        m_tree->add_sibling(courant);
        //cout << "ajout noeud frere (a la racine): " << (char)mot[i] << endl;
    }
    else
    {
        parent->add_child(courant);
        
        if(courant->get_child() != NULL)
			cout << "pas normal" << endl;
    }
    i++;
    while(mot[i] != END_OF_WORD)
    {
        parent = courant;
        /*cout << "parent = courant" << endl;
        if(parent == NULL)
			cout << "PARENT NULL" << endl;*/
        courant = new Noeud(mot[i]);
        if(courant == NULL)
        {
            cerr << "Erreur|Ligne 90 Tokeniseur.cpp|Erreur création d'un noeud " << endl;
            return false;
        }
        //cout << "on va ajouter depuis le noeud: " << (char)parent->get_val() << endl;
        parent->add_child(courant);
        i++;
    }
    //cout << "fin mot: " << (char)mot[i] << endl;
	courant->set_end_word();
    return true;
}
/*
int Tokeniseur::search(const std::wstring& mot, const string& POS) const
{
    Noeud* tmp = m_tree->search(mot, POS);
    if(tmp != NULL)
    {
		//tmp->print_info();
		//cout << endl << endl;
		return 1;
	}
    return 0;
}*/

void destroy_vect(vector<Noeud*>& nodes)
{
	for(unsigned i=0; i<nodes.size(); ++i)
	{
		nodes[i]->~Noeud();
		nodes[i] = NULL;
	}
	nodes.clear();
}

bool Tokeniseur::search(wstring& w_word)
{
	return m_tree->search_and_get(w_word);
}


/*
bool Tokeniseur::integriteArbre()
{
    if(m_chemin == " ") // on a pas load un dico valide
    {
        cerr << "Warning|Ligne 115 Tokeniseur.cpp|Pas de dictionnaire valide chargé " << endl;
        return false;
    }
    ifstream fichier(m_chemin.c_str());
    if(fichier == NULL)
    {
        cerr << "Erreur|Ligne 121 Tokeniseur.cpp|Impossible d'ouvrir un fichier" << endl;
        return false;
    }
    string buffer;
    int code;
    while(fichier >> buffer >> code)
    {
        if(search(buffer) != code)
        {
            cerr << "Erreur|Ligne 130 Tokeniseur.cpp|Erreur avec le mot " << buffer << " il vaut "<<search(buffer) <<" au lieu de " <<code<<endl;
            return false;
        }
    }
    return true;
}
*/
/*
bool Tokeniseur::printCode(char* cheminCorpus)
{
    ifstream fichier(cheminCorpus);
    char c;
    bool continuer = true;
    vector<string> mots;
    string buffer;

    if(fichier == NULL)
    {
        cerr << "Erreur|Ligne 142 Tokeniseur.cpp|Impossible d'ouvrir un fichier" << endl;
        return false;
    }
    if(!fichier.get(c))
    {
        cerr << "Warning|Ligne 151 Tokeniseur.cpp|Le fichier est vide" <<endl;
        return false;
    }

    while(continuer)  // remplit un vector de mot
    {
        continuer = true;
        buffer.clear();
        if(isSeparator(c))
        {
            buffer = c;
            mots.push_back(buffer);
            if(!fichier.get(c))
                continuer = false;
        }
        else
        {
            while(continuer && !isSeparator(c))
            {
                buffer+=c;
                if(!fichier.get(c))
                    continuer = false;
            }
            mots.push_back(buffer);
        }
    }  // valide
    Noeud* courant;
    int indiceValide,codeValide;

    for(unsigned int i=0;i<mots.size();i++)
    {
        if(mots[i] == " ")
            continue;
		if(mots[i] == "\n")
		{
			cout << endl;
			continue;
		}
        indiceValide = i;
        courant = m_tree->search(mots[i]);
        codeValide = -1;
        continuer = true;
        do
        {
            if(courant == NULL)
            {
                continuer = false;
                if(codeValide == -1) // on n'a pas trouvé de mot valide
                {
                    cout << "-1 ";
                }
                else // on a trouve un mot valide les tours d'avant
                {
                    cout << codeValide << " ";
                    i = indiceValide;
                }
            }
            else
            {
                if(courant->get_code_POS() != -1) //on trouve un mot valide
                {
                    indiceValide = i;
                    codeValide = courant->get_code_POS();
                }
                courant = courant->get_child();
                if(courant != NULL)
                {
                    i++;
                    if(mots[i] == " ")
                    {
                        courant = courant->search("_");
                    }
                    else
                        courant = courant->search(mots[i]);
                }
            }
        } while(continuer);
    }
    return true;
}

*/ 
