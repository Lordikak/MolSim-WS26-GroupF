/*
 * FileReader.h
 *
 *  Created on: 23.02.2010
 *      Author: eckhardw
 */

#pragma once

#include <list>

#include "Particle.h"
#include "ParticleContainer.h"

class FileReader {
   public:
    FileReader();
    virtual ~FileReader();

    static void ReadFile(ParticleContainer &particles, char *filename);
};
