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
    class MatchException : public PyonException {
    public:
        using PyonException::PyonException;
    };

    class MatcherBase {
    public:
        virtual ~MatcherBase() = default;

        virtual bool match(std::shared_ptr<const ast::Node> node, Any &result) const = 0;
        [[nodiscard]] virtual std::string outline([[maybe_unused]] std::size_t indent) const { return ""; }
    };
} // matcher

#endif //RAMPACK_MATCHERBASE_H
