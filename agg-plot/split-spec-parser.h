#ifndef AGGPLOT_SPLIT_SPEC_PARSER_H
#define AGGPLOT_SPLIT_SPEC_PARSER_H

#include "agg_trans_affine.h"
#include "my_list.h"
#include "utils.h"

typedef agg::trans_affine matrix;

enum direction_e { along_x, along_y };

namespace split {

  template <class base_type>
  struct node {
    typedef pod_list<node*> list;

    virtual void transform(matrix& m) = 0;

    virtual list* tree() { return 0; };
    virtual base_type* content() { return 0; };
    virtual void content(const base_type& src) { };

    virtual matrix* get_matrix() { return 0; };

    virtual ~node() {};

    static void init(node* tree);
  };

  template <class base_type>
  void node<base_type>::init(node<base_type>* tree)
  {
    matrix m;
    tree->transform(m);
  }

  template <class base_type>
  class node_leaf : public node<base_type> {
    base_type m_content;
    matrix m_matrix;

  public:
    node_leaf() : m_content(), m_matrix() {};

    virtual base_type* content() { return &m_content; };
    virtual void content(const base_type& src) { m_content = src; };

    virtual matrix* get_matrix() { return &m_matrix; };

    virtual void transform(matrix& m) { trans_affine_compose (m_matrix, m); };
  };

  template <class base_type>
  class node_tree : public node<base_type> {

    typedef typename node<base_type>::list node_list;

    node_list *m_head;
    direction_e m_dir;
    double m_frac;
  
  public:
    node_tree(direction_e dir) : m_head(0), m_dir(dir) {};
  
    ~node_tree()
    {
      node_list *n;
      for (node_list *p = m_head; p; p = n)
	{
	  n = p->next();
	  delete p->content();
	  delete p;
	}
    };

    virtual node_list* tree() { return m_head; };

    virtual void transform(matrix& m) 
    {
      node_list *t;
      matrix lm;
      double* p = (m_dir == along_x ? &lm.tx : &lm.ty);
      double* s = (m_dir == along_x ? &lm.sx : &lm.sy);

      *s = m_frac;

      for (t = m_head; t != NULL; t = t->next(), *p += m_frac)
	{
	  node<base_type>* nd = t->content();
	  matrix sm(lm);
	  trans_affine_compose (sm, m);
	  nd->transform(sm);
	}
    };

    void add(node<base_type>* child) 
    { 
      node_list *t = new node_list(child);
      m_head = node_list::push_back(m_head, t);
    }

    void fraction(int n) { m_frac = 1 / (double)n; };
  };


  /* ------------------------------------------ string_lexer */
  class string_lexer {
    const char *m_content;
    const char *m_ptr;

  public:
    string_lexer(const char *str) : m_content(str), m_ptr(m_content) {};

    char next()
    { 
      char c = *m_ptr;
      if (c != '\0')
	m_ptr++;
      return c;
    };

    void push() { if (m_ptr > m_content) m_ptr--; };

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

  template <class base_type, class lexer>
  extern node<base_type>* parse (lexer& lex);

  template <class base_type, class lexer>
  node<base_type>* subtree (lexer& lex, direction_e dir)
  {
    node_tree<base_type> * ls = new node_tree<base_type>(dir);

    int c;
    for (c = 0; ; c++)
      {
	node<base_type>* child = parse<base_type, lexer>(lex);
	if (! child)
	  break;
	ls->add(child);
      }
  
    ls->fraction(c);

    return ls;
  }

  template <class base_type, class lexer>
  node<base_type>* parse (lexer& lex)
  {
    char t = lex.next();

    switch (t)
      {
      case '.':
	return new node_leaf<base_type>();
      case 'h':
	return subtree<base_type, lexer>(lex, along_x);
      case 'v':
	return subtree<base_type, lexer>(lex, along_y);
      case '(':
	{
	  node<base_type> *nd = parse<base_type, lexer>(lex);
	  if (! lex.checknext(')'))
	    return NULL;
	  return nd;
	}
      case ')':
	lex.push();
	return NULL;
      default:
	return NULL;
      }

    return NULL;
  }
}

#endif
