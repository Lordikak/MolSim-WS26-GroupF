
#include <CLI11/CLI11.hpp>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <list>

#include "FileReader.h"
#include "outputWriter/VTKWriter.h"
#include "outputWriter/XYZWriter.h"
#include "utils/ArrayUtils.h"

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
    // declaration and intialization with default values
    // replaced if program is run with optional inputs
    double delta_t = 0.014;
    double end_time = 1000;

    std::cout << "Hello from MolSim for PSE!" << std::endl;
    ParticleContainer particles;
    std::string file_path;

    // parse command line inputs with CLI 11
    CLI::App app("Optional argument parser");

    // optional arguments for delta T and end time, provided with default values
    app.add_option("file", file_path, "Input File")->required()->check(CLI::ExistingFile);

    app.add_option("--dt", delta_t, "K DeltaT")->check(CLI::PositiveNumber);
    app.add_option("--end", end_time, "End Time")->check(CLI::PositiveNumber);

    // parse with library functions
    try {
        CLI11_PARSE(app, argc, argsv);
    } catch (const CLI::ParseError &_) {
        std::cerr << "Error parsing inputs. Usage:\n"
                  << "./MolSim filepath\n"
                  << "additional options:\n"
                  << "--dt double: define a positive double value as the delta T for simulation\n"
                  << "--end endtime: define a positive double value as the end time for simulation"
                  << std::endl;
        return 1;
    }

    FileReader file_reader;
    file_reader.ReadFile(particles, file_path);

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
    // store old force in old_f and reset it, as it is calculated in the next step
    for (auto p : particles) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    particles.ApplyPairForces([](auto p_i, auto p_j) {
        const Vec3 delta_x = p_j.x() - p_i.x();
        const double length = ArrayUtils::L2Norm(delta_x);
        return (p_i.m() * p_j.m() / (length * length * length)) * delta_x;
    });
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
