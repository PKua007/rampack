//
// Created by Piotr Kubala on 30/01/2023.
//

#include <sstream>

#include "Exceptions.h"


std::string ContractException::makeWhat(const std::string &file_, const std::string &function_, std::size_t line_,
                                        const std::string &condition_, const std::string &message_)
{
    std::ostringstream out;
    out << "Contract violated!" << std::endl;
    out << "Location         : " << file_ << ":" << line_ << " (function \"" << function_ << "\"):" << std::endl;
    out << "Failed condition : " << condition_ << std::endl;
    out << "Message          : " << message_;
    return out.str();
}
