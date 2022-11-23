//
// Created by pkua on 23.11.22.
//

#ifndef RAMPACK_VERSION_H
#define RAMPACK_VERSION_H

#include <cstddef>
#include <string>
#include <tuple>
#include <ostream>


class Version {
private:
    std::size_t major{};
    std::size_t minor{};
    std::size_t patch{};

    static std::size_t parseToken(std::string token);

public:
    Version() = default;

    constexpr Version(std::size_t major, std::size_t minor = 0, std::size_t patch = 0)
            : major{major}, minor{minor}, patch{patch}
    { }

    Version(const std::string &versionStr);

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
};


#endif //RAMPACK_VERSION_H
