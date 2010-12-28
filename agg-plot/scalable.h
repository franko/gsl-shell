
/* scalable.h
 * 
 * Copyright (C) 2009, 2010 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AGGPLOT_SCALABLE_H
#define AGGPLOT_SCALABLE_H

#include "agg_trans_affine.h"

struct vertex_source {
  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;
  virtual void apply_transform(const agg::trans_affine& m, double as) = 0;
  virtual bool affine_compose(agg::trans_affine& m) { return false; };
  virtual ~vertex_source() { };
};

typedef vertex_source scalable;

template <class T, bool approx>
class vs_proxy : public vertex_source {
protected:
  T m_base;

public:
  vs_proxy(): vertex_source(), m_base() {};

  virtual void rewind(unsigned path_id) { m_base.rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_base.vertex(x, y);  };
  virtual void apply_transform(const agg::trans_affine& m, double as) { };

  T& self() { return m_base; };
};

/* The same as vs_proxy but with approximation_scale. */
template <class T>
class vs_proxy<T, true> : public vertex_source {
protected:
  T m_base;

public:
  vs_proxy(): vertex_source(), m_base() {};

  virtual void rewind(unsigned path_id) { m_base.rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_base.vertex(x, y);  };
  virtual void apply_transform(const agg::trans_affine& m, double as)
  { 
    this->m_base.approximation_scale(as); 
  };

  T& self() { return m_base; };
};

class boxed_scalable : public scalable {
  vertex_source *m_object;

 public:
  boxed_scalable(vertex_source *p) : scalable(), m_object(p) {};

  ~boxed_scalable() { };

  virtual void rewind(unsigned path_id) { m_object->rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_object->vertex(x, y); };
  virtual void apply_transform(const agg::trans_affine& m, double as) { m_object->apply_transform(m, as); };

 private:
  boxed_scalable();
};

/* this class does work does permit to perform an AGG transformation
   like conv_stroke, conv_dash or any other transform. This adapter
   is meant to preserve the scalable or the window_drawable interface. */
template<class conv_type, class base_type>
class vs_adapter : public base_type {
protected:
  conv_type m_output;
  base_type* m_source;

public:
  vs_adapter(base_type* src): m_output(*src), m_source(src) { };
  
  ~vs_adapter() { delete this->m_source; };

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

  conv_type& self() { return m_output; };
};

template<class conv_type, bool approx>
class scalable_adapter : public vs_adapter<conv_type, scalable> 
{
  typedef vs_adapter<conv_type, scalable> root_type;

public:
  scalable_adapter(scalable *src) : root_type(src) { };

  template <class init_type>
  scalable_adapter(scalable* src, init_type& val): root_type(src, val) {};

  virtual void apply_transform(const agg::trans_affine& m, double as)
  {
    this->m_source->apply_transform(m, as);
  };
};

template<class conv_type>
class scalable_adapter<conv_type, true> : public vs_adapter<conv_type, scalable> 
{
  typedef vs_adapter<conv_type, scalable> root_type;

public:
  scalable_adapter(scalable *src) : root_type(src) { };

  template <class init_type>
  scalable_adapter(scalable* src, init_type& val): root_type(src, val) {};

  virtual void apply_transform(const agg::trans_affine& m, double as)
  {
    this->m_output.approximation_scale(as);
    this->m_source->apply_transform(m, as);
  };
};

#endif
