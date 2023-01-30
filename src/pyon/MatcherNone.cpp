//
// Created by pkua on 10.12.22.
//

#include "MatcherNone.h"


namespace pyon::matcher {
    MatcherNone &MatcherNone::mapTo(const std::function<Any()> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatchReport MatcherNone::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::NONE) {
            std::ostringstream out;
            out << "Matching None failed:" << std::endl;
            out << "✖ Got incorrect node type: " << node->getNodeName();
            return out.str();
        }

        result = this->mapping();
        return true;
    }

    std::string MatcherNone::outline(std::size_t indent) const {
        return std::string(indent, ' ') + "None";
    }
}