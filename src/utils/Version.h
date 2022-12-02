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
    std::size_t major{};
    std::size_t minor{};
    std::size_t patch{};

    static std::size_t parseToken(std::string token);

public:
    /**
     * @brief Default constructor assuming version 0.0.0.
     */
    Version() = default;

    /**
     * @brief Constructor for @a major.0.0 version, where @a major can be of any type implicitly convertible to
     * @a std::size_t.
     * @details It is templated in order to prevent overload resolution from using Version(const char*) constructor for
     * some integral types such as @a int.
     */
    template<typename MAJOR_T>
    constexpr Version(MAJOR_T major)
            : major(major)      // () to allow narrowing conversion
    { }

    /**
     * @brief Constructor for @a major.@a minor.0 version.
     */
    constexpr Version(std::size_t major, std::size_t minor)
            : major{major}, minor{minor}
    { }

    /**
     * @brief Constructor for @a major.@a minor.@a patch version.
     */
    constexpr Version(std::size_t major, std::size_t minor, std::size_t patch)
            : major{major}, minor{minor}, patch{patch}
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
        return std::tie(lhs.major, lhs.minor, lhs.patch) == std::tie(rhs.major, rhs.minor, rhs.patch);
    }

    friend bool operator!=(const Version &lhs, const Version &rhs) {
        return !(rhs == lhs);
    }

    friend bool operator<(const Version &lhs, const Version &rhs) {
        return std::tie(lhs.major, lhs.minor, lhs.patch) < std::tie(rhs.major, rhs.minor, rhs.patch);
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

    [[nodiscard]] constexpr std::size_t getMajor() const { return this->major; }
    [[nodiscard]] constexpr std::size_t getMinor() const { return this->minor; }
    [[nodiscard]] constexpr std::size_t getPatch() const { return this->patch; }
};


constexpr Version CURRENT_VERSION{0, 3, 0};


#endif //RAMPACK_VERSION_H
