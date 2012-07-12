#ifndef AGGPLOT_MY_TREE_H
#define AGGPLOT_MY_TREE_H

#include "my_list.h"

namespace tree {

  template <class base_type, class tree_data_type>
  struct node {
    typedef pod_list<node*> list;

    virtual list* tree() { return 0; };
    virtual list* tree(tree_data_type& data) { return 0; };

    virtual base_type* content() = 0;
    virtual void content(const base_type& src) = 0;

    virtual ~node() {};
  };

  template <class base_type, class tree_data_type>
  class leaf : public node<base_type, tree_data_type> {
    base_type m_content;

  public:
    leaf() : m_content() {};

    virtual base_type* content() { return &m_content; };
    virtual void content(const base_type& src) { m_content = src; };
  };

  template <class base_type, class tree_data_type>
  class tree_node : public node<base_type, tree_data_type> {

    typedef node<base_type, tree_data_type> node_type;
    typedef typename node<base_type, tree_data_type>::list node_list;

    node_list *m_head;
    tree_data_type m_data;

  public:
    tree_node() : m_head(0), m_data() {};

    template <class init_type>
    tree_node(const init_type& init_val) : m_head(0), m_data(init_val) {};

    ~tree_node()
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
    virtual node_list* tree(tree_data_type& data)
    {
      data = m_data;
      return m_head;
    };

    virtual base_type* content() { return 0; };
    virtual void content(const base_type& src) { };

    void add(node_type* child)
    {
      node_list *t = new node_list(child);
      m_head = node_list::push_back(m_head, t);
    }
  };

  template <class base_type, class tree_data_type, class f>
  void walk_rec(node<base_type, tree_data_type> *t)
  {
    base_type *c = t->content();
    if (c)
      f::func(*c);

    typename node<base_type, tree_data_type>::list *ls = t->tree();
    if (ls)
      {
        for ( ; ls; ls = ls->next())
          walk_rec<base_type, tree_data_type, f> (ls->content());
      }
  }
}

#endif
