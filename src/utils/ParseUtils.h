//
// Created by pkua on 29.08.22.
//

#ifndef RAMPACK_PARSEUTILS_H
#define RAMPACK_PARSEUTILS_H

#include <vector>
#include <istream>
#include <sstream>
#include <string>
#include <map>


class ParseUtils {
public:
    template <typename T>
    static std::vector<T> tokenize(std::istream &in) {
        std::vector<T> tokens;
        in >> std::ws;
        while (in.good()) {
            T token;
            in >> token;
            if (in.good())
                in >> std::ws;
            tokens.push_back(token);
        }
        return tokens;
    }

    template <typename T>
    static std::vector<T> tokenize(const std::string &str) {
        std::istringstream in(str);
        return tokenize<T>(in);
    }

    /* It parses tokenized string to a key=>value map. Allowed fields are given by 'fields'. Values of fields are
     * all tokens that follow a token equal to the field name, up to the end or another field token (they are joined
     * using spaces). If one of allowed fields is "", then everything before "named" fields is regarded as "" field's
     * value.
     *
     * Example: for fields "", "pear", "plum" and "apple", tokens:
     *
     * 1 2 3 apple 4 5 6 pear plum 7 8 9
     *
     * will be parsed into:
     *
     * ""      => "1 2 3"
     * "apple" => "4 5 6"
     * "pear"  => ""
     * "plum"  => "7 8 9"
     * */
    static std::map<std::string, std::string> parseFields(const std::vector<std::string> &fields,
                                                          const std::vector<std::string> &tokens);

    static std::map<std::string, std::string> parseFields(const std::vector<std::string> &fields,
                                                          const std::string &str)
    {
        return ParseUtils::parseFields(fields, ParseUtils::tokenize<std::string>(str));
    }

    static std::map<std::string, std::string> parseFields(const std::vector<std::string> &fields, std::istream &in) {
        return ParseUtils::parseFields(fields, ParseUtils::tokenize<std::string>(in));
    }

    static bool isAnythingLeft(std::istream &stream);
};


#endif //RAMPACK_PARSEUTILS_H
