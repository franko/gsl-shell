#ifndef CPPUTILS_SHARED_VECTOR_H
#define CPPUTILS_SHARED_VECTOR_H

#include "pthreadpp.h"
#include "agg_array.h"

template <class T>
class shared_vector : public agg::pod_bvector<T>
{
public:
    shared_vector(): agg::pod_bvector<T>() { }

    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

private:
    agg::pod_bvector<T> m_data;
    pthread::mutex m_mutex;
};

#endif
