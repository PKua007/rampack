//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_FOLD_H
#define RAMPACK_FOLD_H


#include <sstream>
#include <utility>

/**
 * @brief A simple class for folding long strings into multiline.
 * @details Usage:
 *
 * \code
 * std::cout << Fold("Long string without a margin").width(10) << std::endl;
 * \endcode
 *
 * <pre>
 * |Long      |
 * |string    |
 * |without a |
 * |margin    |
 * </pre>
 *
 * \code
 * std::cout << Fold("Long string with a margin").width(10).margin(4) << std::endl;
 * \endcode
 *
 * <pre>
 * |    Long  |
 * |    string|
 * |    with a|
 * |    margin|
 * </pre>
 */
class Fold {
private:
    std::size_t width_{};
    std::size_t margin_{};
    std::string str;

    [[nodiscard]] std::size_t findEnterInCurrentLine(std::size_t lineBeg, std::size_t effectiveWidth) const;
    [[nodiscard]] std::size_t findLastSpaceInCurrentLine(std::size_t lineBeg, std::size_t effectiveWidth) const;
    [[nodiscard]] bool spaceOnBreak(std::size_t lineBeg, std::size_t effectiveWidth) const;
    [[nodiscard]] bool notLastLine(std::size_t lineBeg, std::size_t effectiveWidth) const;

public:
    explicit Fold(std::string str) : str{std::move(str)} { }
    Fold() : str{""} { }

    Fold &width(std::size_t width_);
    Fold &margin(std::size_t margin_);
    operator std::string() const;
};

std::ostream &operator<<(std::ostream &out, const Fold &fold);


#endif //RAMPACK_FOLD_H
