//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_NODELITERAL_H
#define RAMPACK_NODELITERAL_H


#include <string>
#include <utility>

#include "Node.h"


namespace pyon::ast {
    class NodeInt : public Node {
    private:
        long int value{};

        explicit NodeInt(long int value) : Node(INT), value{value} { }

    public:
        static constexpr Type NODE_TYPE = INT;

        [[nodiscard]] long int getValue() const { return this->value; }

        static std::shared_ptr<const NodeInt> create(unsigned long int value);
    };


    class NodeFloat : public Node {
    private:
        double value{};

        explicit NodeFloat(double value) : Node(FLOAT), value{value} { }

    public:
        static constexpr Type NODE_TYPE = FLOAT;

        [[nodiscard]] double getValue() const { return this->value; }

        static std::shared_ptr<const NodeFloat> create(double value);
    };


    class NodeBoolean : public Node {
    private:
        bool value{};

        explicit NodeBoolean(bool value) : Node(BOOLEAN), value{value} { }

    public:
        static constexpr Type NODE_TYPE = BOOLEAN;

        [[nodiscard]] bool getValue() const { return this->value; }

        static std::shared_ptr<const NodeBoolean> create(bool value);
    };


    class NodeString : public Node {
    private:
        std::string value{};

        explicit NodeString(std::string value) : Node(STRING), value{std::move(value)} { }

    public:
        static constexpr Type NODE_TYPE = STRING;

        [[nodiscard]] const std::string &getValue() const { return this->value; }

        static std::shared_ptr<const NodeString> create(std::string value);
    };


    class NodeNone : public Node {
    private:
        NodeNone() : Node(NONE) { }

    public:
        static constexpr Type NODE_TYPE = NONE;

        static std::shared_ptr<const NodeNone> create();
    };
} // ast

#endif //RAMPACK_NODELITERAL_H
