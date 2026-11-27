#pragma once
#include <omp.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <vector>

#include "utils/Index.h"
#include "utils/Vec3.h"

/**
 * @brief Promises the compiler that iterations in the loop are independent from each other
 *
 * Place it directly before a loop, it allows vectorizing loops the compiler cant prove independent
 * iterations for.
 *
 * Also allows reordering of floating point sums inside the loop when using clang.
 */
#if defined(__clang__)
#define INDEPENDENT_ITERATIONS _Pragma("clang loop vectorize(assume_safety)")
// Maybe "clang loop vectorize(assume_safety) vectorize_predicate(enable) interleave_count(1)" (So
// it doesnt have the remainder loop)
#elif defined(__GNUC__)
#define INDEPENDENT_ITERATIONS _Pragma("GCC ivdep")
#else
#define INDEPENDENT_ITERATIONS
#endif

/**
 * @brief Handle to a Single Particle in the Container
 *
 * Stores only the index of the particle because the entries are not consecutive in memory because
 * of SoA layout.
 *
 * Because its only the index and reference to the container its cheap to copy.
 *
 * @tparam Container `ParticleContainer` for read / write, `const ParticleContainer` for read only
 */
template <class Container>
class ParticleRef {
    std::reference_wrapper<Container>
        container_;      ///< Container that stores the data of the particle
    std::size_t index_;  ///< index of the Particle in the Container

   public:
    /**
     * @brief Creates a handle to the particle at index @p index.
     * @param container Container that holds the Particle
     * @param index Index of the Particle in the Container
     */
    ParticleRef(Container &container, const std::size_t index)
        : container_(container), index_(index) {}

    /// @name Read access
    /// @{
    Vec3 x() const { return container_.get().x(index_); }          ///< Position
    Vec3 v() const { return container_.get().v(index_); }          ///< Velocity
    Vec3 f() const { return container_.get().f(index_); }          ///< Force
    Vec3 old_f() const { return container_.get().old_f(index_); }  ///< Force of the previous step
    double m() const { return container_.get().m(index_); }        ///< Mass
    int type() const { return container_.get().type(index_); }     ///< Type of Particle
    std::size_t index() const { return index_; }  ///< Index in the Container that stores it
    /// @}

    /// @name Write access (For non const Container)
    /// @{
    void SetX(const Vec3 &x) { container_.get().SetX(index_, x); }  ///< Sets Position
    void SetV(const Vec3 &v) { container_.get().SetV(index_, v); }  ///< Sets Velocity
    void SetF(const Vec3 &f) { container_.get().SetF(index_, f); }  ///< Sets Current Force
    void SetOldF(const Vec3 &old_f) {
        container_.get().SetOldF(index_, old_f);
    }  ///< Sets old Force
    void SetM(const double m) { container_.get().SetM(index_, m); }           ///< Sets Mass
    void SetType(const int type) { container_.get().SetType(index_, type); }  ///< Set Type tag
    /// @}
};

class ParticleContainer;

template <class K>
concept PairKernel = std::is_invocable_r_v<Vec3, K, ParticleRef<const ParticleContainer>,
                                           ParticleRef<const ParticleContainer>>;

/**
 * @brief Forward iterator over all particles of a container
 *
 * Dereferencing yields a `ParticleRef` by value (particle objects are never created so cant
 * be pointed to)
 *
 * @tparam Container ParticleContainer or `const ParticleContainer` for read only iteration
 */
template <class Container>
class ParticleIterator {
    std::reference_wrapper<Container> container_;  ///< Container iterating over
    std::size_t index_;                            ///< current index

   public:
    /**
     * @brief Creates a forward iterator pointing to Particle at index @p index in container
     * @p container
     *
     * @param container Container to iterate over
     * @param index Index to start from
     */
    ParticleIterator(Container &container, const std::size_t index)
        : container_(container), index_(index) {}

    /// @brief Handle to the current Particle
    /// @return ParticleRef for the current index
    ParticleRef<Container> operator*() const {
        return ParticleRef<Container>(container_.get(), index_);
    }

    /// @brief Increment the iterator
    /// @return this Iterator
    ParticleIterator &operator++() {
        ++index_;
        return *this;
    }

    /// @brief Compares position of the iterators, only useful for iterators over the same container
    /// @param other Iterator to compare with
    /// @return True if the indices are identical
    bool operator==(const ParticleIterator &other) const { return index_ == other.index_; }
};

// TODO: Implement these (Currently everything is outflow) (Ghost particles for Reflecting needs to
// be added)
enum class Face : std::uint8_t { kXMin, kXMax, kYMin, kYMax, kZMin, kZMax };
enum class BoundaryCondition : std::uint8_t { kOutflow, kReflecting };

struct BoundaryConditions {
    std::array<BoundaryCondition, 6> by_face{
        BoundaryCondition::kOutflow,  // kXMin
        BoundaryCondition::kOutflow,  // kXMax
        BoundaryCondition::kOutflow,  // kYMin
        BoundaryCondition::kOutflow,  // kYMax
        BoundaryCondition::kOutflow,  // kZMin
        BoundaryCondition::kOutflow   // kZMax
    };

    [[nodiscard]] BoundaryCondition For(Face face) const {
        return by_face[static_cast<std::size_t>(face)];
    }
};

constexpr auto kF64Inf = std::numeric_limits<double>::infinity();

using CellCoords = std::array<Index, 3>;
using CellOffset = std::array<std::int32_t, 3>;
using CellStrip = std::tuple<CellCoords, CellCoords, Face>;

inline CellCoords operator+(const CellCoords &c, const CellOffset &o) {
    return {static_cast<Index>(static_cast<std::int32_t>(c[0]) + o[0]),
            static_cast<Index>(static_cast<std::int32_t>(c[1]) + o[1]),
            static_cast<Index>(static_cast<std::int32_t>(c[2]) + o[2])};
}

class CellGrid {
    Vec3 domain_min_, domain_max_;
    std::array<Index, 3> count_;  // Includes halo
    Vec3 cell_size_, inv_cell_size_;
    Index cell_count_;

   public:
    CellGrid()
        : domain_min_{},
          domain_max_{},
          count_{3, 3, 3},
          cell_size_{kF64Inf, kF64Inf, kF64Inf},
          inv_cell_size_{},
          cell_count_(27) {}
    CellGrid(Vec3 min, Vec3 max, double r_cutoff);
    [[nodiscard]] CellCoords CellOf(const Vec3 &x) const;
    [[nodiscard]] Index LinearIndex(const CellCoords &c) const {
        return c[0] + count_[0] * (c[1] + count_[1] * c[2]);
    }
    [[nodiscard]] CellCoords VecIndex(Index c) const;
    [[nodiscard]] bool IsHalo(Index c) const;
    [[nodiscard]] bool IsHalo(const CellCoords &c) const;
    [[nodiscard]] Index CellCount() const { return cell_count_; }
    [[nodiscard]] std::array<Index, 3> const &Count() const { return count_; }
    [[nodiscard]] const Vec3 &DomainMin() const { return domain_min_; }
    [[nodiscard]] const Vec3 &DomainMax() const { return domain_max_; }

    /// @brief Returns x-directed boundary strips for reflecting faces only.
    /// @details Strips cover the interior boundary cells and exclude halo cells.
    ///          Y- and Z-facing strips run along X; X-facing strips contain one cell.
    ///          Cells on edges and corners may appear in multiple strips, once for
    ///          each adjoining reflecting face.
    /// @param boundaries Boundary conditions used to select which faces to include.
    /// @return Boundary strips associated with reflecting faces.
    [[nodiscard]] std::vector<CellStrip> EdgeStripsAlongX(
        const BoundaryConditions &boundaries) const {
        std::vector<CellStrip> strips;

        const Index x_min = 1;
        const Index x_max = count_[0] - 2;
        const Index y_min = 1;
        const Index y_max = count_[1] - 2;
        const Index z_min = 1;
        const Index z_max = count_[2] - 2;

        const auto add_if_reflecting = [&strips, &boundaries](const CellCoords &first,
                                                              const CellCoords &last, Face face) {
            if (boundaries.For(face) == BoundaryCondition::kReflecting) {
                strips.emplace_back(first, last, face);
            }
        };

        // Y-facing surfaces: x-directed strips, one for each z coordinate.
        for (Index z = z_min; z <= z_max; ++z) {
            add_if_reflecting(CellCoords{x_min, y_min, z}, CellCoords{x_max, y_min, z},
                              Face::kYMin);
            add_if_reflecting(CellCoords{x_min, y_max, z}, CellCoords{x_max, y_max, z},
                              Face::kYMax);
        }

        // Z-facing surfaces: x-directed strips, one for each y coordinate.
        for (Index y = y_min; y <= y_max; ++y) {
            add_if_reflecting(CellCoords{x_min, y, z_min}, CellCoords{x_max, y, z_min},
                              Face::kZMin);
            add_if_reflecting(CellCoords{x_min, y, z_max}, CellCoords{x_max, y, z_max},
                              Face::kZMax);
        }

        // X-facing surfaces: each cell is a single-cell strip.
        for (Index z = z_min; z <= z_max; ++z) {
            for (Index y = y_min; y <= y_max; ++y) {
                add_if_reflecting(CellCoords{x_min, y, z}, CellCoords{x_min, y, z}, Face::kXMin);
                add_if_reflecting(CellCoords{x_max, y, z}, CellCoords{x_max, y, z}, Face::kXMax);
            }
        }

        return strips;
    }
};

/**
 * @brief Stores Particles in a Structure of Arrays (SoA)
 *
 * Every quantity is kept in its own std::vector so loops read over consecutive memory positions
 * and can therefore be vectorized.
 *
 * Because particles dont exist as objects values are read as copies (`f(i)`) and written with
 * setters (`SetF(i, ...)`).
 *
 * Iterate over with (`for (auto p : particles)` to get a handle for each particle
 */
class ParticleContainer {
    Vec3Vector x_;           ///< positions
    Vec3Vector v_;           ///< velocities
    Vec3Vector f_;           ///< forces
    Vec3Vector old_f_;       ///< old forces
    std::vector<double> m_;  ///< masses
    std::vector<int> type_;  ///< types

    std::vector<Index> cell_start_;
    CellGrid grid_;

    BoundaryConditions boundary_conditions_{{
        BoundaryCondition::kReflecting,  // kXMin
        BoundaryCondition::kReflecting,  // kXMax
        BoundaryCondition::kReflecting,  // kYMin
        BoundaryCondition::kReflecting,  // kYMax
        BoundaryCondition::kReflecting,  // kZMin
        BoundaryCondition::kReflecting   // kZMax
    }};

    std::vector<Index> cell_particle_count_;  // this is only here so it doesnt need to get reallocd
                                              // each force calc.
    std::vector<Index> keys_;                 // Also to keep as scratch vector
    std::vector<Index> perm_;
    std::vector<double> scratch_double_;
    std::vector<int> scratch_int_;

   private:
    template <PairKernel Kernel>
    void PairsWithinAndNext(Kernel kernel, Index start, Index end, Index next_end);
    template <PairKernel Kernel>
    void PairsBetween(Kernel kernel, Index a_start, Index a_end, Index b_start, Index b_end);
    template <PairKernel Kernel>
    void PairsOfCell(Kernel kernel, CellCoords c);

    Vec3 GhostParticlePosition(const Vec3 &position, Face face);
    template <class BoundaryKernel>
    Vec3 GhostForce(BoundaryKernel boundary_kernel, auto p_i, Face face);
    template <class BoundaryKernel>
    void GhostParticleBoundryForce(BoundaryKernel boundary_kernel, Index a_start, Index a_end,
                                   Face face);

    void ApplyGravity(Index index_start, Index index_end, const Vec3 &gravity);

    [[nodiscard]] Index end(const CellCoords cell_index) const {
        return cell_start_[grid_.LinearIndex(cell_index) + 1];
    }
    [[nodiscard]] Index start(const CellCoords cell_index) const {
        return cell_start_[grid_.LinearIndex(cell_index)];
    }

   public:
    ParticleContainer() = default;
    explicit ParticleContainer(CellGrid grid) : grid_(std::move(grid)) {}

    /// @name Iteration
    /// @{

    /// @return Iterator from the first particle
    ParticleIterator<ParticleContainer> begin() { return {*this, 0}; }
    /// @return Iterator past the last particle
    ParticleIterator<ParticleContainer> end() { return {*this, size()}; }
    /// @return Read only iterator from the first particle
    [[nodiscard]] ParticleIterator<const ParticleContainer> begin() const { return {*this, 0}; }
    /// @return Read only Iterator past the last particle
    [[nodiscard]] ParticleIterator<const ParticleContainer> end() const { return {*this, size()}; }
    /// @}

    /**
     * @brief Appends a single particle
     * @param x Position
     * @param v Velocity
     * @param m Mass
     * @param type Type, use it for whatever you want (e.g. to separate molecules belonging to
     * different bodies, matters, and so on)
     */
    void add(const Vec3 &x, const Vec3 &v, double m, int type = 0);
    /// @return Particle count
    [[nodiscard]] std::size_t size() const { return m_.size(); }
    /**
     * @brief Reserves Memory for @p n particles to reduce amount of allocations
     *
     * @param n Expected number of particles
     */
    void reserve(std::size_t n);

    /// @name Read access by index (returns copies)
    /// @{
    [[nodiscard]] Vec3 x(const std::size_t i) const {
        return x_.get(i);
    }  ///< Position of particle i
    [[nodiscard]] Vec3 v(const std::size_t i) const {
        return v_.get(i);
    }  ///< Velocity of particle i
    [[nodiscard]] Vec3 f(const std::size_t i) const { return f_.get(i); }  ///< Force of particle i
    [[nodiscard]] Vec3 old_f(const std::size_t i) const {
        return old_f_.get(i);
    }  ///< Old Force of particle i
    [[nodiscard]] double m(const std::size_t i) const { return m_[i]; }     ///< Mass of particle i
    [[nodiscard]] int type(const std::size_t i) const { return type_[i]; }  ///< Type of particle i
    /// @}

    /// @name Write access by index
    /// @{
    void SetX(const std::size_t i, const Vec3 &x) { x_.set(i, x); }  ///< Set position of particle i
    void SetV(const std::size_t i, const Vec3 &v) { v_.set(i, v); }  ///< Set velocity of particle i
    void SetF(const std::size_t i, const Vec3 &f) { f_.set(i, f); }  ///< Set force of particle i
    void SetOldF(const std::size_t i, const Vec3 &old_f) {
        old_f_.set(i, old_f);
    }  ///< Set old force of particle i
    void SetM(const std::size_t i, const double m) { m_[i] = m; }  ///< Set mass of particle i
    void SetType(const std::size_t i, const int type) {
        type_[i] = type;
    }  ///< Set type of particle i
    /// @}

    /**
     * @brief Applies the external gravity force to every particle.
     *
     * Splits the particles into contiguous ranges and processes those ranges in
     * parallel. If the container is empty, this function does nothing.
     *
     * @param gravity Gravity acceleration applied to the particles
     */
    void ApplyExternalForces(const Vec3 &gravity) {
        const Index total = size();
        if (total == 0) {
            return;
        }

        const Index num_parts = std::min(total, static_cast<Index>(omp_get_max_threads()));

        std::vector<std::pair<Index, Index>> ranges;
        ranges.reserve(num_parts);

        const Index base = total / num_parts;
        const Index remainder = total % num_parts;

        Index start = 0;
        for (Index part = 0; part < num_parts; ++part) {
            const Index count = base + (part < remainder ? 1 : 0);
            const Index end = start + count;

            ranges.emplace_back(start, end);
            start = end;
        }

#pragma omp parallel for schedule(static)
        for (std::size_t part = 0; part < ranges.size(); ++part) {
            ApplyGravity(ranges[part].first, ranges[part].second, gravity);
        }
    }

    /**
     * @brief Applies repulsive boundary forces to particles near the container edges.
     *
     * Iterates over the edge strips along the x-axis and applies the boundary force
     * for each strip's associated face.
     *
     * @tparam BoundaryKernel Callable that computes the force on a particle from its
     *         ghost image, with a signature compatible with
     *         `Vec3(Vec3 position, Vec3 ghost_position)`.
     *
     * @param boundary_kernel Computes the force acting on a particle near a boundary
     *        from its ghost image.
     */
    template <class BoundaryKernel>
    void ApplyBoundryForces(BoundaryKernel boundary_kernel) {
#pragma omp parallel for schedule(dynamic)
        for (const auto &[first, last, face] : grid_.EdgeStripsAlongX(boundary_conditions_)) {
            GhostParticleBoundryForce(boundary_kernel, start(first), end(last), face);
        }
    }

    /**
     * @brief Applies pairwise forces between all particles
     *
     * Calls the @p kernel once per unordered pair (i, j) with i < j, applying to both using
     * Newtons third law F_ij = -F_ji
     *
     * Forces are accumulated into current values of f, old f is not written. Call this after
     * resetting f to zero (potentially saving to old f)
     *
     * The inner loop is vectorized
     *
     * @warning The kernel must not have side effects and MUST NOT READ f(), that results in
     * potentially undefined behaviour because the values are written during the loop and the
     * compiler gets the independence promise
     *
     * @tparam Kernel Callable with signature
     * `Vec3(ParticleRef<const ParticleContainer> p_i, ParticleRef<const ParticleContainer>
     * p_j)`
     *
     * @param kernel Returns the force acting on @p p_i by @p p_j
     */
    template <PairKernel Kernel>
    void ApplyPairForces(const Kernel kernel) {
#ifndef _OPENMP
        const CellCoords count = grid_.Count();
        for (Index z = 1; z < count[2] - 1; ++z) {
            for (Index y = 1; y < count[1] - 1; ++y) {
                for (Index x = 1; x < count[0] - 1; ++x) {
                    PairsOfCell(kernel, {x, y, z});
                }
            }
        }
#else

        const CellCoords count = grid_.Count();

#pragma omp parallel
        for (Index z_offset = 0; z_offset < 3; ++z_offset) {
            for (Index y_offset = 0; y_offset < 2; ++y_offset) {
                const Index y_job_count = (count[1] - 1 - y_offset) / 2;
                const Index z_job_count = (count[2] - z_offset) / 3;
#pragma omp for schedule(dynamic)
                for (Index i = 0; i < y_job_count * z_job_count; ++i) {
                    Index y = (i % y_job_count) * 2 + 1 + y_offset;
                    Index z = (i / y_job_count) * 3 + 1 + z_offset;

                    for (Index x = 1; x < count[0] - 1; ++x) {
                        PairsOfCell(kernel, {x, y, z});
                    }
                }
            }
        }

#endif
    }

    void Rebuild();

    /**
     * @brief Calculate the total energy across the system. Method used for testing.
     * Calls the @p kernel once per unordered pair (i, j) with i < j
     *
     * Kinetic energy is calculated for each particle, while Potential energy is calculated only
     * once per unordered pair
     *
     * @param kernel Adds the potential between @p p_i and @p p_j to the accumulator @p energy
     * @return The total potential energy
     */
    template <class Kernel>
        requires requires(const Kernel m, ParticleRef<const ParticleContainer> a,
                          ParticleRef<const ParticleContainer> b) {
            { m.Potential(a, b) } -> std::convertible_to<double>;
        }
    double CalculateEnergy(const Kernel kernel) const {
        double energy = 0.0;
        const std::size_t n = size();

        for (std::size_t i = 0; i < n; ++i) {
            const ParticleRef<const ParticleContainer> p_i(*this, i);
            // add the kinetic energy from this particle
            energy += 0.5 * p_i.m() * ArrayUtils::L2NormSquared(p_i.v());

            // Not strictly necessary but no compiler promise violated
            INDEPENDENT_ITERATIONS
            for (std::size_t j = i + 1; j < n; ++j) {
                const ParticleRef<const ParticleContainer> p_j(*this, j);
                // add the potential energy to the accumulator energy
                energy += kernel.Potential(p_i, p_j);
            }
        }
        return energy;
    }
};

// Find the mirrored ghost position, then calculate its repulsive force.
template <class BoundaryKernel>
Vec3 ParticleContainer::GhostForce(BoundaryKernel boundary_kernel, auto p_i, Face face) {
    const Vec3 position = p_i.x();
    const Vec3 ghost_x = GhostParticlePosition(position, face);
    return boundary_kernel(position, ghost_x);
}

// Applies the boundary-kernel force to each particle in the specified range.
template <class BoundaryKernel>
void ParticleContainer::GhostParticleBoundryForce(BoundaryKernel boundary_kernel, Index a_start,
                                                  Index a_end, Face face) {
    for (Index i = a_start; i < a_end; ++i) {
        const ParticleRef<const ParticleContainer> p_i(*this, i);
        const Vec3 f_i = GhostForce(boundary_kernel, p_i, face);
        f_.set(i, f_.get(i) + f_i);
    }
}

template <PairKernel Kernel>
void ParticleContainer::PairsWithinAndNext(const Kernel kernel, const Index start, const Index end,
                                           const Index next_end) {
    for (Index i = start; i < end; ++i) {
        const ParticleRef<const ParticleContainer> p_i(*this, i);
        Vec3 f_i{};

        // Safe because every j > i is visited once per i. F_j is written so the write locations
        // dont overlap and only reads the Force it writes to in the same iteration
        INDEPENDENT_ITERATIONS
        for (Index j = i + 1; j < next_end; j++) {
            const ParticleRef<const ParticleContainer> p_j(*this, j);
            const Vec3 f_ij = kernel(p_i, p_j);
            f_i = f_i + f_ij;
            f_.set(j, f_.get(j) - f_ij);
        }
        f_.set(i, f_.get(i) + f_i);
    }
}

template <PairKernel Kernel>
void ParticleContainer::PairsBetween(const Kernel kernel, const Index a_start, const Index a_end,
                                     const Index b_start, const Index b_end) {
    assert(a_end <= b_start || b_end <= a_start);
    if (b_start == b_end) return;
    for (Index i = a_start; i < a_end; ++i) {
        const ParticleRef<const ParticleContainer> p_i(*this, i);
        Vec3 f_i{};
        // Safe when used correctly (see assert)
        INDEPENDENT_ITERATIONS
        for (Index j = b_start; j < b_end; j++) {
            const ParticleRef<const ParticleContainer> p_j(*this, j);
            const Vec3 f_ij = kernel(p_i, p_j);
            f_i = f_i + f_ij;
            f_.set(j, f_.get(j) - f_ij);
        }
        f_.set(i, f_.get(i) + f_i);
    }
}
template <PairKernel Kernel>
void ParticleContainer::PairsOfCell(const Kernel kernel, const CellCoords c) {
    PairsWithinAndNext(kernel, start(c), end(c), end(c + CellCoords{1, 0, 0}));

    for (auto row : {CellOffset{-1, 1, -1}, CellOffset{-1, 0, 1}, CellOffset{-1, 1, 1},
                     CellOffset{-1, 1, 0}}) {
        PairsBetween(kernel, start(c), end(c), start(c + row), end(c + row + CellOffset{2, 0, 0}));
    }
}

#undef INDEPENDENT_ITERATIONS
