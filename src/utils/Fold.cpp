//
// Created by Piotr Kubala on 12/12/2020.
//

#include "Fold.h"
#include "Exceptions.h"

Fold::operator std::string() const {
    Expects(width_ > 1);
    Expects( margin_ < width_);
    if (this->str.empty())
        return "";

    std::ostringstream formatted;
    std::size_t lineBeg{};

    std::string marginSpaces = std::string(this->margin_, ' ');
    std::size_t effectiveWidth = this->width_ - this->margin_;

    while (this->notLastLine(lineBeg, effectiveWidth)) {
        std::size_t newline = this->findEnterInCurrentLine(lineBeg, effectiveWidth);
        std::size_t spacePos = this->findLastSpaceInCurrentLine(lineBeg, effectiveWidth);
        if (newline != std::string::npos) {
            formatted << marginSpaces << this->str.substr(lineBeg, newline - lineBeg + 1);
            lineBeg = newline + 1;
        } else if (this->spaceOnBreak(lineBeg, effectiveWidth)) {
            formatted << marginSpaces << this->str.substr(lineBeg, effectiveWidth) << '\n';
            lineBeg = lineBeg + effectiveWidth + 1;
        } else if (spacePos == std::string::npos) {
            // too long line to break nicely
            formatted << marginSpaces << this->str.substr(lineBeg, effectiveWidth) << '\n';
            lineBeg = lineBeg + effectiveWidth;
        } else {
            // found space to break
            formatted << marginSpaces << this->str.substr(lineBeg, spacePos - lineBeg + 1) << '\n';
            lineBeg = spacePos + 1;
        }
    }

    if (lineBeg < this->str.size())
        formatted << marginSpaces << this->str.substr(lineBeg);
    return formatted.str();
}

Fold &Fold::width(std::size_t width_) {
    this->width_ = width_;
    return *this;
}

Fold &Fold::margin(std::size_t margin_) {
    this->margin_ = margin_;
    return *this;
}

std::size_t Fold::findEnterInCurrentLine(std::size_t lineBeg, std::size_t effectiveWidth) const {
    std::size_t newline = this->str.find('\n', lineBeg);
    if (newline != std::string::npos && newline - lineBeg >= effectiveWidth)
        return std::string::npos;
    return newline;
}

std::size_t Fold::findLastSpaceInCurrentLine(std::size_t lineBeg, std::size_t effectiveWidth) const {
    std::size_t spacePos = this->str.rfind(' ', lineBeg + effectiveWidth - 1);
    if (spacePos != std::string::npos && spacePos < lineBeg)
        return std::string::npos;
    return spacePos;
}

bool Fold::spaceOnBreak(std::size_t lineBeg, std::size_t effectiveWidth) const {
    return this->str[lineBeg + effectiveWidth] == ' ';
}

bool Fold::notLastLine(std::size_t lineBeg, std::size_t effectiveWidth) const {
    return lineBeg + effectiveWidth < this->str.size();
}

std::ostream &operator<<(std::ostream &out, const Fold &fold) {
    return out << static_cast<std::string>(fold);
}
