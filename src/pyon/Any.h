//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_ANY_H
#define RAMPACK_ANY_H

#include <any>
#include <type_traits>

#include "PyonException.h"


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
        Any &operator=(T &&value_) noexcept { this->value = std::forward<T>(value_); }

        template<typename T>
        bool is() {
            return this->value.type() == typeid(T);
        }

        template<typename T>
        T as() {
            if (!this->is<T>())
                throw AnyCastException("pyon::Any::as: bad any cast");
            return std::any_cast<T>(this->value);
        }
    };
} // matcher

#endif //RAMPACK_ANY_H
