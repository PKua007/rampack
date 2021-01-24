//
// Created by Piotr Kubala on 12/12/2020.
//

#include "Config.h"
#include "Utils.h"
#include "Assertions.h"

#include <istream>
#include <set>
#include <ostream>

Config Config::parse(std::istream &in, char delim, bool allowRedefinition) {
    Expects(delim != '#' && delim != ';');

    Config result;
    std::size_t lineNum = 0;
    std::string line;
    std::string currentSection;
    while (std::getline(in, line)) {
        lineNum++;
        stripComment(line);
        trim(line);
        if (line.empty())
            continue;

        if (isItSectionEntry(line, lineNum)) {
            currentSection = line.substr(1, line.size() - 2) + ".";
            continue;
        }

        auto field = splitField(line, delim, lineNum, currentSection);
        if (result.hasField(field.key) && !allowRedefinition) {
            throw ConfigParseException("Redefinition of field \"" + field.key + "\" in line "
                                       + std::to_string(lineNum));
        }

        result.fieldMap[field.key] = field.value;
        if (!result.hasField(field.key))
            result.keys.push_back(field.key);
    }

    result.buildRootSections();
    return result;
}

bool Config::isItSectionEntry(std::string &line, std::size_t lineNum) {
    if (line.front() != '[')
        return false;
    if (line.back() != ']')
        throw ConfigParseException("Malformed section entry in line " + std::to_string(lineNum));
    return line.front() == '[';
}

/**
 * @brief Parses field from the line. Format key=valye. The current section is appended to the key: section.key
 */
Config::Field Config::splitField(const std::string &line, char delim, std::size_t lineNum,
                                 const std::string &currentSection)
{
    Field field;
    std::size_t pos = line.find(delim);
    if (pos == std::string::npos)
        throw ConfigParseException("No '" + std::string(1, delim) + "' sign in line " + std::to_string(lineNum));

    field.key = line.substr(0, pos);
    if (field.key.find('.') != std::string::npos)
        throw ConfigParseException("Key " + field.key + " in line " + std::to_string(lineNum) + " has '.' sign");
    trim(field.key);
    field.key = currentSection + field.key;

    field.value = (pos == line.length() - 1) ? "" : line.substr(pos + 1);
    trim(field.value);
    return field;
}

/**
 * @brief Removes comment from the line (starting with ; or #).
 */
void Config::stripComment(std::string &line) {
    std::size_t pos = line.find_first_of(";#");
    if (pos != std::string::npos)
        line.erase(pos);
}

std::string Config::getString(const std::string & field) const {
    auto iter = this->fieldMap.find(field);
    if (iter == this->fieldMap.end())
        throw ConfigNoFieldException("No \"" + field + "\" field in config");
    return (*iter).second;
}

int Config::getInt(const std::string & field) const {
    return std::stoi(this->getString(field));
}

unsigned long Config::getUnsignedLong(const std::string & field) const {
    auto str = this->getString(field);
    if (std::stoi(str) < 0)
        throw std::invalid_argument("unsigned long field negative");
    return std::stoul(str);
}

double Config::getDouble(const std::string & field) const {
    return std::stod(this->getString(field));
}

float Config::getFloat(const std::string & field) const
{
    return std::stof(this->getString(field));
}

bool Config::getBoolean(const std::string &field) const {
    if (this->getString(field) == "true")
        return true;
    else if (this->getString(field) == "false")
        return false;
    else
        throw ConfigParseException("Cannot read boolean from " + field + ". It is neither 'true' nor 'false'");
}

bool Config::hasField(const std::string &field) const {
    return std::find(keys.begin(), keys.end(), field) != keys.end();
}

bool Config::hasRootSection(const std::string &section) const {
    return std::find(this->rootSections.begin(), this->rootSections.end(), section) != this->rootSections.end();
}

/**
 * @brief It takes all keys and construct root sections. Note, that "" is also root section.
 * @detail Ex: keys key1, sec1.key2, sec2.sub1.key3, sec2.sub2.key4 yield "", sec1 and sec2 root sections
 */
void Config::buildRootSections() {
    std::set<std::string> sectionsSet;
    for (const auto &key : this->keys) {
        std::size_t pos = key.find('.');
        std::string sectionName;
        if (pos == std::string::npos)
            sectionName = "";
        else
            sectionName = key.substr(0, pos);
        if (sectionsSet.find(sectionName) == sectionsSet.end()) {
            sectionsSet.insert(sectionName);
            this->rootSections.push_back(sectionName);
        }
    }
}

Config Config::fetchSubconfig(const std::string &rootSection) const {
    if (rootSection.find('.') != std::string::npos)
        throw std::invalid_argument("Root section names do not have dots");

    if (!this->hasRootSection(rootSection))
        throw std::invalid_argument("No root section " + rootSection + " in config");

    Config result;
    if (rootSection.empty()) {
        // Fetching "empty" section, so all keys without section
        for (const auto &key : this->keys) {
            if (key.find('.') == std::string::npos) {
                result.fieldMap[key] = this->fieldMap.at(key);
                result.keys.push_back(key);
            }
        }
    } else {
        // Fetching named section
        for (const auto &key : this->keys) {
            if (startsWith(key, rootSection + ".")) {
                std::string subKey = key.substr(rootSection.size() + 1);
                result.fieldMap[subKey] = this->fieldMap.at(key);
                result.keys.push_back(subKey);
            }
        }
    }
    result.buildRootSections();
    return result;
}

std::ostream &operator<<(std::ostream &out, const Config &config) {
    for (const auto &pair : config.fieldMap)
        out << pair.first << " = " << pair.second << std::endl;
    return out;
}