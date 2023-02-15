//
// Created by Piotr Kubala on 15/02/2023.
//

#ifndef RAMPACK_VERSIONMODE_H
#define RAMPACK_VERSIONMODE_H

#include "frontend/ModeBase.h"


class VersionMode : public ModeBase {
public:
    explicit VersionMode(Logger &logger) : ModeBase(logger) { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_VERSIONMODE_H
