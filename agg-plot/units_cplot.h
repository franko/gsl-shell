
#include "cplot.h"
#include "units.h"

class units_cplot : public cplot {
public:
  virtual void draw(canvas &canvas) 
  {
    update();
    draw_axis(canvas);
    draw_lines(canvas);
  };

private:
  void draw_axis(canvas& can);

  virtual bool update()
  {
    bool updated = bbox_update();

    if (! updated)
      return false;

    m_ux = units<double>(m_x1, m_x2);
    m_uy = units<double>(m_y1, m_y2);

    int ixi, ixs;
    double xi, xs, xd;
    m_ux.limits(ixi, ixs, xd);
    xi = ixi * xd;
    xs = ixs * xd;

    int iyi, iys;
    double yi, ys, yd;
    m_uy.limits(iyi, iys, yd);
    yi = iyi * yd;
    ys = iys * yd;

    double fx = 1/(xs - xi), fy = 1/(ys - yi);
    m_trans = agg::trans_affine(fx, 0.0, 0.0, fy, -xi/fx, -yi/fy);

    return true;
  }

  units<double> m_ux;
  units<double> m_uy;
};
