//
// Created by pkua on 29.08.22.
//

#include <algorithm>

#include "ParseUtils.h"
#include "utils/Assertions.h"


std::map<std::string, std::string> ParseUtils::parseFields(const std::vector<std::string> &fields,
                                                           const std::vector<std::string> &tokens)
{
    std::map<std::string, std::string> fieldMap;
    auto currField = fieldMap.end();
    for (const auto &token : tokens) {
        if (std::find(fields.begin(), fields.end(), token) != fields.end()) {
            if (fieldMap.find(token) != fieldMap.end())
                throw ValidationException("Redefined field: " + token);
            currField = fieldMap.insert({token, ""}).first;
        } else {
            if (currField == fieldMap.end()) {
                if (std::find(fields.begin(), fields.end(), "") == fields.end())
                    throw ValidationException("Empty field name is not supported in this context");
                currField = fieldMap.insert({"", ""}).first;
            }

            auto &value = currField->second;
            if (value.empty()) {
                value = token;
            } else {
                value += " ";
                value += token;
            }
        }
    }
    return fieldMap;
}

bool ParseUtils::isAnythingLeft(std::istream &stream) {
    if (!stream.good())
        return false;
    stream >> std::ws;
    return stream.good();
}
