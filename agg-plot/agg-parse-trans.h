#ifndef AGG_PARSE_TRANS_H
#define AGG_PARSE_TRANS_H

extern "C" {
#include "lua.h"
}

#include <exception>

#include "scalable.h"
#include "drawable.h"
#include "agg_color_rgba.h"

class agg_spec_error : public std::exception {
public:
  enum err_e {
    invalid_tag = 0,
    invalid_spec,
    missing_parameter,
    invalid_object,
    generic_error
  };
  
  agg_spec_error(enum err_e err) : m_code(err) {};
  agg_spec_error() : m_code(generic_error) {};

  virtual const char* what() const throw()
  {
    return m_msg[(int) m_code];
  }

private:
  err_e m_code;

  static const char *m_msg[];
};

extern drawable * parse_graph_args (lua_State *L, agg::rgba8& color);

#endif
