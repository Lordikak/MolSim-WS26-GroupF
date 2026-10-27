#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <random>
#include <vector>

struct InitParticle {
    std::array<double, 3> x;
    std::array<double, 3> v;
    double m;
};

inline std::vector<InitParticle> MakeParticles(const std::size_t n) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> x(0., 100.);
    std::uniform_real_distribution<double> v(-1., 1.);
    std::uniform_real_distribution<double> m(1., 4.);

    std::vector<InitParticle> particles(n);
    for (auto &p : particles) {
        p.x = {x(rng), x(rng), x(rng)};
        p.v = {v(rng), v(rng), v(rng)};
        p.m = m(rng);
    }
    return particles;
}

template <class F>
double MedianNsPerN2(F compute_f, std::size_t n, int reps, int warmups) {
    std::vector<double> seconds;
    seconds.reserve(reps);
    for (int r = 0; r < reps + warmups; ++r) {
        const auto start = std::chrono::steady_clock::now();
        compute_f();
        const auto end = std::chrono::steady_clock::now();
        if (r >= warmups) {
            seconds.push_back(std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count());
        }
    }
    std::ranges::sort(seconds);
    const double med = seconds[seconds.size() / 2];

    return med * 1e9 / (n*n);
}