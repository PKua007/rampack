//
// Created by Piotr Kubala on 15/12/2022.
//

#include <iomanip>
#include <map>

#include "MatcherAlternative.h"


namespace pyon::matcher {
    std::string MatcherAlternative::generateAlternativeUnmatchedReport(const std::string &reason) const {
        std::ostringstream out;
        out << "Matching Alternative failed:" << std::endl;
        out << "✖ " << reason << std::endl;
        out << "✓ Available alternatives:";
        auto synopses = this->collectSynopses();
        this->presentAlternatives(out, synopses, 2);
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
        auto outlines = this->collectOutlines(indent + numberWidth + 2);
        this->presentAlternatives(out, outlines, indent);

        return out.str();
    }

    std::string MatcherAlternative::synopsis() const {
        if (this->alternatives.empty())
            return "(empty Alternative)";
        return implode(this->collectSynopses(), " | ");
    }

    std::vector<std::string> MatcherAlternative::collectSynopses() const {
        std::vector<std::string> synopses;
        std::map<std::string, std::size_t> seenSynopses;
        for (const auto &matcher : this->alternatives) {
            auto synopsis = matcher->synopsis();
            auto seenSynopsisIt = seenSynopses.find(synopsis);
            if (seenSynopsisIt == seenSynopses.end()) {
                synopses.push_back(synopsis);
                seenSynopses[synopsis] = 1;
            } else {
                seenSynopsisIt->second++;
            }
        }

        for (auto &synopsis : synopses) {
            std::size_t count = seenSynopses[synopsis];
            if (count > 1)
                synopsis += " (" + std::to_string(count) + " variants)";
        }

        return synopses;
    }

    std::vector<std::string> MatcherAlternative::collectOutlines(std::size_t indent) const {
        std::vector<std::string> outlines;
        outlines.reserve(this->alternatives.size());
        auto outlineGetter = [indent](const std::shared_ptr<MatcherBase> &matcher) {
            return matcher->outline(indent).substr(indent);
        };
        std::transform(this->alternatives.begin(), this->alternatives.end(), std::back_inserter(outlines),
                       outlineGetter);
        return outlines;
    }

    void MatcherAlternative::presentAlternatives(std::ostringstream &out, const std::vector<std::string> &alternatives_,
                                                 std::size_t indent) const
    {
        std::string spaces(indent, ' ');
        std::size_t numberWidth = std::to_string(alternatives_.size()).length();
        for (std::size_t i{}; i < alternatives_.size(); i++) {
            const auto &outline = alternatives_[i];
            std::string number = std::to_string(i + 1) + ".";
            out << std::endl << spaces << std::setw(static_cast<int>(numberWidth + 2)) << std::left << number;
            out << outline;
        }
    }
} // matcher