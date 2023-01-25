//
// Created by Piotr Kubala on 25/01/2023.
//

#ifndef RAMPACK_PREVIEWMODE_H
#define RAMPACK_PREVIEWMODE_H

#include "frontend/ModeBase.h"


class PreviewMode : public ModeBase {
public:
    explicit PreviewMode(Logger &logger) : ModeBase(logger) { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_PREVIEWMODE_H
