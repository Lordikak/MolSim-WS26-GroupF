
#include <cmath>
#include <filesystem>
#include <iostream>
#include <list>

#include "FileReader.h"
#include "outputWriter/XYZWriter.h"
#include "utils/ArrayUtils.h"

/**** forward declaration of the calculation functions ****/

/**
 * calculate the force for all particles
 */
void calculateF();

/**
 * calculate the position for all particles
 */
void calculateX();

/**
 * calculate the position for all particles
 */
void calculateV();

/**
 * plot the particles to a xyz-file
 */
void plotParticles(int iteration);

constexpr double start_time = 0;
constexpr double end_time = 1000;
// constexpr double end_time = 0.014 * 3;

constexpr double delta_t = 0.014;

// TODO: what data structure to pick?
std::list<Particle> particles;

int main(int argc, char *argsv[]) {
    std::cout << "Hello from MolSim for PSE!" << std::endl;
    if (argc != 2) {
        std::cout << "Erroneous programme call! " << std::endl;
        std::cout << "./molsym filename" << std::endl;
    }

    FileReader fileReader;
    fileReader.readFile(particles, argsv[1]);

    // reset ouput folder
    std::filesystem::remove_all("output");
    std::filesystem::create_directories("output");

    double current_time = start_time;

    int iteration = 0;

    // for this loop, we assume: current x, current f and current v are known
    while (current_time < end_time) {
        // calculate new x
        calculateX();
        // calculate new f
        calculateF();
        // calculate new v
        calculateV();

        iteration++;
        if (iteration % 10 == 0) {
            plotParticles(iteration);
        }
        std::cout << "Iteration " << iteration << " finished." << std::endl;

        current_time += delta_t;
    }

    std::cout << "output written. Terminating..." << std::endl;
    return 0;
}

void calculateX() {
    for (auto &p : particles) {
        const std::array<double, 3> &x = p.getX();
        const std::array<double, 3> &f = p.getF();
        const double m = p.getM();

        std::array<double, 3> new_x;

        for (std::size_t i = 0; i < new_x.size(); ++i) {
            new_x[i] = x[i] + delta_t * f[i] + (delta_t * delta_t / m) * f[i];
        }
        p.setX(new_x);
    }
}

void calculateF() {
    std::list<Particle>::iterator iterator;
    iterator = particles.begin();

    for (auto &p1 : particles) {
        std::array<double, 3> f_acting_on_1 = {0., 0., 0.};

        const double m1 = p1.getM();
        const std::array<double, 3> x1 = p1.getX();

        for (auto &p2 : particles) {
            const double m2 = p2.getM();
            const std::array<double, 3> x2 = p2.getX();

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

        p1.setF(f_acting_on_1);
    }
}

void calculateV() {
    for (auto &p : particles) {
        const std::array<double, 3> &v = p.getV();
        const std::array<double, 3> &f = p.getF();
        const std::array<double, 3> &old_f = p.getOldF();
        const double m = p.getM();

        std::array<double, 3> new_v;

        for (std::size_t i = 0; i < new_v.size(); ++i) {
            new_v[i] = v[i] + delta_t * (f[i] + old_f[i]) / (2 * m);
        }
        p.setV(new_v);
    }
}

void plotParticles(int iteration) {
    std::string out_name("MD_vtk");

    outputWriter::XYZWriter writer;
    writer.plotParticles(particles, out_name, iteration);
}
