#pragma once
#include <span>
#include <vector>

#include "Index.h"

namespace vector_utils {
template <class T>
void Permutate(std::vector<T> &field, std::span<const Index> perm, std::vector<T> &scratch) {
    scratch.resize(perm.size());
    // Gather using perm
    for (std::size_t i = 0; i < perm.size(); ++i) {
        scratch[i] = field[perm[i]];
    }
    field.swap(scratch);
}
}  // namespace vector_utils
