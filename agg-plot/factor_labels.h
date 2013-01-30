#include "agg_array.h"

class factor_labels
{
public:
	factor_labels(double delta): m_mark_delta(delta) {}

	double mark(int k) { return m_mark_index[k] * m_mark_delta; }
	const char *label(int k) { return m_label_text[k]->cstr(); }
	int labels_number() { return m_label_text.size(); }

private:
	double m_mark_delta;
    agg::pod_bvector<short> m_mark_index;
    ptr_list<str> m_label_text;
};
