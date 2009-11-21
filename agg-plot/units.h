
/*
  $Id$
*/

#ifndef AGGPLOT_UNITS_H
#define AGGPLOT_UNITS_H

#include <string>
#include <vector>

#include "utils.h"

template<class num_type>
class units {
private:
  int m_major;
  int order;
  num_type dmajor; // equal to (m_major * 10^order)
  int m_inf, m_sup; // expressed in the base of (m_major * 10^order)
  int nb_decimals;

 private:
  void init(num_type min, num_type max, num_type spacefact);

public:
  units(): m_major(1), order(0), dmajor(1), m_inf(0), m_sup(1), nb_decimals(0) {}; 
  units (num_type min, num_type max, num_type spacefact = 5.0)
    { init(min, max, spacefact); };

  int begin() const { return m_inf; };
  int end()   const { return m_sup; };

  void  limits(int &start, int &fin, num_type &step) 
  { start = m_inf; fin = m_sup; step = dmajor; };

  void        mark_label (std::string& label, int mark) const;
  num_type    mark_value (int mark) const { return dmajor * mark; };

  num_type mark_scale(num_type x);
};

template<class num_type>
void units<num_type>::init(num_type yinf, num_type ysup, num_type spacefact)
{
  num_type del;

  if (ysup == yinf)
    ysup = yinf + 1.0;

  del = (ysup - yinf) / spacefact;

  order = (int) floor(log10(del));

  num_type expf = pow(10, order);
  num_type delr = del / expf;

  if (5 <= delr)
    m_major = 5;
  else if (2 <= delr)
    m_major = 2;
  else
    m_major = 1;

  m_inf = (int) floor(yinf / (m_major * expf));
  m_sup = (int) ceil(ysup / (m_major * expf));

  nb_decimals = (order < 0 ? -order : 0);

  dmajor = m_major * expf;
}

template<class num_type>
void units<num_type>::mark_label (std::string& lab, int mark) const
{
  bool minus = (m_inf < 0);
  int asup = (minus ? -m_inf : m_sup);
  char fmt[8];

  if (nb_decimals == 0)
    {
      int space = (int)log10(asup * dmajor) + (minus ? 1 : 0) + 1;
      sprintf (fmt, "%%%id", space);
      string_printf (lab, fmt, (int) (mark * dmajor));
    }
  else
    {
      int dec = (nb_decimals < 10 ? nb_decimals : 9);
      int base = floor(asup * dmajor);
      int space = dec + (base > 0 ? (int)log10(base): 0) + 1 \
	+ (minus ? 1 : 0) + 1;
      sprintf (fmt, "%%%i.%if", space, dec);
      string_printf (lab, fmt, mark * dmajor);
    }
}

template<class num_type>
num_type units<num_type>::mark_scale (num_type x)
{
  num_type xinf = m_inf * dmajor, xsup = m_sup * dmajor;
  return (x - xinf) / (xsup - xinf);
}

#endif
