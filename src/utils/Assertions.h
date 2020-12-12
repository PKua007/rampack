//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_ASSERTIONS_H
#define RAMPACK_ASSERTIONS_H

#include <stdexcept>

// Cpp Core Guidelines-style assertions for design by contract
// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i6-prefer-expects-for-expressing-preconditions


#define ASSERTIONS_S(x) #x
#define ASSERTIONS_S_(x) ASSERTIONS_S(x)
#define __WHERE__ __FILE__ ":" ASSERTIONS_S_(__LINE__)

// Preconditions check (argument validation)
#define Expects(cond) if (!(cond)) throw PreconditionException(__WHERE__ ": Precondition (" #cond ") failed")
#define ExpectsMsg(cond, msg) if (!(cond)) throw PreconditionException(__WHERE__ msg)

// Postconditions check (results assertion)
#define Ensures(cond) if (!(cond)) throw PostconditionException(__WHERE__ ": Postcondition (" #cond ") failed")
#define EnsuresMsg(cond, msg) if (!(cond)) throw PostconditionException(__WHERE__ msg)

// Runtime assertion. Why duplicate assert from cassert? Because we don't want to disable is in release mode and
// be more C++ and throw exception
#define Assert(cond) if (!(cond)) throw AssertionException(__WHERE__ ": Assertion (" #cond ") failed")
#define AssertMsg(cond, msg) if (!(cond)) throw AssertionException(__WHERE__ msg)

// Additional macros for validating things like input from file - wrong input shouldn't be considered as assertion
// fail, because it is not the programmer's fault ;)
#define Validate(cond) if (!(cond)) throw ValidationException(__WHERE__ ": Validation (" #cond ") failed")
#define ValidateMsg(cond, msg) if (!(cond)) throw ValidationException(__WHERE__ msg)

/**
 * @brief An exception thrown by Validate and ValidateMsg macros.
 */
struct ValidationException : public std::logic_error {
    explicit ValidationException(const std::string &msg) : std::logic_error{msg} { }
};

/**
 * @brief An exception thrown by Validate and ValidateMsg macros.
 */
struct PreconditionException : public std::logic_error {
    explicit PreconditionException(const std::string &msg) : std::logic_error{msg} { }
};

/**
 * @brief An exception thrown by Validate and ValidateMsg macros.
 */
struct PostconditionException : public std::logic_error {
    explicit PostconditionException(const std::string &msg) : std::logic_error{msg} { }
};

/**
 * @brief An exception thrown by Validate and ValidateMsg macros.
 */
struct AssertionException : public std::logic_error {
    explicit AssertionException(const std::string &msg) : std::logic_error{msg} { }
};

#endif //RAMPACK_ASSERTIONS_H
