
#include <algorithm>

extern "C" void sort_libstd(double *farray, unsigned int elements);

void sort_libstd(double *farray, unsigned int elements)
{
    std::sort(farray, farray + elements);
}

template <typename T, typename Compare>
void insertion_sort(T* array, unsigned nb)
{
    for (T* p = array + 1; p < array + nb; p++) {
        T current_value = *p;
        T* hp = p;
        for ( ; hp > array; hp--) {
            if (!Compare::less(current_value, hp[-1]))
                break;
            hp[0] = hp[-1];
        }
        hp[0] = current_value;
    }
}

template <typename T, typename Compare>
void partition(T* array, unsigned is, unsigned ie, unsigned pivot_index)
{
    T pivot_value = array[pivot_index];
    
}

