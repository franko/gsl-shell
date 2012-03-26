#ifndef CPP_LIST_H
#define CPP_LIST_H

template <class T>
class list {
  T m_content;
  list* m_next;

public:
  list(const T& c, list* next = 0) : m_content(c), m_next(next) { };
  list(      T& c, list* next = 0) : m_content(c), m_next(next) { };

        T& content()       { return m_content; };
  const T& content() const { return m_content; };

  static list* push_back(list* head, list* n)
  {
    list* k = head;

    if (! k)
      return n;

    while (k->m_next)
      k = k->m_next;
    k->m_next = n;

    return head;
  }

  static int length(list* ls)
  {
    int n = 0;
    for ( ; ls; ls = ls->next())
      n++;
    return n;
  }

  static void free(list* p)
  {
    list* n;
    for (/* */; p; p = n)
    {
      n = p->m_next;
      delete p;
    }
  }

  static list* pop(list* p)
  {
    list* tail = p->next();
    delete p;
    return tail;
  }

  list* next() { return m_next; };
};

#endif
