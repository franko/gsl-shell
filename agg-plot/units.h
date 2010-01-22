#ifndef AGGPLOT_UNITS_H
#define AGGPLOT_UNITS_H

#include "utils.h"

template<class T>
class units {
private:
  int m_major;
  int order;
  T dmajor; // equal to (m_major * 10^order)
  int m_inf, m_sup; // expressed in the base of (m_major * 10^order)
  int nb_decimals;

  void init(T min, T max, T spacefact);

public:
  units(): m_major(1), order(0), dmajor(1), m_inf(0), m_sup(1), nb_decimals(0) {}; 
  units (T min, T max, T spacefact = 5.0)
    { init(min, max, spacefact); };

  int begin() const { return m_inf; };
  int end()   const { return m_sup; };

  void limits(int &start, int &fin, T &step) 
  { 
    start = m_inf; 
    fin = m_sup; 
    step = dmajor; 
  };

  void mark_label (char *label, unsigned size, int mark) const;
  T    mark_value (int mark) const { return dmajor * mark; };
  T    mark_scale(T x);
};

template<class T>
void units<T>::init(T yinf, T ysup, T spacefact)
{
  T del;

  if (ysup == yinf)
    ysup = yinf + 1.0;

  del = (ysup - yinf) / spacefact;

  order = (int) floor(log10(del));

  T expf = pow(10, order);
  T delr = del / expf;

  if (5 <= delr)
    m_major = 5;
  else if (2 <= delr)
    m_major = 2;
  else
    m_major = 1;

  m_inf = (int) floor(yinf / (m_major * expf) + 1e-5);
  m_sup = (int) ceil (ysup / (m_major * expf) - 1e-5);

  nb_decimals = (order < 0 ? -order : 0);

  dmajor = m_major * expf;
}

template<class T>
void units<T>::mark_label (char *lab, unsigned size, int mark) const
{
  bool minus = (m_inf < 0);
  int asup = (minus ? -m_inf : m_sup);
  char fmt[16];

  if (size < 16)
    return;

  if (nb_decimals == 0)
    {
      snprintf (lab, size, "%d", int(mark * dmajor));
      lab[size-1] = '\0';
    }
  else
    {
      int dec = (nb_decimals < 10 ? nb_decimals : 9);
      int base = (int) floor(asup * dmajor);
      int space = dec + (base > 0 ? (int)log10(base): 0) + 1 \
	+ (minus ? 1 : 0) + 1;
      snprintf (fmt, 16, "%%%i.%if", space, dec);
      fmt[15] = '\0';
      snprintf (lab, size, fmt, mark * dmajor);
      lab[size-1] = '\0';
    }
}

template<class T>
T units<T>::mark_scale (T x)
{
  T xinf = m_inf * dmajor, xsup = m_sup * dmajor;
  return (x - xinf) / (xsup - xinf);
}

#endif
