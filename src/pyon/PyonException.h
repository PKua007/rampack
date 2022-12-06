//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_PYONEXCEPTION_H
#define RAMPACK_PYONEXCEPTION_H

#include <stdexcept>


namespace pyon {
    class PyonException : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    namespace ast {
        class ASTException : public PyonException {
            using PyonException::PyonException;
        };
    }
} // pyon

#endif //RAMPACK_PYONEXCEPTION_H
