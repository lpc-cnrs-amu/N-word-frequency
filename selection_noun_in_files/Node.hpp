#ifndef NODE_HPP
#define NODE_HPP

#include <iostream>
#include <iomanip>
#include <vector>

#define END_OF_WORD 0x0000

class Node;

class Node
{
    private:
        wchar_t m_val;
        bool m_end_word = false;
        Node* m_child;
        Node* m_sibling;

    public:
        ~Node();
        Node(wchar_t c);
        bool add_child(Node* child);
        bool add_sibling(Node* sibling);

        void set_val(wchar_t c);
        int get_val() const;
        
        Node* get_child() const;
        Node* get_sibling() const;
        
        bool is_end_word();
        void set_end_word();
        
        bool search_and_get(std::wstring& word);
};
#endif
