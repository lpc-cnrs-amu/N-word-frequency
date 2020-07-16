#include "Noeud.hpp"
using namespace std;

Noeud::Noeud(wchar_t c): m_val(c), m_end_word(false), m_child(NULL),
	m_sibling(NULL)
{

}

Noeud::Noeud(Noeud* copy, unsigned num_POS)
{
	/*this->m_POS.push_back( copy->m_POS[num_POS] );
	this->m_lemme.push_back( copy->m_lemme[num_POS] );
	this->m_freq_films_2.push_back( copy->m_freq_films_2[num_POS] );
	this->m_nb_lettres.push_back( copy->m_nb_lettres[num_POS] );
	this->m_nb_syll.push_back( copy->m_nb_syll[num_POS] );*/
	this->m_child = NULL;
	this->m_sibling = NULL;
}

Noeud::~Noeud()
{/*
    if(m_child != NULL)
        delete m_child;
    if(m_sibling != NULL)
        delete m_sibling;*/
}

bool Noeud::add_child(Noeud* child)
{
    if(child == NULL)
    {
		cout << "noeud null !!" << endl;
        return false;
    }
    if(m_child == NULL)
    {
        m_child = child;
        //cout << "ajout noeud enfant: " << (char)child->get_val() << " au noeud: "<< (char)this->get_val() << endl;
    }
    else
    {
		/*if(m_child != NULL)
			cout << "pas nul child" << endl;*/
		return m_child->add_sibling(child);
    }

    return true;
}

bool Noeud::add_sibling(Noeud* sibling)
{
    if(sibling == NULL)
    {
		cout << "frere null !"<<endl;
        return false;
    }

    sibling->m_sibling = m_sibling;
    m_sibling = sibling;
    //cout << "ajout noeud frere: " << (char)sibling->get_val() << " au noeud: "<< (char)this->get_val() << endl;
    return true;

}


void Noeud::set_val(wchar_t c)
{
    m_val = c;
}
		
int Noeud::get_val() const
{
    return m_val;
}

Noeud* Noeud::get_child() const
{
    return m_child;
}

Noeud* Noeud::get_sibling() const
{
    return m_sibling;
}

void Noeud::set_end_word()
{
	m_end_word = true;
}

bool Noeud::is_end_word()
{
	return m_end_word;
}
        
/*
ostream& operator<<(ostream &flux, Noeud const& noeud)
{
    flux << noeud.get_val() << "(" << left << setw(6) << noeud.get_code_POS() << ")";
    return flux;
}
*//*
Noeud* Noeud::search(const std::wstring& mot, const string& POS)
{
    int i=0;
    int code_POS = pos_to_int(POS);
    //cout << "POS = " << code_POS << endl;
    Noeud* courant = this;
    while(courant != NULL)
    {
        if(courant->get_val() == mot[i])
        {
			//cout << "Val noeud= " << (char)courant->get_val() << ", lettre= " << (char)mot[i] << endl;
            i++;
            // Find the word
            if(mot[i] == END_OF_WORD)
            {
				// check if POS ok
				for(unsigned num_POS=0; num_POS < courant->m_POS.size(); ++num_POS)
				{
					//cout << courant->m_POS[num_POS] << " ";
					if( courant->m_POS[num_POS] == code_POS )
					{
						//cout << endl;
						return courant;
					}
				}
				return NULL;
            }
            else
            {
                courant = courant->get_child();
            }
        }
        else
        {
            courant = courant->get_sibling();
        }
    }
    return NULL;
}*/

bool Noeud::search_and_get(std::wstring& mot)
{
    unsigned i=0;
    Noeud* courant = this;
    while(courant != NULL)
    {
        if(courant->get_val() == mot[i])
        {
            i++;
            
            // Find the word
            if(mot[i] == END_OF_WORD)
				return courant->m_end_word;
            else
                courant = courant->get_child();
        }
        else
            courant = courant->get_sibling();
    }
    return false;
}


