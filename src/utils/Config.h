//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_CONFIG_H
#define RAMPACK_CONFIG_H


#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>


/**
 * @brief An exception thrown if there was a problem parsing config in Config.
 */
class ConfigParseException : public std::runtime_error
{
public:
    explicit ConfigParseException(const std::string & _what) : std::runtime_error(_what)
    {}
};


/**
 * @brief An exception thrown if there was an access to nonexistent field in Config.
 */
class ConfigNoFieldException : public std::runtime_error
{
public:
    explicit ConfigNoFieldException(const std::string & _what) : std::runtime_error(_what)
    {}
};


/**
 * @brief A key=value config file parser (INI format) with sections support.
 *
 * File format:
 * \code
 * key1=value1
 * # standalone comment
 * key2=value2 # end-line comment;
 *
 * # empty lines are omitted
 * key3 = value3 # whitespace is trimmed
 * # key3 = value3 - duplicate fields are forbidden
 * \endcode
 *
 * Example explaining sections:
 * \code
 * # Key without section
 * key1=value1
 *
 * # Sections names are in [...]
 * [section]
 * key2=value2
 *
 * # Sections do not nest, but this effect can be achieved by using dots within section names
 * [section.subsection1]
 * key3=value3
 *
 * [section.subsection2]
 * key4=value4
 * \endcode
 *
 * The values are then accessed by @a secionName.keyName.
 *
 * Each section can be used to fetch subconfig object (fetchSubconfig()). Then, only the keys from this section are
 * exported an they no longer have @a sectionName prefix. Note, that subsections nested by using dot will remain,
 * so the process may be recursed.
 */
class Config
{
private:
    std::map<std::string, std::string>  fieldMap;
    std::vector<std::string>            keys;
    std::vector<std::string>            rootSections;

    struct Field {
        std::string key;
        std::string value;
    };

    static void stripComment(std::string &line);
    static bool isItSectionEntry(std::string &line, std::size_t lineNum);
    static Field splitField(const std::string &line, char delim, std::size_t lineNum,
                            const std::string &currentSection);

    void buildRootSections();

public:

    /**
     * @brief Parses given stream.
     * @details Format: see class description.
     * @param in stream to parse from
     * @param delim delimiter for key, value; defaults to '='
     * @param allowRedefinition if `true`, redefinition of field will overwrite the old value; if `false`, it will
     * throw ConfigParseException
     * @throws ConfigParseException on parse error (no delimiter of a duplicate field)
     * @throws std::invalid_argument when delim = '#' (comment)
     * @return Config object to be deleted manualy after use
     */
    static Config parse(std::istream &in, char delim = '=', bool allowRedefinition = false);

    Config() = default;

    bool hasField(const std::string &field) const;
    std::size_t size() const { return this->keys.size(); }
    bool empty() const { return this->keys.empty(); }

    std::string getString(const std::string &field) const;
    int getInt(const std::string &field) const;
    unsigned long getUnsignedLong(const std::string &field) const;
    double getDouble(const std::string &field) const;
    float getFloat(const std::string &field) const;
    bool getBoolean(const std::string &field) const;

    /**
     * @brief Returns keys in a config, preserving order from an input
     * @return keys in a config, preserving order from an input
     */
    std::vector<std::string> getKeys() const { return this->keys; }

    /**
     * @brief Returns names of root sections - everything before the first dot in section name.
     * @details Note, that there is a section with an empty name for keys without any section.
     */
    std::vector<std::string> getRootSections() const { return this->rootSections; }

    /**
     * @brief Returns @a true if a section with name @a section is present
     * @details Note, that there is a section with an empty name for keys without any section.
     */
    bool hasRootSection(const std::string &section) const;

    /**
     * @brief Prepares subconfig file for section @a rootSection.
     * @details It includes all key=value pairs from this root section, including subsections. Of course all
     * @a rootSection prefixes are stripped in the result, but subsections remain. The procedure can be recursed for
     * deeper nesting. Note, that one can also fetch fields with no section passing the empty string.
     */
    Config fetchSubconfig(const std::string &rootSection) const;

    friend std::ostream &operator<<(std::ostream &out, const Config &config);
};


#endif //RAMPACK_CONFIG_H
