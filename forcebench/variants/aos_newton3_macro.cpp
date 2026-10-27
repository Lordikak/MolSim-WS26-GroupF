#include <vector>
#include <cmath>
#include "../Bench.h"
#include <cstdio>

#include "utils/Vec3.h"

#if defined(__clang__)
#define INDEPENDENT_ITERATIONS _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__)
#define INDEPENDENT_ITERATIONS _Pragma("GCC ivdep")
#else
#define INDEPENDENT_ITERATIONS
#endif

struct Particle {
    Vec3 x{};
    Vec3 v{};
    Vec3 f{};
    Vec3 old_f{};
    double m{};
};

using ParticleContainer = std::vector<Particle>;

void CalculateF(ParticleContainer &particles) {
    // Write the forces for the last step.
    for (auto &p : particles) {
        p.old_f = p.f;
        p.f = Vec3{};
    }

    for (std::size_t i = 0; i < particles.size(); ++i) {
        auto &p1 = particles[i];
        const Vec3 x1 = p1.x;
        const double m1 = p1.m;
        Vec3 f1{};

        INDEPENDENT_ITERATIONS
        for (std::size_t j = i + 1; j < particles.size(); j++) {
            const Vec3 delta_x = particles[j].x - x1;
            const double length = ArrayUtils::L2Norm(delta_x);
            const Vec3 f_ij = (m1 * particles[j].m / (length * length * length)) * delta_x;
            f1 = f1 + f_ij;
            particles[j].f = particles[j].f - f_ij;
        }
        p1.f = p1.f + f1;
    }
}

int main() {
    constexpr std::size_t N = 4000;
    constexpr int kReps = 6;
    constexpr int kWarmups = 1;

    ParticleContainer particles;

    particles.reserve(N);
    for (const auto& p : MakeParticles(N)) {
        particles.push_back({p.x, p.v, {}, {}, p.m});
    }

    const double ns = MedianNsPerN2([&] {
        CalculateF(particles);
    }, N, kReps, kWarmups);

    double checksum = 0;
    for (const auto& p : particles) {
        const Vec3 f = p.f;
        checksum += std::abs(f[0]) + std::abs(f[1]) + std::abs(f[2]);
    }

    std::printf("aos_newton3_macro %.3f cs: %.10g\n", ns, checksum);
}

