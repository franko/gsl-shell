#ifndef AGGPLOT_RESOURCE_MANAGER_H
#define AGGPLOT_RESOURCE_MANAGER_H


class no_management {
 public:
  template <class T>
  static void acquire(T* p) {};

  template <class T>
  static void dispose(T* p) {};
};

class lua_management {
public:
  template <class T>
  static void acquire(T* p) { };

  template <class T>
  static void dispose(T* p) 
    { 
      if (p->dispose())
	delete p;
    };
};

#endif
