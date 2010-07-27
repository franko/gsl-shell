#ifndef AGGPLOT_DRAWABLES_H
#define AGGPLOT_DRAWABLES_H

#include "scalable.h"

#include "agg_path_storage.h"

namespace draw {

  template <class T, bool system_managed>
  class vs_proxy : public scalable {
    T m_base;

  public:
    vs_proxy(): scalable(), m_base() {};

    virtual void rewind(unsigned path_id) { m_base.rewind(path_id); };
    virtual unsigned vertex(double* x, double* y) { return m_base.vertex(x, y);  };

    virtual void approximation_scale(double as) { };
    virtual bool dispose() { return (system_managed ? false : true); };

    T& get_base() { return m_base; };
  };

  typedef vs_proxy<agg::path_storage, true> path;
}

#endif
