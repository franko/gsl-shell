#ifndef AGGPLOT_RESOURCE_MANAGER_H
#define AGGPLOT_RESOURCE_MANAGER_H


class lua_management {
public:
  template <class T>
  static void acquire(T* p) { };

  template <class T>
  static void dispose(T* p) { delete p; };
};

#endif
