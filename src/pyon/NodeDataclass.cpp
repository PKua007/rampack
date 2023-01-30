//
// Created by pkua on 06.12.22.
//

#include "NodeDataclass.h"
#include "utils/Exceptions.h"


namespace pyon::ast {
    std::shared_ptr<const NodeDataclass> NodeDataclass::create(std::string className,
                                                               std::shared_ptr<const NodeArray> positionalArguments,
                                                               std::shared_ptr<const NodeDictionary> keywordArguments)
    {
        Expects(!className.empty());
        Expects(positionalArguments != nullptr);
        Expects(keywordArguments != nullptr);

        return std::shared_ptr<const NodeDataclass>(new NodeDataclass(
            std::move(className), std::move(positionalArguments), std::move(keywordArguments)
        ));
    }
}