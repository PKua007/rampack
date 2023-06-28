//
// Created by Piotr Kubala on 30/01/2023.
//

#include <sstream>
#include <filesystem>

#include "Exceptions.h"


std::string ContractException::makeWhat(const std::string &file_, const std::string &function_, std::size_t line_,
                                        const std::string &condition_, const std::string &message_)
{
    std::string relativePath = std::filesystem::relative(file_, RAMPACK_ROOT_DIR);
    relativePath = "<project root>/" + relativePath;

    std::ostringstream out;
    out << "Contract violated!" << std::endl;
    out << "Location         : " << relativePath << ":" << line_ << " (function \"" << function_ << "\"):" << std::endl;
    out << "Failed condition : " << condition_ << std::endl;
    out << "Message          : " << message_;
    return out.str();
}
