//
// Created by pkua on 23.11.22.
//

#ifndef RAMPACK_VERSION_H
#define RAMPACK_VERSION_H

#include <cstddef>
#include <string>
#include <tuple>
#include <ostream>


/**
 * @brief Utility class representing a semantic version with comparison operators.
 */
class Version {
private:
    std::size_t major_{};
    std::size_t minor_{};
    std::size_t patch_{};

    static std::size_t parseToken(std::string token);

public:
    /**
     * @brief Default constructor assuming version 0.0.0.
     */
    Version() = default;

    /**
     * @brief Constructor for @a major_.0.0 version, where @a major_ can be of any type implicitly convertible to
     * @a std::size_t.
     * @details It is templated in order to prevent overload resolution from using Version(const char*) constructor for
     * some integral types such as @a int.
     */
    template<typename MAJOR_T>
    constexpr Version(MAJOR_T major_)
            : major_(major_)      // () to allow narrowing conversion
    { }

    /**
     * @brief Constructor for @a major_.@a minor_.0 version.
     */
    constexpr Version(std::size_t major_, std::size_t minor_)
            : major_{major_}, minor_{minor_}
    { }

    /**
     * @brief Constructor for @a major_.@a minor_.@a patch_ version.
     */
    constexpr Version(std::size_t major_, std::size_t minor_, std::size_t patch_)
            : major_{major_}, minor_{minor_}, patch_{patch_}
    { }

    /**
     * @brief Constructor parsing @a versionStr of the form "1", "1.0", "1.1", "2.3.1", etc.
     */
    Version(const std::string &versionStr);

    /**
     * @brief The same as Version(const std::string &).
     */
    Version(const char *versionStr) : Version(std::string{versionStr}) { }

    friend bool operator==(const Version &lhs, const Version &rhs) {
        return std::tie(lhs.major_, lhs.minor_, lhs.patch_) == std::tie(rhs.major_, rhs.minor_, rhs.patch_);
    }

    friend bool operator!=(const Version &lhs, const Version &rhs) {
        return !(rhs == lhs);
    }

    friend bool operator<(const Version &lhs, const Version &rhs) {
        return std::tie(lhs.major_, lhs.minor_, lhs.patch_) < std::tie(rhs.major_, rhs.minor_, rhs.patch_);
    }

    friend bool operator>(const Version &lhs, const Version &rhs) {
        return rhs < lhs;
    }

    friend bool operator<=(const Version &lhs, const Version &rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>=(const Version &lhs, const Version &rhs) {
        return !(lhs < rhs);
    }

    friend std::ostream &operator<<(std::ostream &os, const Version &version);

    [[nodiscard]] std::string str() const;

    [[nodiscard]] constexpr std::size_t getMajor() const { return this->major_; }
    [[nodiscard]] constexpr std::size_t getMinor() const { return this->minor_; }
    [[nodiscard]] constexpr std::size_t getPatch() const { return this->patch_; }
};


constexpr Version CURRENT_VERSION{0, 9, 3};

// Auxiliary versions user in the code

/** Version when input file format was rewritten */
constexpr Version INPUT_REVAMP_VERSION{0, 9, 0};

/** Version when shape centers and orientation were made consistent */
constexpr Version CONSISTENT_SHAPES_VERSION{0, 2, 0};


#endif //RAMPACK_VERSION_H
