
#include "agg_basics.h"
#include "agg_array.h"

enum split_e { vertical, horizontal, leaf };

struct partition {
    short split; // should be a split_e enum value
    short childs_number;
};

class window_part {
public:
    window_part(const char* split);

    const agg::rect_i& rect(unsigned k) { return m_rect[k]; }
    
    void split(const agg::rect_i& r, unsigned& k);

private:
    bool parse_element(const char*& p);

    // The partitions are stored in linear form in m_index.
    // Is a node has a child its childs will follow in order
    // in the vector.
    agg::pod_bvector<partition> m_index;
    agg::pod_bvector<agg::rect_i> m_rect;
};
