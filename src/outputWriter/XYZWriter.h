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
    std::filesystem::path dir_;
    std::string prefix_;

   public:
    /**
     * @brief Creates the Writer
     * @param dir The directory the output gets written to, existing files get removed
     * @param prefix File name prefix, files get named: `<prefix>_<iteration>.<extension>`
     */
    explicit XYZWriter(std::filesystem::path dir, std::string prefix)
        : dir_(std::move(dir)), prefix_(std::move(prefix)) {
        std::filesystem::remove_all(dir_);
        std::filesystem::create_directories(dir_);
    }

    /// @copydoc OutputWriter::Write
    void Write(const ParticleContainer &particles, int iteration) override;
};

}  // namespace output_writer
