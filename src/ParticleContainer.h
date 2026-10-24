#pragma once
#include <vector>

#include "utils/Vec3.h"

template <class Container>
class BasicParticleRef {
    Container *container_;
    std::size_t index_;

   public:
    BasicParticleRef(Container *container, const std::size_t index)
        : container_(container), index_(index) {}

    Vec3 x() const { return container_->x(index_); }
    Vec3 v() const { return container_->v(index_); }
    Vec3 f() const { return container_->f(index_); }
    Vec3 old_f() const { return container_->old_f(index_); }
    double m() const { return container_->m(index_); }
    int type() const { return container_->type(index_); }
    std::size_t index() const { return index_; }

    void SetX(const Vec3 &x) { container_->SetX(index_, x); }
    void SetV(const Vec3 &v) { container_->SetV(index_, v); }
    void SetF(const Vec3 &f) { container_->SetF(index_, f); }
    void SetOldF(const Vec3 &old_f) { container_->SetOldF(index_, old_f); }
    void SetM(const double m) { container_->SetM(index_, m); }
    void SetType(const int type) { container_->SetType(index_, type); }
};

template <class Container>
class BasicParticleIterator {
    std::reference_wrapper<Container> container_;
    std::size_t index_;

   public:
    BasicParticleIterator(Container &container, const std::size_t index)
        : container_(container), index_(index) {}

    BasicParticleRef<Container> operator*() {
        return BasicParticleRef<Container>(&container_.get(), index_);
    }

    BasicParticleIterator &operator++() {
        ++index_;
        return *this;
    }

    bool operator==(const BasicParticleIterator &other) const { return index_ == other.index_; }
};

class ParticleContainer {
    Vec3Vector x_;
    Vec3Vector v_;
    Vec3Vector f_;
    Vec3Vector old_f_;
    std::vector<double> m_;
    std::vector<int> type_;

   public:
    BasicParticleIterator<ParticleContainer> begin() { return {*this, 0}; }
    BasicParticleIterator<ParticleContainer> end() { return {*this, size()}; }
    [[nodiscard]] BasicParticleIterator<const ParticleContainer> begin() const {
        return {*this, 0};
    }
    [[nodiscard]] BasicParticleIterator<const ParticleContainer> end() const {
        return {*this, size()};
    }

    void add(const Vec3 &x, const Vec3 &v, double m, int type = 0);
    [[nodiscard]] std::size_t size() const { return m_.size(); }
    void reserve(std::size_t n);

    [[nodiscard]] Vec3 x(const std::size_t i) const { return x_.get(i); }
    [[nodiscard]] Vec3 v(const std::size_t i) const { return v_.get(i); }
    [[nodiscard]] Vec3 f(const std::size_t i) const { return f_.get(i); }
    [[nodiscard]] Vec3 old_f(const std::size_t i) const { return old_f_.get(i); }
    [[nodiscard]] double m(const std::size_t i) const { return m_[i]; }
    [[nodiscard]] int type(const std::size_t i) const { return type_[i]; }

    void SetX(const std::size_t i, const Vec3 &x) { x_.set(i, x); }
    void SetV(const std::size_t i, const Vec3 &v) { v_.set(i, v); }
    void SetF(const std::size_t i, const Vec3 &f) { f_.set(i, f); }
    void SetOldF(const std::size_t i, const Vec3 &old_f) { old_f_.set(i, old_f); }
    void SetM(const std::size_t i, const double m) { m_[i] = m; }
    void SetType(const std::size_t i, const int type) { type_[i] = type; }
};

using ParticleRef = BasicParticleRef<ParticleContainer>;
using ConstParticleRef = BasicParticleRef<const ParticleContainer>;
