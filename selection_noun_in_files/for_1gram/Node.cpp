#include "Node.hpp"
using namespace std;

Node::Node(wchar_t c): m_val(c), m_end_word(false), m_child(NULL),
	m_sibling(NULL)
{

}

Node::~Node()
{
	
}

bool Node::add_child(Node* child)
{
    if(child == NULL)
    {
		cout << "WARNING - add null child"<<endl;
        return false;
    }
    if(m_child == NULL)
    {
        m_child = child;
        return true;
    }
    return m_child->add_sibling(child);
}

bool Node::add_sibling(Node* sibling)
{
    if(sibling == NULL)
    {
		cout << "WARNING - add null sibling"<<endl;
        return false;
    }
    sibling->m_sibling = m_sibling;
    m_sibling = sibling;
    return true;
}

void Node::set_val(wchar_t c){m_val = c;}
int Node::get_val() const{return m_val;}
Node* Node::get_child() const{return m_child;}
Node* Node::get_sibling() const{return m_sibling;}
void Node::set_end_word(){m_end_word = true;}
bool Node::is_end_word(){return m_end_word;}

bool Node::search_and_get(wstring& word)
{
    unsigned i=0;
    Node* courant = this;
    while(courant != NULL)
    {
        if(courant->get_val() == word[i])
        {
            i++;
            // Find the word
            if(word[i] == END_OF_WORD)
				return courant->m_end_word;
            else
                courant = courant->get_child();
        }
        else
            courant = courant->get_sibling();
    }
    return false;
}
