/*
 * XYZWriter.h
 *
 *  Created on: 01.03.2010
 *      Author: eckhardw
 */

#pragma once

#include <fstream>

#include "ParticleContainer.h"

namespace output_writer {

class XYZWriter {
   public:
    XYZWriter();

    virtual ~XYZWriter();

    void PlotParticles(const ParticleContainer &particles, const std::string &filename,
                       int iteration);
};

}  // namespace output_writer
