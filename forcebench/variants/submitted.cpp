#define main molsim_main
#include "../../src/MolSim.cpp"
#undef main

#include <cmath>
#include <cstdio>

#include "../Bench.h"

int main() {
    constexpr std::size_t N = 4000;
    constexpr int kReps = 6;
    constexpr int kWarmups = 1;

    ParticleContainer particles;
    particles.reserve(N);
    for (const auto &p : MakeParticles(N)) {
        particles.add(p.x, p.v, p.m);
    }

    const double ns = MedianNsPerN2([&] {
        CalculateF(particles);
    }, N, kReps, kWarmups);

    double checksum = 0;
    for (const auto &p : particles) {
        const Vec3 f = p.f();
        checksum += std::abs(f[0]) + std::abs(f[1]) + std::abs(f[2]);
    }

    std::printf("submitted %.3f cs: %.10g\n", ns, checksum);
}