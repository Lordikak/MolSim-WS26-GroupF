#include "ParticleContainer.h"

#include <algorithm>
#include <cmath>
#include <span>
#include <stdexcept>

#include "utils/VectorUtils.h"

CellGrid::CellGrid(Vec3 min, Vec3 max, const double r_cutoff)
    : domain_min_(std::move(min)), domain_max_(std::move(max)) {
    if (!(r_cutoff > 0.0)) throw std::invalid_argument("r_cutoff must be > 0");
    const Vec3 size = domain_max_ - domain_min_;
    for (int d = 0; d < 3; d++) {
        if (!(size[d] > 0.0))
            throw std::invalid_argument("domain size must be > 0 in every dimension");
        const double ratio = size[d] / r_cutoff;
        const Index n = std::max<Index>(1, static_cast<Index>(ratio));
        cell_size_[d] = size[d] / n;
        inv_cell_size_[d] = 1 / cell_size_[d];
        count_[d] = n + 2;
    }
    cell_count_ = count_[0] * count_[1] * count_[2];
}

Index CellGrid::CellOf(const Vec3 &x) const {
    CellCoords c{};
    for (std::size_t d = 0; d < 3; ++d) {
        const double rel = std::floor((x[d] - domain_min_[d]) * inv_cell_size_[d]) + 1.0;
        c[d] = static_cast<Index>(std::clamp(rel, 0.0, static_cast<double>(count_[d] - 1)));
    }
    return LinearIndex(c);
}
CellCoords CellGrid::VecIndex(const Index c) const {
    const Index x = c % count_[0];
    const Index yz = c / count_[0];
    return {x, yz % count_[1], yz / count_[1]};
}
bool CellGrid::IsHalo(const Index c) const { return IsHalo(VecIndex(c)); }
bool CellGrid::IsHalo(const CellCoords &c) const {
    for (std::size_t d = 0; d < 3; ++d) {
        if (c[d] == 0 || c[d] == count_[d] - 1) return true;
    }
    return false;
}

constexpr Index kDropped = std::numeric_limits<Index>::max();

void ParticleContainer::Rebuild() {
    cell_particle_count_.assign(grid_.CellCount(), 0);
    keys_.resize(size());
    for (auto p : *this) {
        const Index index = grid_.CellOf(p.x());
        if (grid_.IsHalo(index)) {
            keys_[p.index()] = kDropped;
            continue;
        }
        keys_[p.index()] = index;
        cell_particle_count_[index] += 1;
    }

    Index prefix_sum = 0;

    for (std::size_t i = 0; i < cell_particle_count_.size(); i++) {
        const Index current_count = cell_particle_count_[i];
        cell_particle_count_[i] = prefix_sum;
        prefix_sum += current_count;
    }

    // Copy sum
    cell_start_ = cell_particle_count_;
    cell_start_.push_back(prefix_sum);
    perm_.resize(prefix_sum);

    // Old count
    const Index n = static_cast<Index>(size());
    // Build backward map
    for (Index i = 0; i < n; ++i) {
        if (keys_[i] == kDropped) continue;
        perm_[cell_particle_count_[keys_[i]]++] = i;
    }

    x_.Permutate(perm_, scratch_double_);
    v_.Permutate(perm_, scratch_double_);
    f_.Permutate(perm_, scratch_double_);
    old_f_.Permutate(perm_, scratch_double_);
    vector_utils::Permutate(m_, perm_, scratch_double_);
    vector_utils::Permutate(type_, perm_, scratch_int_);
}

void ParticleContainer::add(const Vec3 &x, const Vec3 &v, const double m, const int type) {
    x_.push_back(x);
    v_.push_back(v);
    f_.push_back(Vec3{});
    old_f_.push_back(Vec3{});
    m_.push_back(m);
    type_.push_back(type);
}

void ParticleContainer::reserve(const std::size_t n) {
    x_.reserve(n);
    v_.reserve(n);
    f_.reserve(n);
    old_f_.reserve(n);
    m_.reserve(n);
    type_.reserve(n);
}
