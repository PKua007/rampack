//
// Created by Piotr Kubala on 15/12/2022.
//

#ifndef RAMPACK_MATCHERALTERNATIVE_H
#define RAMPACK_MATCHERALTERNATIVE_H

#include <vector>

#include "MatcherBase.h"
#include "MatcherDataclass.h"


namespace pyon::matcher {
    class MatcherAlternative : public MatcherBase {
    private:
        std::vector<std::shared_ptr<MatcherDataclass>> dataclasses;
        std::vector<std::shared_ptr<MatcherBase>> alternatives;

        [[nodiscard]] std::string generateAlternativeUnmatchedReport(const std::string &reason) const;
        [[nodiscard]] std::string generateVariantUnmatchedReport(const std::vector<std::string> &reasons,
                                                                 const std::shared_ptr<const ast::Node> &node) const;
        [[nodiscard]] std::string nameNode(const std::shared_ptr<const ast::Node> &node) const;
        [[nodiscard]] std::vector<std::shared_ptr<MatcherBase>>
        collectVariants(const std::shared_ptr<const ast::Node> &node) const;


    public:
        MatcherAlternative() = default;

        [[nodiscard]] MatcherAlternative copy() const { return *this; }

        MatchReport match(std::shared_ptr<const ast::Node> node, Any &result) const override;
        [[nodiscard]] bool matchNodeType(ast::Node::Type type) const override;
        [[nodiscard]] std::string outline(std::size_t indent) const override;

        template<typename ConcreteMatcher>
        MatcherAlternative &operator|=(const ConcreteMatcher &matcher) {
            static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher>,
                          "ConcreteMatcher template parameter is not a matcher derived from MatcherBase");

            if constexpr (std::is_same_v<MatcherAlternative, std::decay_t<ConcreteMatcher>>) {
                this->dataclasses.insert(this->dataclasses.end(),
                                          matcher.dataclasses.begin(), matcher.dataclasses.end());
                this->alternatives.insert(this->alternatives.end(),
                                          matcher.alternatives.begin(), matcher.alternatives.end());
            } else if constexpr (std::is_same_v<MatcherDataclass, std::decay_t<ConcreteMatcher>>) {
                this->dataclasses.push_back(std::make_shared<MatcherDataclass>(matcher));
                // Add also to alternatives for easier implementation of outline()
                this->alternatives.push_back(std::make_shared<MatcherDataclass>(matcher));
            } else {
                this->alternatives.push_back(std::make_shared<ConcreteMatcher>(matcher));
            }
            return *this;
        }

    };
} // matcher

#endif //RAMPACK_MATCHERALTERNATIVE_H
