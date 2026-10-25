
#include <cmath>
#include <filesystem>
#include <iostream>
#include <list>

#include "FileReader.h"
#include "outputWriter/VTKWriter.h"
#include "outputWriter/XYZWriter.h"
#include "utils/ArrayUtils.h"

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
// constexpr double end_time = 0.014 * 10;
double kEndTime = 1000;

double kDeltaT = 0.014;

int main(int argc, char *argsv[]) {
    std::cout << "Hello from MolSim for PSE!" << std::endl;
    if (argc != 2 && argc != 4) {
        std::cout << "Erroneous programme call! Usage:\n "
                << "./MolSim filename deltaT endTime\nor\n" 
                << "./MolSim filename" << std::endl;
        return 1;
    }
    ParticleContainer particles;

    //reinitialize only upon provided replacement values
    if(argc == 4){
        try {
            kDeltaT = std::stod(argsv[2]);
            kEndTime = std::stod(argsv[3]);
        }
        catch (const std::exception& e) {
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

void CalculateF(ParticleContainer &particles) {
    for (auto p : particles) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    for (auto p1 : particles) {
        const Vec3 x1 = p1.x();
        const double m1 = p1.m();
        Vec3 f1{};

        INDEPENDENT_ITERATIONS
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
