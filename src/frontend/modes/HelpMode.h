//
// Created by Piotr Kubala on 25/01/2023.
//

#ifndef RAMPACK_HELPMODE_H
#define RAMPACK_HELPMODE_H


#include "frontend/ModeBase.h"


class HelpMode : public ModeBase {
private:
    std::string cmd;    // Original cmd (not argv[0], which is now cmd + mode combined)

public:
    explicit HelpMode(Logger &logger, std::string cmd) : ModeBase(logger), cmd{std::move(cmd)} { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_HELPMODE_H
