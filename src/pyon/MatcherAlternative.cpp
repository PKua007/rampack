//
// Created by Piotr Kubala on 15/12/2022.
//

#include <iomanip>

#include "MatcherAlternative.h"


namespace pyon::matcher {
    MatchReport MatcherAlternative::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        for (const auto &alternative : this->alternatives)
            if (alternative->match(node, result))
                return true;

        return false;
    }

    std::string MatcherAlternative::outline(std::size_t indent) const {
        std::string spaces(indent, ' ');
        std::ostringstream out;

        out << spaces << "Alternative:";
        std::size_t numberWidth = std::to_string(this->alternatives.size()).length();
        for (std::size_t i{}; i < this->alternatives.size(); i++) {
            const MatcherBase &alternative = *this->alternatives[i];
            std::string alternativeOutline = alternative.outline(indent + numberWidth + 2);
            alternativeOutline = alternativeOutline.substr(indent + numberWidth + 2);
            std::string number = std::to_string(i + 1) + ".";
            out << std::endl << spaces << std::setw(static_cast<int>(numberWidth + 2)) << std::left << number;
            out << alternativeOutline;
        }

        return out.str();
    }
} // matcher