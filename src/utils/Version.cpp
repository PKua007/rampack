//
// Created by pkua on 23.11.22.
//

#include "Version.h"
#include "Utils.h"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

Version::Version(const std::string &versionStr) {
    auto tokens = explode(versionStr, '.');
    Expects(!tokens.empty() && tokens.size() <= 3);
    switch (tokens.size()) {
        case 3:
            this->patch_ = Version::parseToken(tokens[2]);
        case 2:
            this->minor_ = Version::parseToken(tokens[1]);
        case 1:
            this->major_ = Version::parseToken(tokens[0]);
            break;
        default:
            throw std::runtime_error("");
    }
}

#pragma GCC diagnostic pop   // -Wimplicit-fallthrough


std::size_t Version::parseToken(std::string token) {
    trim(token);
    Expects(!token.empty());
    std::size_t idx;
    std::size_t n;
    try {
        n = std::stoul(token, &idx);
    } catch (const std::invalid_argument &) {
        ExpectsThrow("Malformed version token");
    }
    Expects(idx == token.length());
    return n;
}

std::string Version::str() const {
    std::ostringstream out;
    out << *this;
    return out.str();
}

std::ostream &operator<<(std::ostream &os, const Version &version) {
    return os << version.major_ << "." << version.minor_ << "." << version.patch_;
}
