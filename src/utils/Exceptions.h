//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_EXCEPTIONS_H
#define RAMPACK_EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <cstring>


#define EXCEPTIONS_BLOCK(expr) do { expr } while(false)

// Cpp Core Guidelines-style assertions for design by contract
// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i6-prefer-expects-for-expressing-preconditions

// Preconditions check (argument validation)
#define Expects(cond) EXCEPTIONS_BLOCK(                                                                             \
    if (!(cond))                                                                                                    \
        throw PreconditionException(__FILE__, __func__, __LINE__, #cond, "Precondition failed");                    \
)

#define ExpectsMsg(cond, msg) EXCEPTIONS_BLOCK(                                                                     \
    if (!(cond))                                                                                                    \
        throw PreconditionException(__FILE__, __func__, __LINE__, #cond, msg);                                      \
)

#define ExpectsThrow(msg)   throw PreconditionException(__FILE__, __func__, __LINE__, "Manual throw", msg)


// Postconditions check (results assertion)
#define Ensures(cond) EXCEPTIONS_BLOCK(                                                                             \
    if (!(cond))                                                                                                    \
        throw PostconditionException(__FILE__, __func__, __LINE__, #cond, "Postcondition failed");                  \
)

#define EnsuresMsg(cond, msg) EXCEPTIONS_BLOCK(                                                                     \
    if (!(cond))                                                                                                    \
        throw PostconditionException(__FILE__, __func__, __LINE__, #cond, msg);                                     \
)

#define EnsuresThrow(msg)   throw PostconditionException(__FILE__, __func__, __LINE__, "Manual throw", msg)


// Runtime assertion
#define Assert(cond) EXCEPTIONS_BLOCK(                                                                              \
    if (!(cond))                                                                                                    \
        throw AssertionException(__FILE__, __func__, __LINE__, #cond, "Assertion failed");                          \
)

#define AssertMsg(cond, msg) EXCEPTIONS_BLOCK(                                                                      \
    if (!(cond))                                                                                                    \
        throw AssertionException(__FILE__, __func__, __LINE__, #cond, msg);                                         \
)

#define AssertThrow(msg)   throw AssertionException(__FILE__, __func__, __LINE__, "Manual throw", msg)


// Additional macros for validating user input throwing InputError
#define ValidateMsg(cond, msg) EXCEPTIONS_BLOCK(                                                                    \
    if (!(cond))                                                                                                    \
        throw InputError(msg);                                                                                      \
)

#define ValidateOpenedDesc(stream, filename, desc) EXCEPTIONS_BLOCK(                                                \
    if (!(stream))                                                                                                  \
        throw InputError("Could not open '" + (filename) + "' " + (desc) + ": " + strerror(errno));                 \
)


struct RampackException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct InputError : public RampackException {
    using RampackException::RampackException;
};

struct InternalError : public RampackException {
    using RampackException::RampackException;
};

struct ContractException : public InternalError {
private:
    std::string file;
    std::string function;
    std::size_t line{};
    std::string condition;
    std::string message;

    static std::string makeWhat(const std::string &file_, const std::string &function_, std::size_t line_,
                                const std::string &condition_, const std::string &message_);

public:
    ContractException(std::string file, std::string function, std::size_t line, std::string condition,
                      std::string message)
            : InternalError(ContractException::makeWhat(file, function, line, condition, message)),
              file{std::move(file)}, function{std::move(function)}, line{line}, condition{std::move(condition)},
              message{std::move(message)}
    { }
};

/**
 * @brief An exception thrown by Validate and ValidateMsg macros.
 */
struct PreconditionException : public ContractException {
    explicit PreconditionException(std::string file, std::string function, std::size_t line, std::string condition,
                                   std::string message)
            : ContractException(std::move(file), std::move(function), line, std::move(condition), std::move(message))
    { }
};

/**
 * @brief An exception thrown by Validate and ValidateMsg macros.
 */
struct PostconditionException : public ContractException {
    explicit PostconditionException(std::string file, std::string function, std::size_t line, std::string condition,
                                    std::string message)
            : ContractException(std::move(file), std::move(function), line, std::move(condition), std::move(message))
    { }
};

/**
 * @brief An exception thrown by Validate and ValidateMsg macros.
 */
struct AssertionException : public ContractException {
    explicit AssertionException(std::string file, std::string function, std::size_t line, std::string condition,
                                std::string message)
            : ContractException(std::move(file), std::move(function), line, std::move(condition), std::move(message))
    { }
};

#endif //RAMPACK_EXCEPTIONS_H
