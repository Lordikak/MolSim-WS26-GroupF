#include <algorithm>
#include <chrono>
#include <cstdio>
#include <list>
#include <random>
#include <vector>

#include "utils/Vec3.h"
#include <cmath>
#include "../Bench.h"

struct Particle {
    Vec3 x{};
    Vec3 v{};
    Vec3 f{};
    Vec3 old_f{};
    double m{};
};

// Copied unchanged from commit a0ad2bd (src/MolSim.cpp):
void calculateF(std::list<Particle> &particles) {
    std::list<Particle>::iterator iterator;
    iterator = particles.begin();

    for (auto &p1 : particles) {
        std::array<double, 3> f_acting_on_1 = {0., 0., 0.};

        const double m1 = p1.m;
        const std::array<double, 3> x1 = p1.x;

        for (auto &p2 : particles) {
            const double m2 = p2.m;
            const std::array<double, 3> x2 = p2.x;

            const std::array<double, 3> delta_x = {x2[0] - x1[0], x2[1] - x1[1], x2[2] - x1[2]};

            const double length = std::sqrt(delta_x[0] * delta_x[0] + delta_x[1] * delta_x[1] +
                                            delta_x[2] * delta_x[2]);

            if (length == 0) {
                continue;
            }

            const double scale = m1 * m2 / (length * length * length);

            for (std::size_t i = 0; i < f_acting_on_1.size(); ++i) {
                f_acting_on_1[i] += scale * delta_x[i];
            }
        }

        p1.f= f_acting_on_1;
    }
}

int main() {
    constexpr std::size_t N = 4000;
    constexpr int kReps = 6;
    constexpr int kWarmups = 1;

    std::list<Particle> particles;

    for (const auto& p : MakeParticles(N)) {
        particles.push_back({p.x, p.v, {}, {}, p.m});
    }

    const double ns = MedianNsPerN2([&] {
        calculateF(particles);
    }, N, kReps, kWarmups);

    double checksum = 0;
    for (const auto& p : particles) {
        const Vec3 f = p.f;
        checksum += std::abs(f[0]) + std::abs(f[1]) + std::abs(f[2]);
    }

    std::printf("original %.3f cs: %.10g\n", ns, checksum);
}

