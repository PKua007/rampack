//
// Created by pkua on 10.12.22.
//

#include <sstream>
#include <optional>

#include "MatcherArray.h"
#include "utils/Assertions.h"


namespace pyon::matcher {
    const Any &ArrayData::front() const {
        if (this->empty())
            throw ast::ArrayNoSuchIndexException("pyon::matcher::ArrayData::front: empty array");
        return this->data.front();
    }

    const Any &ArrayData::back() const {
        if (this->empty())
            throw ast::ArrayNoSuchIndexException("pyon::matcher::ArrayData::back: empty array");
        return this->data.back();
    }

    const Any &ArrayData::at(std::size_t idx) const {
        if (idx >= this->size()) {
            std::ostringstream msg;
            msg << "pyon::matcher::ArrayData::at: trying to access index " << idx << ", but array's size is ";
            msg << this->size();
            throw ast::ArrayNoSuchIndexException(msg.str());
        }
        return this->data[idx];
    }

    ArrayData::ArrayData(std::vector<Any> data) {
        // We use swap since move construction
        std::swap(this->data, data);
    }

    std::string MatcherArray::generateArrayUnmatchedReport(const std::string &reason) const {
        std::ostringstream out;
        out << "Matching Array failed:" << std::endl;
        out << "✖ " << reason << std::endl;
        out << "✓ Expected format: " << this->outline(2).substr(2);
        return out.str();
    }

    std::string MatcherArray::generateIndexUnmatchedReport(std::size_t index, const std::string &reason) const {
        std::ostringstream out;
        out << "Matching Array failed: Matching index " << index << " failed:" << std::endl;
        out << "✖ " << replaceAll(reason, "\n", "\n  ");
        return out.str();
    }

    MatcherArray::MatcherArray(std::size_t size_) {
        this->size(size_);
    }

    MatcherArray::MatcherArray(int size_) {
        this->size(size_);
    }

    MatchReport MatcherArray::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::ARRAY)
            return this->generateArrayUnmatchedReport("Got incorrect node type: " + node->getNodeName());

        const auto &nodeArray = node->as<ast::NodeArray>();
        std::vector<Any> arrayDataVec;
        if (this->elementMatcher == nullptr) {
            arrayDataVec.assign(nodeArray->begin(), nodeArray->end());
        } else {
            arrayDataVec.reserve(nodeArray->size());
            for (std::size_t i{}; i < nodeArray->size(); i++) {
                const auto &nodeArrayElem = nodeArray->at(i);
                Any elemResult;
                auto matchReport = this->elementMatcher->match(nodeArrayElem, elemResult);
                if (!matchReport)
                    return this->generateIndexUnmatchedReport(i, matchReport.getReason());
                arrayDataVec.push_back(elemResult);
            }
        }
        ArrayData arrayData(std::move(arrayDataVec));

        for (const auto &filter: this->filters)
            if (!filter.predicate(arrayData))
                return this->generateArrayUnmatchedReport("Condition not satisfied: " + filter.description);

        result = this->mapping(arrayData);
        return true;
    }

    std::string MatcherArray::outline(std::size_t indent) const {
        std::size_t linesAtLeast{};
        std::optional<std::string> elementOutline;
        if (this->elementMatcher != nullptr) {
            elementOutline = this->elementMatcher->outline(indent + 2);
            elementOutline = "with elements: " + elementOutline->substr(indent + 2);
            linesAtLeast++;
            if (isMultiline(*elementOutline))
                linesAtLeast++;
        }

        linesAtLeast += this->filters.size();

        std::ostringstream out;
        std::string spaces(indent, ' ');
        if (linesAtLeast == 0)
            out << spaces << "Array";
        else if (linesAtLeast == 1)
            out << spaces << "Array, ";
        else
            out << spaces << "Array:";

        if (elementOutline.has_value()) {
            if (linesAtLeast >= 2)
                out << std::endl << spaces << "- ";
            out << *elementOutline;
        }

        for (const auto &filter : this->filters) {
            if (linesAtLeast >= 2)
                out << std::endl << spaces << "- ";
            out << filter.description;
        }

        return out.str();
    }

    MatcherArray &MatcherArray::mapTo(const std::function<Any(const ArrayData&)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherArray &MatcherArray::mapToDefault() {
        this->mapping = [](const ArrayData &array) { return array; };
        return *this;
    }

    MatcherArray &MatcherArray::filter(const std::function<bool(const ArrayData&)> &filter) {
        this->filters.push_back({filter, "<undefined filter>"});
        return *this;
    }

    MatcherArray &MatcherArray::describe(const std::string &description) {
        Expects(!this->filters.empty());
        this->filters.back().description = description;
        return *this;
    }

    MatcherArray &MatcherArray::size(std::size_t size_) {
        this->filter([size_](const ArrayData &array) { return array.size() == size_; });
        this->describe("with size = " + std::to_string(size_));
        return *this;
    }

    MatcherArray &MatcherArray::sizeAtLeast(std::size_t size_) {
        this->filter([size_](const ArrayData &array) { return array.size() >= size_; });
        this->describe("with size >= " + std::to_string(size_));
        return *this;
    }

    MatcherArray &MatcherArray::sizeAtMost(std::size_t size_) {
        this->filter([size_](const ArrayData &array) { return array.size() <= size_; });
        this->describe("with size <= " + std::to_string(size_));
        return *this;
    }

    MatcherArray &MatcherArray::sizeInRange(std::size_t low, std::size_t high) {
        Expects(low <= high);
        this->filter([low, high](const ArrayData &array) {
            return array.size() >= low && array.size() <= high;
        });
        this->describe("with size in range [" + std::to_string(low) + ", " + std::to_string(high) + "]");
        return *this;
    }

    MatcherArray &MatcherArray::empty() {
        this->filter([](const ArrayData &array) { return array.empty(); });
        this->describe("empty");
        return *this;
    }

    MatcherArray &MatcherArray::nonEmpty() {
        this->filter([](const ArrayData &array) { return !array.empty(); });
        this->describe("non-empty");
        return *this;
    }

    std::string MatcherArray::synopsis() const {
        if (this->elementMatcher == nullptr)
            return "Array[any expression]";
        else
            return "Array[" + this->elementMatcher->synopsis() + "]";
    }
} // matcher