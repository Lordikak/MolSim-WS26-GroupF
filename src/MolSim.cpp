
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
void CalculateX(ParticleContainer &particles);

/**
 * calculate the position for all particles
 */
void CalculateV(ParticleContainer &particles);

/**
 * plot the particles to a xyz-file
 */
void PlotParticles(ParticleContainer &particles, int iteration);

constexpr double kStartTime = 0;
constexpr double kEndTime = 1000;
// constexpr double end_time = 0.014 * 10;

constexpr double kDeltaT = 0.014;

int main(int argc, char *argsv[]) {
    std::cout << "Hello from MolSim for PSE!" << std::endl;
    if (argc != 2) {
        std::cout << "Erroneous programme call! " << std::endl;
        std::cout << "./molsym filename" << std::endl;
    }
    ParticleContainer particles;

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
    while (current_time < kEndTime) {
        // calculate new x
        CalculateX(particles);
        // calculate new f
        CalculateF(particles);
        // calculate new v
        CalculateV(particles);

        iteration++;
        if (iteration % 10 == 0) {
            PlotParticles(particles, iteration);
        }
        std::cout << "Iteration " << iteration << " finished." << std::endl;

        current_time += kDeltaT;
    }

    std::cout << "output written. Terminating..." << std::endl;
    return 0;
}

void CalculateX(ParticleContainer &particles) {
    for (auto p : particles) {
        p.SetX(p.x() + kDeltaT * p.v() + (kDeltaT * kDeltaT / (2 * p.m())) * p.f());
    }
}

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

void CalculateV(ParticleContainer &particles) {
    for (auto p : particles) {
        p.SetV(p.v() + kDeltaT * (p.f() + p.old_f()) / (2 * p.m()));
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
