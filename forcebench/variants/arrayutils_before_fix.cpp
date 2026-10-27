#include <array>
#include <cmath>
#include <list>

#include "../legacy/ArrayUtils.h"
#include "../Bench.h"

using Vec3 = std::array<double, 3>;

struct Particle {
    Vec3 x{};
    Vec3 v{};
    Vec3 f{};
    Vec3 old_f{};
    double m{};
};

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

            const std::array<double, 3> delta_x = x2 - x1;

            const double length = ArrayUtils::L2Norm(delta_x);

            if (length == 0) {
                continue;
            }

            const double scale = m1 * m2 / (length * length * length);

            f_acting_on_1 = f_acting_on_1 + scale * delta_x;
        }

        p1.f = f_acting_on_1;
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

    std::printf("arrayutils_before_fix %.3f cs: %.10g\n", ns, checksum);
}
