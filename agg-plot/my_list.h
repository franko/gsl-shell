#ifndef AGGPLOT_LIST_H
#define AGGPLOT_LIST_H

template <class T>
class pod_list {
  T m_content;
  pod_list *m_next;

public:
  pod_list(const T& c, pod_list* next = 0) : m_content(c), m_next(next) { };

        T& content()       { return m_content; };
  const T& content() const { return m_content; };

  static pod_list* push_back(pod_list* head, pod_list* n)
  {
    pod_list* k = head;

    if (! k)
      return n;
	
    while (k->m_next)
      k = k->m_next;
    k->m_next = n;

    return head;
  }

  pod_list* next() { return m_next; };
};

namespace list {

  template <class T>
  int length(pod_list<T> *ls)
  {
    int n = 0;
    for ( ; ls; ls = ls->next())
      n++;
    return n;
  }

  template <class T>
  void free(pod_list<T> *p)
  {
    pod_list<T> *n;
    for (/* */; p; p = n)
    {
      n = p->m_next;
      delete p;
    }
  }
}

#endif
