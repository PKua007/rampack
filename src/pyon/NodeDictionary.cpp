//
// Created by pkua on 06.12.22.
//

#include "NodeDictionary.h"
#include "utils/Assertions.h"


namespace pyon::ast {
    NodeDictionary::NodeDictionary(const std::vector<std::pair<std::string, std::shared_ptr<const Node>>> &map)
            : Node(DICTIONARY)
    {
        this->keys.reserve(map.size());
        for (const auto &[key, node] : map) {
            this->keys.push_back(key);
            this->map[key] = node;
        }
    }

    const std::shared_ptr<const Node> &NodeDictionary::at(const std::string &key) const {
        auto it = this->map.find(key);
        if (it == this->map.end())
            throw DictionaryNoSuchKeyException("pyon::ast::NodeDictionary::at: There is no '" + key + "' in map");
        return it->second;
    }

    std::shared_ptr<const NodeDictionary>
            NodeDictionary::create(const std::vector<std::pair<std::string, std::shared_ptr<const Node>>> &map)
    {
        for (const auto &[key, value] : map)
            Expects(value != nullptr);
        return std::shared_ptr<const NodeDictionary>(new NodeDictionary(map));
    }

    std::shared_ptr<const NodeDictionary>
    NodeDictionary::create(const std::initializer_list<std::pair<std::string, std::shared_ptr<const Node>>> &map)
    {
        return NodeDictionary::create(std::vector<std::pair<std::string, std::shared_ptr<const Node>>>{map});
    }

    std::shared_ptr<const NodeDictionary> NodeDictionary::create() {
        return NodeDictionary::create({});
    }
} // ast