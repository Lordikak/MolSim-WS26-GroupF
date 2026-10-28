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

class XYZWriter final : public OutputWriter {
    std::string prefix_;

   public:
    explicit XYZWriter(std::string prefix) : prefix_(std::move(prefix)) {}

    void Write(const ParticleContainer &particles, int iteration) override;
};

}  // namespace output_writer
