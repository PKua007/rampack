//
// Created by Piotr Kubala on 17/01/2023.
//

#ifndef RAMPACK_INIPARAMETERSFACTORY_H
#define RAMPACK_INIPARAMETERSFACTORY_H

#include "frontend/Parameters.h"
#include "frontend/RampackParameters.h"


// TODO: who does the validation?
class IniParametersFactory {
public:
    static RampackParameters create(const Parameters &params);
};


#endif //RAMPACK_INIPARAMETERSFACTORY_H
