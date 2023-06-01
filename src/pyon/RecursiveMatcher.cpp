//
// Created by Piotr Kubala on 01/06/2023.
//

#include "RecursiveMatcher.h"

namespace pyon::matcher {
    using namespace pyon::ast;

    MatchReport RecursiveMatcher::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (this->container->theMatcherWeak.expired()) {
            throw UninitializedRecursiveMatcherException("RecursiveMatcher::attach() was never called prior to "
                                                         "::match()");
        }
        auto theMatcher = this->container->theMatcherWeak.lock();
        return theMatcher->match(node, result);
    }

    bool RecursiveMatcher::matchNodeType(Node::Type type) const {
        if (this->container->theMatcherWeak.expired()) {
            throw UninitializedRecursiveMatcherException("RecursiveMatcher::attach() was never called prior to "
                                                         "::matchNodeType()");
        }
        auto theMatcher = this->container->theMatcherWeak.lock();
        return theMatcher->matchNodeType(type);
    }

    std::string RecursiveMatcher::outline(std::size_t indent) const {
        if (this->container->theMatcherWeak.expired()) {
            throw UninitializedRecursiveMatcherException("RecursiveMatcher::attach() was never called prior to "
                                                         "::outline()");
        }

        auto theMatcher = this->container->theMatcherWeak.lock();
        std::string spaces(indent, ' ');
        std::ostringstream out;
        out << spaces << "{recursion on (" << theMatcher->synopsis() << ")}";
        return out.str();
    }

    std::string pyon::matcher::RecursiveMatcher::synopsis() const {
        if (this->container->theMatcherWeak.expired()) {
            throw UninitializedRecursiveMatcherException("RecursiveMatcher::attach() was never called prior to "
                                                         "::synopsis()");
        }

        return "{recursion}";
    }
}
