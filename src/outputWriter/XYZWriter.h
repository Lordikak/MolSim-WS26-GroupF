/*
 * XYZWriter.h
 *
 *  Created on: 01.03.2010
 *      Author: eckhardw
 */

#pragma once

#include <fstream>
#include <string>
#include <utility>

#include "OutputWriter.h"
#include "ParticleContainer.h"

namespace output_writer {

/**
 * @brief Writes particle positions
 */
class XYZWriter final : public OutputWriter {
    std::string prefix_;

   public:
    /**
     * @brief Creates the Writer
     * @param prefix File name prefix, files get named: `<prefix>_<iteration>.<extension>`
     */
    explicit XYZWriter(std::string prefix) : prefix_(std::move(prefix)) {}

    /// @copydoc OutputWriter::Write
    void Write(const ParticleContainer &particles, int iteration) override;
};

}  // namespace output_writer
