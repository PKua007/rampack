//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_NODEDATACLASS_H
#define RAMPACK_NODEDATACLASS_H

#include "NodeLiteral.h"
#include <utility>
#include <vector>

#include "Node.h"
#include "NodeArray.h"
#include "NodeDictionary.h"


namespace pyon::ast {
    class NodeDataclass : public Node {
    private:
        std::string className;
        std::shared_ptr<const NodeArray> positionalArguments;
        std::shared_ptr<const NodeDictionary> keywordArguments;

        NodeDataclass(std::string className, std::shared_ptr<const NodeArray> positionalArguments,
                      std::shared_ptr<const NodeDictionary> keywordArguments)
                : Node(DATACLASS), className{std::move(className)},
                  positionalArguments{std::move(positionalArguments)}, keywordArguments{std::move(keywordArguments)}
        { }

    public:
        static constexpr Type NODE_TYPE = DATACLASS;

        [[nodiscard]] const std::string &getClassName() const { return this->className; }
        [[nodiscard]] const std::shared_ptr<const NodeArray> &getPositionalArguments() const {
            return this->positionalArguments;
        }
        [[nodiscard]] const std::shared_ptr<const NodeDictionary> &getKeywordArguments() const {
            return this->keywordArguments;
        }
        [[nodiscard]] bool empty() const {
            return this->positionalArguments->empty() && this->keywordArguments->empty();
        }

        static std::shared_ptr<const NodeDataclass> create(std::string className,
                                                           std::shared_ptr<const NodeArray> positionalArguments,
                                                           std::shared_ptr<const NodeDictionary> keywordArguments);
    };
} // ast

#endif //RAMPACK_NODEDATACLASS_H
