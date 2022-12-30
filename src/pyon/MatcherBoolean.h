//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHERBOOLEAN_H
#define RAMPACK_MATCHERBOOLEAN_H

#include <functional>
#include <optional>

#include "MatcherBase.h"
#include "NodeLiteral.h"


namespace pyon::matcher {
    class MatcherBoolean : public MatcherBase {
    private:
        std::function<Any(bool)> mapping = [](bool b) { return b; };
        std::optional<bool> filterValue;

        [[nodiscard]] std::string generateUnmatchedReport(const std::string &reason) const;

    public:
        MatcherBoolean() = default;
        explicit MatcherBoolean(bool b) : filterValue{b} { }

        [[nodiscard]] MatcherBoolean copy() const { return *this; }

        MatchReport match(std::shared_ptr<const ast::Node> node, Any &result) const override;
        [[nodiscard]] bool matchNodeType(ast::Node::Type type) const override { return type == ast::Node::BOOLEAN; }
        [[nodiscard]] std::string outline(std::size_t indent) const override;

        MatcherBoolean &isTrue();
        MatcherBoolean &isFalse();

        template<typename T>
        MatcherBoolean &mapTo() {
            this->mapping = [](bool b) { return static_cast<T>(b); };
            return *this;
        }

        MatcherBoolean &mapTo(const std::function<Any(bool)> &mapping_);
    };
} // matcher

#endif //RAMPACK_MATCHERBOOLEAN_H
