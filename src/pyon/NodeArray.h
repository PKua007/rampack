//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_NODEARRAY_H
#define RAMPACK_NODEARRAY_H

#include <vector>

#include "Node.h"


namespace pyon::ast {
    class ArrayNoSuchIndexException : public ASTException {
    public:
        using ASTException::ASTException;
    };


    class NodeArray : public Node {
    private:
        std::vector<std::shared_ptr<const Node>> elems;

        explicit NodeArray(std::vector<std::shared_ptr<const Node>> elems) : Node(ARRAY), elems{std::move(elems)} { }

    public:
        using iterator = decltype(elems)::const_iterator;

        static constexpr Type NODE_TYPE = ARRAY;

        [[nodiscard]] bool empty() const { return this->elems.empty(); }
        [[nodiscard]] std::size_t size() const { return this->elems.size(); }
        [[nodiscard]] const std::shared_ptr<const Node> &front() const;
        [[nodiscard]] const std::shared_ptr<const Node> &back() const;
        [[nodiscard]] iterator begin() const { return this->elems.begin(); }
        [[nodiscard]] iterator end() const { return this->elems.end(); }
        [[nodiscard]] const std::shared_ptr<const Node> &at(std::size_t idx) const;

        static std::shared_ptr<const NodeArray> create();
        static std::shared_ptr<const NodeArray> create(std::vector<std::shared_ptr<const Node>> elems);
        static std::shared_ptr<const NodeArray> create(std::initializer_list<std::shared_ptr<const Node>> elems);
    };

} // ast

#endif //RAMPACK_NODEARRAY_H
