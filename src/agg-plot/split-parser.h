#ifndef AGGPLOT_SPLIT_PARSER_H
#define AGGPLOT_SPLIT_PARSER_H

#include "tree.h"

enum direction_e { along_x, along_y };

template <class base_type>
class split {

    typedef tree::node<base_type, direction_e> node_type;

public:
    // ----------------------------------------------------string_lexer
    class lexer {
        const char *m_content;
        const char *m_ptr;

    public:
        lexer(const char *str) : m_content(str), m_ptr(m_content) {};

        char next()
        {
            char c = *m_ptr;
            if (c != '\0')
                m_ptr++;
            return c;
        };

        void push()
        {
            if (m_ptr > m_content)
                m_ptr--;
        };

        bool checknext(char reqchar)
        {
            char c = *m_ptr;
            if (c == reqchar)
            {
                m_ptr++;
                return true;
            }
            return false;
        };
    };

    // ------------------------------------------------parser: exprlist
    static node_type* exprlist (lexer& lex, direction_e dir)
    {
        typedef tree::tree_node<base_type, direction_e> tree_type;

        tree_type *t = new tree_type(dir);

        for (int c = 0; ; c++)
        {
            node_type* child = parse(lex);
            if (! child)
                break;
            t->add(child);
        }

        return t;
    }

// ------------------------------------------------parser: parse
    static node_type* parse (lexer& lex)
    {
        char t = lex.next();

        switch (t)
        {
        case '.':
            return new tree::leaf<base_type, direction_e>();
        case 'h':
            return exprlist(lex, along_x);
        case 'v':
            return exprlist(lex, along_y);
        case '(':
        {
            node_type *nd = parse(lex);
            if (! lex.checknext(')'))
                return 0;
            return nd;
        }
        case ')':
            lex.push();
            return 0;
        default:
            return 0;
        }

        return 0;
    }
};

#endif
