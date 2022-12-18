//
// Created by Piotr Kubala on 15/12/2022.
//

#ifndef RAMPACK_MATCHERALTERNATIVE_H
#define RAMPACK_MATCHERALTERNATIVE_H

#include <vector>

#include "MatcherBase.h"


namespace pyon::matcher {
    class MatcherAlternative : public MatcherBase {
    private:
        std::vector<std::shared_ptr<MatcherBase>> alternatives;

    public:
        MatcherAlternative() = default;

        [[nodiscard]] MatcherAlternative copy() const { return *this; }

        bool match(std::shared_ptr<const ast::Node> node, Any &result) const override;

        template<typename ConcreteMatcher>
        MatcherAlternative &operator|=(const ConcreteMatcher &matcher) {
            static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher>,
                          "ConcreteMatcher template parameter is not a matcher derived from MatcherBase");

            if constexpr (std::is_same_v<MatcherAlternative, std::decay_t<ConcreteMatcher>>) {
                this->alternatives.insert(this->alternatives.end(),
                                          matcher.alternatives.begin(), matcher.alternatives.end());
            } else {
                this->alternatives.push_back(std::make_shared<ConcreteMatcher>(matcher));
            }
            return *this;
        }
    };
} // matcher

#endif //RAMPACK_MATCHERALTERNATIVE_H
