//
// Created by Piotr Kubala on 15/12/2022.
//

#include <iomanip>
#include <set>

#include "MatcherAlternative.h"


namespace pyon::matcher {
    std::string MatcherAlternative::generateAlternativeUnmatchedReport(const std::string &reason) const {
        std::ostringstream out;
        out << "Matching Alternative failed:" << std::endl;
        out << "✖ " << reason << std::endl;
        out << "✓ Expected format: " << this->outline(2).substr(2);
        return out.str();
    }

    std::string MatcherAlternative::generateVariantUnmatchedReport(const std::vector<std::string> &reasons,
                                                                   const std::shared_ptr<const ast::Node> &node) const
    {
        std::ostringstream out;
        if (reasons.size() == 1) {
            out << "Matching Alternative failed:" << std::endl;
            out << "✖ " << replaceAll(reasons.back(), "\n", "\n  ");
            return out.str();
        }

        out << "Matching Alternative failed: all " << reasons.size() << " variants of " << this->nameNode(node);
        out << " failed to match:";

        std::size_t numberWidth = std::to_string(reasons.size()).length();
        for (std::size_t i{}; i < reasons.size(); i++) {
            const auto &reason = reasons[i];
            out << std::endl;
            out << "✖ (variant " << std::setw(static_cast<int>(numberWidth)) << std::left << (i + 1) << ") ";
            out << replaceAll(reason, "\n", "\n  ");
        }

        return out.str();
    }

    std::string MatcherAlternative::nameNode(const std::shared_ptr<const ast::Node> &node) const {
        if (node->getType() == ast::Node::DATACLASS)
            return "class \"" + node->as<ast::NodeDataclass>()->getClassName() + "\"";
        else
            return node->getNodeName();
    }

    std::vector<std::shared_ptr<MatcherBase>>
    MatcherAlternative::collectVariants(const std::shared_ptr<const ast::Node> &node) const {
        std::vector<std::shared_ptr<MatcherBase>> variants;
        if (node->getType() == ast::Node::DATACLASS) {
            auto nodeDataclass = node->as<ast::NodeDataclass>();
            const std::string &className = nodeDataclass->getClassName();
            for (const auto &matcherDataclass : this->dataclasses)
                if (matcherDataclass->getName() == className)
                    variants.push_back(matcherDataclass);
        } else {
            // this->alternatives also include dataclasses, but here the node type won't match
            for (const auto &matcher: this->alternatives)
                if (matcher->matchNodeType(node->getType()))
                    variants.push_back(matcher);
        }
        return variants;
    }

    MatchReport MatcherAlternative::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        std::vector<std::shared_ptr<MatcherBase>> variants = this->collectVariants(node);

        if (variants.empty()) {
            if (node->getType() == ast::Node::DATACLASS) {
                const std::string &className = node->as<ast::NodeDataclass>()->getClassName();
                return this->generateAlternativeUnmatchedReport("Got unknown class: \"" + className + "\"");
            } else {
                return this->generateAlternativeUnmatchedReport("Got incorrect node type: " + node->getNodeName());
            }
        }

        std::vector<std::string> variantReasons;
        for (const auto &variant : variants) {
            MatchReport matched = variant->match(node, result);
            if (matched)
                return true;
            else
                variantReasons.push_back(matched.getReason());
        }

        return this->generateVariantUnmatchedReport(variantReasons, node);
    }

    bool MatcherAlternative::matchNodeType(ast::Node::Type type) const {
        auto nodeTypeMatches = [type](const auto &alternative) {
            return alternative->matchNodeType(type);
        };
        return std::any_of(this->alternatives.begin(), this->alternatives.end(), nodeTypeMatches);
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

    std::string MatcherAlternative::synopsis() const {
        if (this->alternatives.empty())
            return "(empty Alternative)";
        return implode(this->collectSynopses(), " | ");
    }

    std::vector<std::string> MatcherAlternative::collectSynopses() const {
        std::vector<std::string> synopses;
        std::set<std::string> seenSynopses;
        for (const auto &matcher : this->alternatives) {
            auto synopsis = matcher->synopsis();
            if (seenSynopses.find(synopsis) == seenSynopses.end()) {
                synopses.push_back(synopsis);
                seenSynopses.insert(synopsis);
            }
        }

        return synopses;
    }
} // matcher