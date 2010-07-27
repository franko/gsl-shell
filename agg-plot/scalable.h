#ifndef AGGPLOT_SCALABLE_H
#define AGGPLOT_SCALABLE_H

class vertex_source {
public:
  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;

  virtual ~vertex_source() { };
};

class scalable_object {
public:
  virtual void approximation_scale(double as) = 0;
  virtual bool dispose() = 0;

  virtual ~scalable_object() { };
};

class scalable : public vertex_source, public scalable_object {
public:
  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;

  virtual void approximation_scale(double as) = 0;
  virtual bool dispose() = 0;

  virtual ~scalable() { };
};

/* this class does work does permit to perform an AGG transformation
   like conv_stroke, conv_dash or any other transform. This adapter
   is mean to preserve the scalable or the window_drawable interface. */
template<class conv_type, class base_type>
class vs_adapter : public base_type {
protected:
  conv_type m_output;
  base_type* m_source;

public:
  vs_adapter(base_type* src): m_output(*src), m_source(src) 
  {
  };

  template <class init_type>
  vs_adapter(base_type* src, init_type& val):
    m_output(*src, val), m_source(src)
  {};

  virtual void rewind(unsigned path_id) 
  { 
    m_output.rewind(path_id); 
  };

  virtual unsigned vertex(double* x, double* y) 
  { 
    return m_output.vertex(x, y); 
  };

  virtual bool dispose()
  {
    if (this->m_source->dispose())
      delete this->m_source;
    return true;
  };

  conv_type& self() { return m_output; };
};

#endif
