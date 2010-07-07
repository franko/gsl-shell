#ifndef AGGPLOT_RESOURCE_MANAGER_H
#define AGGPLOT_RESOURCE_MANAGER_H


class no_management {
 public:
  template <class T>
  static void acquire(T* p) {};

  template <class T>
  static void dispose(T* p) {};
};

/*
class ref_manager {
public:
  template <class T>
  static void acquire(T* p) { p->ref(); };

  template <class T>
  static void dispose(T* p) 
  { 
    unsigned rc = p->unref(); 
    if (rc == 0)
      delete p;
  };
};
*/

#endif
