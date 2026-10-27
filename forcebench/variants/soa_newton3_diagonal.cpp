#include "../Bench.h"
#include "ParticleContainer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

#if defined(__clang__)
#define INDEPENDENT_ITERATIONS _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__)
#define INDEPENDENT_ITERATIONS _Pragma("GCC ivdep")
#else
#define INDEPENDENT_ITERATIONS
#endif

void CalculateF(ParticleContainer &particles) {
    for (auto p : particles) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    const std::size_t n = particles.size();
    for (std::size_t d = 1; d < n; ++d) {
        for (std::size_t start = 0; start < n - d; start += d) {
            const std::size_t end = std::min(start + d, n - d);

            INDEPENDENT_ITERATIONS
            for (std::size_t i = start; i < end; ++i) {
                const std::size_t j = i + d;
                const Vec3 delta_x = particles.x(j) - particles.x(i);
                const double length = ArrayUtils::L2Norm(delta_x);
                const Vec3 f_ij = (particles.m(i) * particles.m(j) / (length * length * length)) * delta_x;
                particles.SetF(i, particles.f(i) + f_ij);
                particles.SetF(j, particles.f(j) - f_ij);
            }
        }
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

    std::printf("soa_newton3_diagonal %.3f cs: %.10g\n", ns, checksum);
}
