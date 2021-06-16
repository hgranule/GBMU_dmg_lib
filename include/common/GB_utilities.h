#ifndef COMMON_GB_UTILITIES_H_
#define COMMON_GB_UTILITIES_H_

#include <vector>

template<typename T, typename Comp>
inline typename std::vector<T>::iterator insert_sorting(std::vector<T>& vector, const T& new_elem, Comp comparator) {
    return vector.insert(std::upper_bound(vector.begin(), vector.end(), new_elem, comparator), new_elem);
}

#endif  // COMMON_GB_UTILITIES_H_
