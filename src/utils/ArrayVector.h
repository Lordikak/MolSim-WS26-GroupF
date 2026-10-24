#pragma once
#include <array>
#include <vector>
#include <cstddef>

template <class T, std::size_t N>
class ArrayVector {
    std::array<std::vector<T>, N> values_;

    public:
    void push_back(const std::array<T, N> &value) {
        for (std::size_t d = 0; d < N; ++d) {
            values_[d].push_back(value[d]);
        }
    }
    void reserve(std::size_t n) {
        for (auto &component : values_) {
            component.reserve(n);
        }
    }
    [[nodiscard]] std::size_t size() const {return values_[0].size();}
    [[nodiscard]] std::array<T, N> get(std::size_t i) const {
        std::array<T, N> result{};
        for (std::size_t d = 0; d < N; ++d) {
            result[d] = values_[d][i];
        }
        return result;
    }
    void set(std::size_t i, const std::array<T, N> &value) {
        for (std::size_t d = 0; d < N; ++d) {
            values_[d][i] = value[d];
        }
    }
    std::vector<T> &component(std::size_t d) {return values_[d];}
    [[nodiscard]] const std::vector<T> &component(std::size_t i) const {return values_[i];}
};