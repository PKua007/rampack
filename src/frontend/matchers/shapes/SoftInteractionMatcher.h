//
// Created by Codex on 05/04/2026.
//

#ifndef RAMPACK_SOFTINTERACTIONMATCHER_H
#define RAMPACK_SOFTINTERACTIONMATCHER_H

#include <memory>
#include <vector>

#include "pyon/Matcher.h"
#include "core/interactions/CentralInteractionBase.h"


class SoftInteractionFactory {
public:
    virtual ~SoftInteractionFactory() = default;

    [[nodiscard]] virtual bool supportsTypes(const std::vector<std::string> &typeLabels) const = 0;
    [[nodiscard]] virtual std::shared_ptr<CentralInteractionBase>
    createForTypes(const std::vector<std::string> &typeLabels) const = 0;
};


class SoftInteractionMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
};


#endif //RAMPACK_SOFTINTERACTIONMATCHER_H
