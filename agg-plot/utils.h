#ifndef AGGPLOT_UTILS_H
#define AGGPLOT_UTILS_H

#include "agg_trans_affine.h"
#include "agg_array.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


template <typename T>
T min(T a, T b)
{
    return (a < b) ? a : b;
};

template <typename T>
T max(T a, T b)
{
    return (a > b) ? a : b;
};

template <class T>
class ptr_list {
public:
    ~ptr_list() {
        clear();
    }

    void add(T* p) {
        m_list.add(p);
    }

    T* operator [] (unsigned k) const {
        return m_list[k];
    }

    T* at(unsigned k) const {
        return m_list[k];
    }

    unsigned size() const {
        return m_list.size();
    }

    void clear()
    {
        unsigned n = m_list.size();
        for (unsigned k = 0; k < n; k++)
            delete m_list[k];
        m_list.clear();
    }

private:
    agg::pod_bvector<T*> m_list;
};

inline bool is_unit_matrix(const agg::trans_affine& m, double eps = 1.0e-5)
{
    return (fabs(m.sx - 1.0) < eps && fabs(m.shx) < eps &&
            fabs(m.shy) < eps && fabs(m.sy - 1.0) < eps);
}

extern void   trans_affine_compose  (agg::trans_affine& a,
                                     const agg::trans_affine& b);

extern agg::trans_affine affine_matrix(const agg::rect_i& r);

extern agg::trans_affine identity_matrix;

#endif
