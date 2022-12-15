//
// Created by Piotr Kubala on 15/12/2022.
//

#include "MatcherAlternative.h"

namespace pyon::matcher {
    bool MatcherAlternative::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        for (const auto &alternative : this->alternatives)
            if (alternative->match(node, result))
                return true;

        return false;
    }
} // matcher