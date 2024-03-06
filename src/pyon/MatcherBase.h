//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHERBASE_H
#define RAMPACK_MATCHERBASE_H

#include <memory>
#include <string>

#include "Any.h"
#include "Node.h"


namespace pyon::matcher {
    class MatchReport {
    private:
        bool matched{};
        std::string reason;

    public:
        MatchReport() = default;
        MatchReport(bool matched) : matched{matched} { }
        MatchReport(std::string reason) : matched{false}, reason{std::move(reason)} { }

        [[nodiscard]] operator bool() const { return this->matched; }
        [[nodiscard]] bool isMatched() const { return this->matched; }
        [[nodiscard]] const std::string &getReason() const { return this->reason; }
    };

    class MatcherBase {
    public:
        virtual ~MatcherBase() = default;

        virtual MatchReport match(std::shared_ptr<const ast::Node> node, Any &result) const = 0;
        [[nodiscard]] virtual bool matchNodeType(ast::Node::Type type) const = 0;
        [[nodiscard]] virtual std::string outline([[maybe_unused]] std::size_t indent) const = 0;
        [[nodiscard]] virtual std::string synopsis() const = 0;
    };
} // matcher

#endif //RAMPACK_MATCHERBASE_H
