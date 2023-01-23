//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHERNONE_H
#define RAMPACK_MATCHERNONE_H

#include <functional>

#include "MatcherBase.h"
#include "NodeLiteral.h"


namespace pyon::matcher {
    class MatcherNone : public MatcherBase {
        std::function<Any()> mapping = []() { return Any{}; };

    public:
        [[nodiscard]] MatcherNone copy() const { return *this; }

        MatchReport match(std::shared_ptr<const ast::Node> node, Any &result) const override;
        [[nodiscard]] bool matchNodeType(ast::Node::Type type) const override { return type == ast::Node::NONE; }
        [[nodiscard]] std::string outline(std::size_t indent) const override;

        MatcherNone &mapTo(const std::function<Any()> &mapping_);

        template <typename T>
        MatcherNone &mapTo() {
            return this->mapTo([]() -> T { return {}; });
        }
    };
}


#endif //RAMPACK_MATCHERNONE_H
