//
// Created by pkua on 10.12.22.
//

#include <sstream>

#include "MatcherBoolean.h"


pyon::matcher::MatchReport pyon::matcher::MatcherBoolean::match(std::shared_ptr<const ast::Node> node, Any &result) const {
    if (node->getType() != ast::Node::BOOLEAN)
        return false;

    bool b = node->as<ast::NodeBoolean>()->getValue();
    if (this->filterValue.has_value())
        if (*this->filterValue != b)
            return false;

    result = this->mapping(b);
    return true;
}

pyon::matcher::MatcherBoolean &pyon::matcher::MatcherBoolean::isTrue() {
    this->filterValue = true;
    return *this;
}

pyon::matcher::MatcherBoolean &pyon::matcher::MatcherBoolean::isFalse() {
    this->filterValue = false;
    return *this;
}

pyon::matcher::MatcherBoolean &pyon::matcher::MatcherBoolean::mapTo(const std::function<Any(bool)> &mapping_) {
    this->mapping = mapping_;
    return *this;
}

std::string pyon::matcher::MatcherBoolean::outline(std::size_t indent) const {
    std::string spaces(indent, ' ');
    if (!this->filterValue.has_value())
        return spaces + "Boolean";
    else if (this->filterValue.value())
        return spaces + "Boolean equal True";
    else
        return spaces + "Boolean equal False";
}
