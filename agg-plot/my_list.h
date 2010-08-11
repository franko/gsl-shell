#ifndef AGGPLOT_LIST_H
#define AGGPLOT_LIST_H

#ifndef NULL
#define NULL ((void*)0)
#endif

template <class T>
class pod_list {
  T m_content;
  pod_list *m_next;

public:
  pod_list(const T& c, pod_list* next = NULL) : m_content(c), m_next(next) { };

  void free_subtree()
  {
    if (m_next)
      {
	m_next->free_subtree();
	delete m_next;
      }
  };

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

  static void free(pod_list *list);
};

template <class T>
void pod_list<T>::free(pod_list<T> *list)
{
  if (list)
    {
      list->free_subtree();
      delete list;
    }
}

#endif
