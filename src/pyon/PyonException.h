//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_PYONEXCEPTION_H
#define RAMPACK_PYONEXCEPTION_H

#include "utils/Exceptions.h"


namespace pyon {
    class PyonException : public ValidationException {
    public:
        using ValidationException::ValidationException;
    };

    namespace ast {
        class ASTException : public PyonException {
            using PyonException::PyonException;
        };
    }
} // pyon

#endif //RAMPACK_PYONEXCEPTION_H
