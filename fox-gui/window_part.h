
#include "agg_basics.h"
#include "agg_array.h"

enum split_e { vertical, horizontal, leaf };

struct partition {
    short split; // should be a split_e enum value
    short childs_number;
};

class window_part {
    typedef float num_type;
    typedef agg::rect_base<num_type> rect_type;

public:
    window_part(const char* split);

//    const agg::rect_i& rect(unsigned k) { return m_rect[k]; }

    void split();
    // agg::trans_affine area_matrix(unsigned index, const agg::trans_affine& m);
    agg::trans_affine area_matrix(unsigned index, int canvas_width, int canvas_height);
//    agg::rect_i rect(unsigned index, int canvas_width, int canvas_height);

private:
    void split_rec(const rect_type& r, unsigned& k);
    bool parse_element(const char*& p);

    // The partitions are stored in linear form in m_index.
    // Is a node has a child its childs will follow in order
    // in the vector.
    agg::pod_bvector<partition> m_index;
    agg::pod_bvector<rect_type> m_rect;
};
