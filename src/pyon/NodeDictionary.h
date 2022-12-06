//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_NODEDICTIONARY_H
#define RAMPACK_NODEDICTIONARY_H

#include <vector>
#include <map>

#include "Node.h"


namespace pyon::ast {
    class DictionaryNoSuchKeyException : public ASTException {
        using ASTException::ASTException;
    };

    class NodeDictionary : public Node {
    private:
        std::vector<std::string> keys;
        std::map<std::string, std::shared_ptr<const Node>> map;

        explicit NodeDictionary(const std::vector<std::pair<std::string, std::shared_ptr<const Node>>> &map);

    public:
        using iterator = decltype(map)::const_iterator;

        static constexpr Type NODE_TYPE = DICTIONARY;

        [[nodiscard]] bool empty() const { return this->map.empty(); }
        [[nodiscard]] std::size_t size() const { return this->map.size(); }
        [[nodiscard]] const std::shared_ptr<const Node> &at(const std::string &key) const;
        [[nodiscard]] bool hasKey(const std::string &key) const { return this->map.find(key) != this->map.end(); }
        [[nodiscard]] const std::vector<std::string> &getKeys() const { return this->keys; }
        [[nodiscard]] iterator begin() const { return this->map.begin(); }
        [[nodiscard]] iterator end() const { return this->map.end(); }

        static std::shared_ptr<const NodeDictionary> create();

        static std::shared_ptr<const NodeDictionary>
        create(const std::vector<std::pair<std::string, std::shared_ptr<const Node>>> &map);

        static std::shared_ptr<const NodeDictionary>
        create(const std::initializer_list<std::pair<std::string, std::shared_ptr<const Node>>> &map);
    };
} // ast

#endif //RAMPACK_NODEDICTIONARY_H
