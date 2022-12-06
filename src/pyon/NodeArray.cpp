//
// Created by pkua on 06.12.22.
//

#include <sstream>

#include "NodeArray.h"
#include "utils/Assertions.h"


namespace pyon::ast {
    const std::shared_ptr<const Node> &NodeArray::front() const {
        if (this->empty())
            throw ArrayNoSuchIndexException("pyon::ast::NodeArray::front: empty array");
        return this->elems.front();
    }

    const std::shared_ptr<const Node> &NodeArray::back() const {
        if (this->empty())
            throw ArrayNoSuchIndexException("pyon::ast::NodeArray::back: empty array");
        return this->elems.back();
    }

    const std::shared_ptr<const Node> &NodeArray::at(std::size_t idx) const {
        if (idx >= this->size()) {
            std::ostringstream msg;
            msg << "pyon::ast::NodeArray::at: trying to access index " << idx << ", but array's size is ";
            msg << this->size();
            throw ArrayNoSuchIndexException(msg.str());
        }
        return this->elems[idx];
    }

    std::shared_ptr<const NodeArray> NodeArray::create(std::vector<std::shared_ptr<const Node>> elems) {
        for (const auto &elem : elems)
            Expects(elem != nullptr);

        return std::shared_ptr<const NodeArray>(new NodeArray(std::move(elems)));
    }

    std::shared_ptr<const NodeArray> NodeArray::create(std::initializer_list<std::shared_ptr<const Node>> elems) {
        return NodeArray::create(std::vector<std::shared_ptr<const Node>>{elems});
    }

    std::shared_ptr<const NodeArray> NodeArray::create() {
        return NodeArray::create({});
    }
} // ast