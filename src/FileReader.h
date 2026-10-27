/*
 * FileReader.h
 *
 *  Created on: 23.02.2010
 *      Author: eckhardw
 */

#pragma once

#include "ParticleContainer.h"
#include <string>



class FileReader {
   public:
    FileReader();
    virtual ~FileReader();

    static void ReadFile(ParticleContainer &particles, const std::string &filename);
};
