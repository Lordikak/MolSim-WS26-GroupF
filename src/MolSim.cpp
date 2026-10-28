
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
void PlotParticles(const ParticleContainer &particles,
                   const std::vector<std::unique_ptr<output_writer::OutputWriter>> &writers,
                   int iteration);

constexpr double kStartTime = 0;

int main(int argc, char *argsv[]) {
    // Set by CLI11
    double delta_t;
    double end_time;

    ParticleContainer particles;
    std::string file_path;

    const std::map<std::string, output_writer::OutputFormat> format_names{
        {"xyz", output_writer::OutputFormat::XYZ},
#ifdef ENABLE_VTK_OUTPUT
        {"vtk", output_writer::OutputFormat::VTK},
#endif
    };
    std::vector<std::unique_ptr<output_writer::OutputWriter>> writers;

    // parse command line inputs with CLI 11
    CLI::App app("MD simulation");

    // optional arguments for delta T and end time, provided with default values
    app.add_option("file", file_path, "Input File")->required()->check(CLI::ExistingFile);

    app.add_option("--dt", delta_t, "Time step per iteration")
        ->default_val(0.014)
        ->check(CLI::PositiveNumber);
    app.add_option("--end", end_time, "End Time of the sim")
        ->default_val(1000.0)
        ->check(CLI::PositiveNumber);
    std::vector<output_writer::OutputFormat> output_formats;
    app.add_option("--format", output_formats, "Output format")
        ->transform(CLI::CheckedTransformer(format_names, CLI::ignore_case))
        ->delimiter(',')
        ->option_text("xyz,vtk");

    app.footer(
        "Example:\n"
        "./MolSim input/eingabe-sonne.txt --dt 0.014 --end 1000");
    app.failure_message(CLI::FailureMessage::help);

    // parse with library functions
    CLI11_PARSE(app, argc, argsv);

    std::cout << "Hello from MolSim for PSE!" << std::endl;

    FileReader file_reader;
    file_reader.ReadFile(particles, file_path);

    // Add default writers
    if (output_formats.empty()) {
        output_formats.push_back(output_writer::OutputFormat::XYZ);
    }

    // Create Writers
    const std::string out_name("MD_iteration");
    for (const auto output_format : output_formats) {
        writers.push_back(output_writer::OutputWriter::MakeWriter(output_format, out_name));
    }

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
            PlotParticles(particles, writers, iteration);
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

void PlotParticles(const ParticleContainer &particles,
                   const std::vector<std::unique_ptr<output_writer::OutputWriter>> &writers,
                   const int iteration) {
    for (const auto &writer : writers) {
        writer->Write(particles, iteration);
    }
}
