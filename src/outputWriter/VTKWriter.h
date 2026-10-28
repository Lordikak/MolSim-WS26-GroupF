/*
 * VTKWriter.h
 *
 *  Created on: 01.03.2010
 *      Author: eckhardw
 */

#pragma once
#ifdef ENABLE_VTK_OUTPUT

#include <string>
#include <utility>

#include "OutputWriter.h"
#include "ParticleContainer.h"

namespace output_writer {

/**
 * This class implements the functionality to generate vtk output from
 * particles using the official VTK library.
 */
class VTKWriter final : public OutputWriter {
    std::string prefix_;
    const bool ascii_;

   public:
    /**
     * @brief Creates a VTK writer
     *
     * @todo Pass the Output dir from MakeWriter instead of hardcoding
     *
     * @param prefix File name prefix, files get named: `<prefix>_<iteration>.<extension>`
     * @param ascii If the file should be written as ascii
     */
    explicit VTKWriter(std::string prefix, const bool ascii)
        : prefix_(std::move(prefix)), ascii_(ascii) {}

    // Delete copy constructor and assignment operator
    VTKWriter(const VTKWriter &) = delete;
    VTKWriter &operator=(const VTKWriter &) = delete;

    /**
     * Write VTK output of particles.
     * @param particles Particles to add to the output
     * @param iteration Current iteration number
     */
    void Write(const ParticleContainer &particles, int iteration) override;
};

}  // namespace output_writer
#endif