/*
 * XYZWriter.h
 *
 *  Created on: 01.03.2010
 *      Author: eckhardw
 */

#pragma once

#include <fstream>
#include <list>

#include "Particle.h"
#include "ParticleContainer.h"

namespace outputWriter {

class XYZWriter {
   public:
    XYZWriter();

    virtual ~XYZWriter();

    void PlotParticles(const ParticleContainer &particles, const std::string &filename,
                       int iteration);
};

}  // namespace outputWriter
