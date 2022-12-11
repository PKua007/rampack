//
// Created by pkua on 10.12.22.
//

#include <sstream>

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

    MatcherArray::MatcherArray(std::size_t size_) {
        this->size(size_);
    }

    bool MatcherArray::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::ARRAY)
            return false;

        const auto &nodeArray = node->as<ast::NodeArray>();
        std::vector<Any> arrayDataVec;
        if (this->elementMatcher == nullptr) {
            arrayDataVec.assign(nodeArray->begin(), nodeArray->end());
        } else {
            arrayDataVec.reserve(nodeArray->size());
            for (const auto &nodeArrayElem: *nodeArray) {
                Any elemResult;
                if (!this->elementMatcher->match(nodeArrayElem, elemResult))
                    return false;
                arrayDataVec.push_back(elemResult);
            }
        }
        ArrayData arrayData(std::move(arrayDataVec));

        for (const auto &filter: this->filters)
            if (!filter(arrayData))
                return false;

        result = this->mapping(arrayData);
        return true;
    }

    MatcherArray &MatcherArray::mapTo(const std::function<Any(const ArrayData&)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherArray &MatcherArray::filter(const std::function<bool(const ArrayData&)> &filter) {
        this->filters.push_back(filter);
        return *this;
    }

    MatcherArray &MatcherArray::size(std::size_t size_) {
        this->filters.emplace_back([size_](const ArrayData &array) { return array.size() == size_; });
        return *this;
    }

    MatcherArray &MatcherArray::sizeAtLeast(std::size_t size_) {
        this->filters.emplace_back([size_](const ArrayData &array) { return array.size() >= size_; });
        return *this;
    }

    MatcherArray &MatcherArray::sizeAtMost(std::size_t size_) {
        this->filters.emplace_back([size_](const ArrayData &array) { return array.size() <= size_; });
        return *this;
    }

    MatcherArray &MatcherArray::sizeInRange(std::size_t low, std::size_t high) {
        Expects(low <= high);
        this->filters.emplace_back([low, high](const ArrayData &array) {
            return array.size() >= low && array.size() <= high;
        });
        return *this;
    }

    MatcherArray &MatcherArray::empty() {
        this->filters.emplace_back([](const ArrayData &array) { return array.empty(); });
        return *this;
    }
} // matcher