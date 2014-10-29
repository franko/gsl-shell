#include "agg_array.h"
#include "utils.h"
#include "strpp.h"

class factor_labels
{
public:
	factor_labels(double delta): m_mark_delta(delta) {}

	double mark(int k) const { return m_mark_index[k] * m_mark_delta; }
	const char *label_text(int k) const { return m_label_text[k]->cstr(); }
	int labels_number() const { return m_label_text.size(); }

	void add_mark(int index, const char *text)
	{
		m_mark_index.add(index);
		if (text)
		{
			str* text_str = new str(text);
			m_label_text.add(text_str);
		}
	}

private:
	double m_mark_delta;
    agg::pod_bvector<short> m_mark_index;
    ptr_list<str> m_label_text;
};
