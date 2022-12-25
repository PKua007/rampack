//
// Created by pkua on 10.12.22.
//

#include <sstream>

#include "MatcherBoolean.h"


std::string pyon::matcher::MatcherBoolean::generateUnmatchedReport(const std::string &reason) const {
    std::ostringstream out;
    out << "Matching Boolean failed:" << std::endl;
    out << "✖ " << reason << std::endl;
    out << "✓ Expected format: " << this->outline(0);
    return out.str();
}

pyon::matcher::MatchReport pyon::matcher::MatcherBoolean::match(std::shared_ptr<const ast::Node> node, Any &result) const {
    if (node->getType() != ast::Node::BOOLEAN)
        return this->generateUnmatchedReport("Got incorrect node type: " + node->getNodeName());

    bool b = node->as<ast::NodeBoolean>()->getValue();
    if (this->filterValue.has_value())
        if (*this->filterValue != b)
            return this->generateUnmatchedReport(b ? "Boolean is True" : "Boolean is False");

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
