
#include <cmath>
#include <filesystem>
#include <iostream>
#include <list>

#include "FileReader.h"
#include "outputWriter/VTKWriter.h"
#include "outputWriter/XYZWriter.h"
#include "utils/ArrayUtils.h"

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
#elif defined(__GNUC__)
#define INDEPENDENT_ITERATIONS _Pragma("GCC ivdep")
#else
#define INDEPENDENT_ITERATIONS
#endif

/**** forward declaration of the calculation functions ****/

/**
 * calculate the force for all particles
 */
void CalculateF(ParticleContainer &particles);

/**
 * calculate the position for all particles
 */
void CalculateX(ParticleContainer &particles, double delta_t);

/**
 * calculate the position for all particles
 */
void CalculateV(ParticleContainer &particles, double delta_t);

/**
 * plot the particles to a xyz-file
 */
void PlotParticles(ParticleContainer &particles, int iteration);

constexpr double kStartTime = 0;

int main(int argc, char *argsv[]) {
    double delta_t = 0.014;
    double end_time = 1000;

    std::cout << "Hello from MolSim for PSE!" << std::endl;
    if (argc != 2 && argc != 4) {
        std::cout << "Erroneous programme call! Usage:\n "
                  << "./MolSim filename deltaT endTime\nor\n"
                  << "./MolSim filename" << std::endl;
        return 1;
    }
    ParticleContainer particles;

    // reinitialize only upon provided replacement values
    if (argc == 4) {
        try {
            delta_t = std::stod(argsv[2]);
            end_time = std::stod(argsv[3]);
        } catch (const std::exception &_) {
            std::cerr << "Invalid arguments for deltaT and/or endTime" << "\n";
            return 1;
        }
    }

    FileReader file_reader;
    file_reader.ReadFile(particles, argsv[1]);

    // reset ouput folder
    std::filesystem::remove_all("output");
    std::filesystem::create_directories("output");
    std::filesystem::create_directories("output/XYZ");
    std::filesystem::create_directories("output/VTK");

    double current_time = kStartTime;

    int iteration = 0;

    // for this loop, we assume: current x, current f and current v are known
    while (current_time < end_time) {
        CalculateX(particles, delta_t);
        CalculateF(particles);
        CalculateV(particles, delta_t);

        iteration++;
        if (iteration % 10 == 0) {
            PlotParticles(particles, iteration);
        }
        std::cout << "Iteration " << iteration << " finished.\n";

        current_time += delta_t;
    }

    std::cout << "output written. Terminating..." << std::endl;
    return 0;
}

void CalculateX(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetX(p.x() + delta_t * p.v() + (delta_t * delta_t / (2 * p.m())) * p.f());
    }
}

/**
 * @brief Computes gravitational force for every particle
 *
 * F_ij = m_i * m_j / |x_i - x_j|^3 * (x_j - x_i)
 * computed once per pair and then applies to both using newtons third law: F_ij = - F_ji
 * which halves the work
 *
 * @param particles particles to compute the forces for
 */
void CalculateF(ParticleContainer &particles) {
    // Write the forces for the last step.
    for (auto p : particles) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    for (auto p_i : particles) {
        const Vec3 x_i = p_i.x();
        const double m_i = p_i.m();
        Vec3 f1{};

        // Safe because every j > i is visited once per i. F_j is written so the write locations
        // dont overlap and only reads the Force it writes to in the same iteration
        INDEPENDENT_ITERATIONS
        for (std::size_t j = p_i.index() + 1; j < particles.size(); j++) {
            auto p_j = particles.getParitcleByIndex(j);
            const Vec3 x_j = p_j.x();
            const double m_j = p_j.m();

            const Vec3 delta_x = x_j - x_i;
            const double length = ArrayUtils::L2Norm(delta_x);

            const Vec3 f_ij = (m_i * m_j / (length * length * length)) * delta_x;

            p_i.SetF(p_i.f() + f_ij);
            // newton 3rd law
            p_j.SetF(p_j.f() - f_ij);
        }
    }
}

void CalculateV(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetV(p.v() + delta_t * (p.f() + p.old_f()) / (2 * p.m()));
    }
}

void PlotParticles(ParticleContainer &particles, const int iteration) {
    const std::string out_name("MD_itaration");

    output_writer::XYZWriter xyz_writer;
    xyz_writer.PlotParticles(particles, out_name, iteration);

#ifdef ENABLE_VTK_OUTPUT
    output_writer::VTKWriter vtk_writer;
    vtk_writer.PlotParticles(particles, out_name, iteration);
#endif
}
