//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_ANY_H
#define RAMPACK_ANY_H

#include <any>
#include <type_traits>

#include "PyonException.h"
#include "Node.h"
#include "utils/Utils.h"


namespace pyon::matcher {
    class AnyCastException : public PyonException {
        using PyonException::PyonException;
    };


    class Any {
    private:
        std::any value;

    public:
        Any() = default;
        Any(const Any&) = default;
        Any(Any&&) noexcept = default;
        Any &operator=(const Any&) = default;
        Any &operator=(Any&&) noexcept = default;

        template<typename T>
        Any(T &&value) : value{std::forward<T>(value)} { }

        template<typename T>
        Any &operator=(T &&value_) noexcept {
            this->value = std::forward<T>(value_);
            return *this;
        }

        template<typename T>
        [[nodiscard]] bool is() const {
            return this->value.type() == typeid(T);
        }

        template<typename T>
        [[nodiscard]] T as() const {
            if (!this->is<T>()) {
                auto nameFrom = demangle(this->value.type().name());
                auto nameTo = demangle(typeid(T).name());
                throw AnyCastException("pyon::Any::as: casting " + nameFrom + " to " + nameTo);
            }
            return std::any_cast<T>(this->value);
        }

        template<typename ConcreteNode>
        std::shared_ptr<const ConcreteNode> asNode() const {
            return this->as<std::shared_ptr<const ast::Node>>()->as<ConcreteNode>();
        }

        void reset() {
            this->value.reset();
        }

        [[nodiscard]] bool isEmpty() const {
            return !this->value.has_value();
        }
    };
} // matcher

#endif //RAMPACK_ANY_H
