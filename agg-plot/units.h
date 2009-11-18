
/*
  $Id$
*/

#ifndef AGGPLOT_UNITS_H
#define AGGPLOT_UNITS_H

#include <string>
#include <vector>

template<class num_type>
class units {
private:
  int major;
  int order;
  num_type dmajor; // equal to (major * 10^order)
  int inf, sup; // expressed in the base of (major * 10^order)
  int nb_decimals;

 private:
  void init(num_type min, num_type max, num_type spacefact);

public:
  units (std::vector<num_type> &x, num_type spacefact = 5.0);
  units (num_type min, num_type max, num_type spacefact = 5.0)
    { init(min, max, spacefact); };

  void  limits(int &start, int &fin) { start = inf; fin = sup; };
  void  limits(int &start, int &fin, num_type &step) 
  { start = inf; fin = sup; step = dmajor; };

  int      decimals () const { return nb_decimals; };
  void     mark_label (std::string &label, int mark) const;
  num_type mark_value (int mark) const { return dmajor * mark; };
  num_type mark_scale(num_type x);
  
  static void get_limits (std::vector<num_type> &x, num_type &inf, num_type &sup);

};


template<class num_type>
void units<num_type>::get_limits (std::vector<num_type> &x, num_type &inf, num_type &sup)
{
  typename vector<num_type>::iterator j = x.begin();

  if (j == x.end())
    return;
    
  inf = x[j];
  sup = x[j];
  j++;
    
  for ( ; j != x.end(); j++)
    {
      num_type v = x[j];
      if (inf > v)
	inf = v;
      if (sup < v)
	sup = v;
    }
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
    major = 5;
  else if (2 <= delr)
    major = 2;
  else
    major = 1;

  inf = (int) floor(yinf / (major * expf));
  sup = (int) ceil(ysup / (major * expf));

  nb_decimals = (order < 0 ? -order : 0);

  dmajor = major * expf;
}

template<class num_type>
units<num_type>::units (std::vector<num_type> &x, num_type spacefact)
{
  num_type yinf, ysup;
  units::get_limits (x, yinf, ysup);
  init (yinf, ysup, spacefact);
}

template<class num_type>
void units<num_type>::mark_label (std::string &lab, int mark) const
{
  bool minus = (inf < 0);
  int asup = (minus ? -inf : sup);
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
  int inf, sup;
  num_type ef;
  limits(inf, sup, ef);
  return (x - inf * ef) / ((sup - inf) * ef);
}

#endif
