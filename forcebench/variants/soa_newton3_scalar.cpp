#include "../Bench.h"
#include "ParticleContainer.h"
#include <cstdio>
#include <cmath>

void CalculateF(ParticleContainer &particles) {
    // Write the forces for the last step.
    for (auto p : particles) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    for (auto p1 : particles) {
        const Vec3 x1 = p1.x();
        const double m1 = p1.m();
        Vec3 f1{};

        for (std::size_t j = p1.index() + 1; j < particles.size(); j++) {
            const Vec3 delta_x = particles.x(j) - x1;
            const double length = ArrayUtils::L2Norm(delta_x);
            const Vec3 f_ij = (m1 * particles.m(j) / (length * length * length)) * delta_x;
            f1 = f1 + f_ij;
            particles.SetF(j, particles.f(j) - f_ij);
        }
        p1.SetF(p1.f() + f1);
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

    std::printf("soa_newton3_scalar %.3f cs: %.10g\n", ns, checksum);
}
