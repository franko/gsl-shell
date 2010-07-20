#ifndef AGGPLOT_UNITS_H
#define AGGPLOT_UNITS_H

class units {
private:
  int m_major;
  int order;
  double dmajor; // equal to (m_major * 10^order)
  int m_inf, m_sup; // expressed in the base of (m_major * 10^order)
  int nb_decimals;

  void init(double min, double max, double spacefact);

public:
  units(): m_major(1), order(0), dmajor(1), m_inf(0), m_sup(1), nb_decimals(0) {}; 
  units (double min, double max, double spacefact = 5.0)
  { init(min, max, spacefact); };

  int begin() const { return m_inf; };
  int end()   const { return m_sup; };

  void limits(int &start, int &fin, double &step) 
  { 
    start = m_inf; 
    fin = m_sup; 
    step = dmajor; 
  };

  void   mark_label (char *label, unsigned size, int mark) const;
  double mark_value (int mark) const { return dmajor * mark; };
  double mark_scale(double x);
};

#endif
