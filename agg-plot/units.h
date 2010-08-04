
/* units.h
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
