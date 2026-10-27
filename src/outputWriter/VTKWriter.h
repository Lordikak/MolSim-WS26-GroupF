/*
 * VTKWriter.h
 *
 *  Created on: 01.03.2010
 *      Author: eckhardw
 */

#pragma once
#ifdef ENABLE_VTK_OUTPUT

#include <string>

#include "ParticleContainer.h"

namespace output_writer {

/**
 * This class implements the functionality to generate vtk output from
 * particles using the official VTK library.
 */
class VTKWriter {
   public:
    VTKWriter() = default;
    ~VTKWriter() = default;

    // Delete copy constructor and assignment operator
    VTKWriter(const VTKWriter &) = delete;
    VTKWriter &operator=(const VTKWriter &) = delete;

    /**
     * Write VTK output of particles.
     * @param particles Particles to add to the output
     * @param filename Output filename
     * @param iteration Current iteration number
     */
    static void PlotParticles(const ParticleContainer &particles, const std::string &filename,
                              int iteration);
};

}  // namespace output_writer
#endif