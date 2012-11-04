
#include "agg_basics.h"
#include "agg_array.h"
#include "agg_trans_affine.h"

enum split_e { vertical, horizontal, leaf };

struct partition {
    short split; // should be a split_e enum value
    short childs_number;
};

class window_part {
    typedef float num_type;
    typedef agg::rect_base<num_type> rect_type;

public:
    window_part() {};

    int parse(const char* split);

    void split();
    int get_slot_index(const char* str);
    unsigned get_slot_number() const;

    agg::rect_i rect(unsigned index, int canvas_width, int canvas_height) const;

private:
    int skip_node(int index, int& leaf_count);
    int goto_child_index(int pindex, int child_index, int& leaf_count);

    void split_rec(const rect_type& r, unsigned& k);
    bool parse_element(const char*& p);

    // The partitions are stored in linear form in m_index.
    // Is a node has a child its childs will follow in order
    // in the vector.
    agg::pod_bvector<partition> m_index;
    agg::pod_bvector<rect_type> m_rect;
};
