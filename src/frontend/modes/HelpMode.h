//
// Created by Piotr Kubala on 25/01/2023.
//

#ifndef RAMPACK_HELPMODE_H
#define RAMPACK_HELPMODE_H


#include "frontend/ModeBase.h"


class HelpMode : public ModeBase {
public:
    explicit HelpMode(Logger &logger) : ModeBase(logger) { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_HELPMODE_H
