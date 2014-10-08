#ifndef MY_CONV_SIMPLE_MARKER_H
#define MY_CONV_SIMPLE_MARKER_H

#include "agg_basics.h"
#include "agg_trans_affine.h"

using namespace agg;

namespace my {
//---------------------------------------------------------conv_simple_marker
template<class MarkerLocator, class MarkerShapes>
class conv_simple_marker
{
public:
    conv_simple_marker(MarkerLocator& ml, MarkerShapes& ms);

    void rewind(unsigned path_id);
    unsigned vertex(double* x, double* y);

private:
    conv_simple_marker(const conv_simple_marker<MarkerLocator, MarkerShapes>&);
    const conv_simple_marker<MarkerLocator, MarkerShapes>&
    operator = (const conv_simple_marker<MarkerLocator, MarkerShapes>&);

    enum status_e
    {
        initial,
        markers,
        polygon,
        stop
    };

    MarkerLocator* m_marker_locator;
    MarkerShapes*  m_marker_shapes;
    status_e       m_status;
    double         m_x1, m_y1;
};


//------------------------------------------------------------------------
template<class MarkerLocator, class MarkerShapes>
conv_simple_marker<MarkerLocator, MarkerShapes>::conv_simple_marker(MarkerLocator& ml, MarkerShapes& ms) :
    m_marker_locator(&ml),
    m_marker_shapes(&ms),
    m_status(initial)
{
}


//------------------------------------------------------------------------
template<class MarkerLocator, class MarkerShapes>
void conv_simple_marker<MarkerLocator, MarkerShapes>::rewind(unsigned)
{
    m_status = initial;
}


//------------------------------------------------------------------------
template<class MarkerLocator, class MarkerShapes>
unsigned conv_simple_marker<MarkerLocator, MarkerShapes>::vertex(double* x, double* y)
{
    unsigned cmd = path_cmd_move_to;

    while(!is_stop(cmd))
    {
        switch(m_status)
        {
        case initial:
            m_marker_locator->rewind(0);
            m_status = markers;

        case markers:
            if(is_stop(m_marker_locator->vertex(&m_x1, &m_y1)))
            {
                cmd = path_cmd_stop;
                m_status = stop;
                break;
            }
            m_marker_shapes->rewind(0);
            m_status = polygon;

        case polygon:
            cmd = m_marker_shapes->vertex(x, y);
            if(is_stop(cmd))
            {
                cmd = path_cmd_move_to;
                m_status = markers;
                break;
            }
            *x += m_x1;
            *y += m_y1;
            return cmd;

        case stop:
            cmd = path_cmd_stop;
            break;
        }
    }
    return cmd;
}
}

#endif
