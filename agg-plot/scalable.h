
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

  virtual ~vertex_source() { };
};

struct scalable_object {
  virtual void apply_transform(const agg::trans_affine& m, double as) = 0;
  virtual bool dispose() = 0;

  virtual ~scalable_object() { };
};

struct scalable : public vertex_source, public scalable_object {
  virtual ~scalable() { };
};

/* This class is basically a wrapper around a native AGG vertex_source object.
   The wrapper implements the "scalable" interface. */
template <class T, bool system_managed = false>
class vs_proxy : public scalable {
protected:
  T m_base;

public:
  vs_proxy(): scalable(), m_base() {};

  virtual void rewind(unsigned path_id) { m_base.rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_base.vertex(x, y);  };

  virtual void apply_transform(const agg::trans_affine& m, double as) { };
  virtual bool dispose() { return (system_managed ? false : true); };

  T& self() { return m_base; };
};

/* The same as vs_proxy but with approximation_scale. */
template <class T, bool system_managed = false>
class vs_proxy_approx : public vs_proxy<T, system_managed> {
  typedef vs_proxy<T, system_managed> root_type;
public:
  vs_proxy_approx(): root_type() {};

  virtual void apply_transform(const agg::trans_affine& m, double as)
  { 
    this->m_base.approximation_scale(as); 
  };
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
  typedef conv_type self_type;

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

template<class conv_type>
class scalable_adapter : public vs_adapter<conv_type, scalable> {
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
class scalable_adapter_approx : public vs_adapter<conv_type, scalable> {
  typedef vs_adapter<conv_type, scalable> root_type;

public:
  scalable_adapter_approx(scalable *src) : root_type(src) { };

  template <class init_type>
  scalable_adapter_approx(scalable* src, init_type& val): root_type(src, val) {};

  virtual void apply_transform(const agg::trans_affine& m, double as)
  {
    this->m_output.approximation_scale(as);
    this->m_source->apply_transform(m, as);
  };
};

#endif
