#include "ParticleContainer.h"
#include "utils/Vec3.h"
#include <cmath>
#include "../Bench.h"


Vec3 AddForces(const ParticleContainer &particles, const std::size_t start, const std::size_t end,
               const Vec3 &x1, const double m1, Vec3 f) {
    for (std::size_t j = start; j < end; j++) {
        const Vec3 delta_x = particles.x(j) - x1;
        const double length = ArrayUtils::L2Norm(delta_x);
        const double scale = m1 * particles.m(j) / (length * length * length);
        f = f + scale * delta_x;
    }
    return f;
}

void CalculateF(ParticleContainer &particles) {
    for (auto p1 : particles) {
        const double m1 = p1.m();
        const Vec3 x1 = p1.x();

        const auto i = p1.index();

        auto f_acting_on_1 = AddForces(particles, 0, i, x1, m1, Vec3{});
        f_acting_on_1 = AddForces(particles, i + 1, particles.size(), x1, m1, f_acting_on_1);

        p1.SetOldF(p1.f());
        p1.SetF(f_acting_on_1);
    }
}

int main() {
    constexpr std::size_t N = 4000;
    constexpr int kReps = 6;
    constexpr int kWarmups = 1;

    ParticleContainer particles;

    particles.reserve(N);
    for (const auto& p : MakeParticles(N)) {
        particles.add(p.x, p.v, p.m);
    }

    const double ns = MedianNsPerN2([&] {
        CalculateF(particles);
    }, N, kReps, kWarmups);

    double checksum = 0;
    for (const auto& p : particles) {
        const Vec3 f = p.f();
        checksum += std::abs(f[0]) + std::abs(f[1]) + std::abs(f[2]);
    }

    std::printf("soa_vectorized_all_pairs %.3f cs: %.10g\n", ns, checksum);
}
