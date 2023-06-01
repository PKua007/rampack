//
// Created by Piotr Kubala on 01/06/2023.
//

#ifndef RAMPACK_RECURSIVEMATCHER_H
#define RAMPACK_RECURSIVEMATCHER_H

#include <memory>

#include "MatcherBase.h"


namespace pyon::matcher {
    class UninitializedRecursiveMatcherException : public MatchException {
        using MatchException::MatchException;
    };


    class RecursiveMatcher : public MatcherBase {
    private:
        struct MatcherContainer {
            std::weak_ptr<MatcherBase> theMatcherWeak;
        };

        std::shared_ptr<MatcherContainer> container = std::make_shared<MatcherContainer>();
        std::shared_ptr<MatcherBase> theMatcherStrong;

    public:
        MatchReport match(std::shared_ptr<const ast::Node> node, Any &result) const override;
        [[nodiscard]] bool matchNodeType(ast::Node::Type type) const override;
        [[nodiscard]] std::string outline(std::size_t indent) const override;
        [[nodiscard]] std::string synopsis() const override;

        template<typename ConcreteMatcher>
        void attach(const ConcreteMatcher &matcher) {
            static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher>,
                          "ConcreteMatcher template parameter is not a matcher derived from MatcherBase");

            auto theMatcher = std::make_shared<ConcreteMatcher>(matcher);
            this->theMatcherStrong = theMatcher;
            this->container->theMatcherWeak = theMatcher;
        }
    };

}

#endif //RAMPACK_RECURSIVEMATCHER_H
