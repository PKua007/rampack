//
// Created by Piotr Kubala on 12/12/2022.
//

#include "MatcherDataclass.h"
#include "NodeDataclass.h"


namespace pyon::matcher {
    StandardArguments::StandardArguments(std::vector<StandardArgument> arguments) {
        std::vector<std::string> names;
        names.reserve(arguments.size());
        std::transform(arguments.begin(), arguments.end(), std::back_inserter(names), [](const StandardArgument &arg) {
            return arg.name;
        });
        std::sort(names.begin(), names.end());
        if (std::unique(names.begin(), names.end()) != names.end())
            throw DataclassException("pyon::matcher::StandardArguments: duplicate argument names");

        this->arguments = std::move(arguments);
    }

    const StandardArgument &StandardArguments::front() const {
        if (this->empty())
            throw NoSuchArgumentException("pyon::matcher::StandardArgument::front: empty array");
        return this->arguments.front();
    }

    const StandardArgument &StandardArguments::back() const {
        if (this->empty())
            throw NoSuchArgumentException("pyon::matcher::StandardArgument::back: empty array");
        return this->arguments.back();
    }

    const StandardArgument &StandardArguments::at(std::size_t idx) const {
        if (idx >= this->size()) {
            std::ostringstream msg;
            msg << "pyon::matcher::StandardArgument::at: trying to access index " << idx << ", but there are only ";
            msg << this->size() << " arguments";
            throw NoSuchArgumentException(msg.str());
        }
        return this->arguments[idx];
    }

    const Any &StandardArguments::at(const std::string &name) const {
        auto it = std::find_if(this->arguments.begin(), this->arguments.end(), [name](const StandardArgument &arg) {
            return arg.name == name;
        });

        if (it == this->arguments.end())
            throw NoSuchArgumentException("pyon::matcher::StandardArgument::at: there is no argument named: " + name);

        return it->value;
    }

    bool StandardArguments::hasArgument(const std::string &name) const {
        auto it = std::find_if(this->arguments.begin(), this->arguments.end(), [name](const StandardArgument &arg) {
            return arg.name == name;
        });

        return it != this->arguments.end();
    }

    std::size_t DataclassData::size() const {
        return this->standardArguments.size() + this->variadicArguments.size() + this->variadicKeywordArguments.size();
    }

    std::size_t DataclassData::positionalSize() const {
        return this->standardArguments.size() + this->variadicArguments.size();
    }

    bool DataclassData::empty() const {
        return this->standardArguments.empty()
               && this->variadicArguments.empty()
               && this->variadicKeywordArguments.empty();
    }

    bool DataclassData::positionalEmpty() const {
        return this->standardArguments.empty() && this->variadicArguments.empty();
    }

    const Any &DataclassData::at(std::size_t idx) const {
        if (idx >= this->positionalSize()) {
            std::ostringstream msg;
            msg << "pyon::matcher::DataclassData::at: trying to access index " << idx << ", but there are only ";
            msg << this->positionalSize() << " positional arguments";
            throw NoSuchArgumentException(msg.str());
        }

        if (idx < this->standardArguments.size())
            return this->standardArguments.at(idx).value;

        idx -= this->standardArguments.size();
        Assert(idx < this->variadicArguments.size());
        return this->variadicArguments.at(idx);
    }

    const Any &DataclassData::at(const std::string &name) const {
        if (this->standardArguments.hasArgument(name))
            return this->standardArguments.at(name);

        if (!this->variadicKeywordArguments.hasKey(name))
            throw NoSuchArgumentException("pyon::matcher::DataclassData::at: there is no argument named " + name);

        return this->variadicKeywordArguments.at(name);
    }

    MatcherDataclass &MatcherDataclass::variadicArguments(const MatcherArray &variadicMatcher) {
        this->variadicArgumentsMatcher = variadicMatcher;
        this->variadicArgumentsMatcher.mapToDefault();
        return *this;
    }

    MatcherDataclass &MatcherDataclass::variadicKeywordArguments(const MatcherDictionary &variadicMatcher) {
        this->variadicKeywordArgumentsMatcher = variadicMatcher;
        this->variadicKeywordArgumentsMatcher.mapToDefault();
        return *this;
    }

    // TODO: Tests for matching variadic arguments (for example forcing their minimal number)
    bool MatcherDataclass::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::DATACLASS)
            return false;

        const auto &nodeClass = node->as<ast::NodeDataclass>();
        if (nodeClass->getClassName() != this->name)
            return false;

        const auto &nodePositional = nodeClass->getPositionalArguments();
        const auto &nodeKeyword = nodeClass->getKeywordArguments();

        ArrayData variadicArguments;
        std::vector<std::shared_ptr<const ast::Node>> variadicNodes;
        if (nodePositional->size() > this->argumentsSpecification.size())
            variadicNodes.assign(nodePositional->begin() + this->argumentsSpecification.size(), nodePositional->end());
        auto variadicNodesArray = ast::NodeArray::create(std::move(variadicNodes));
        Any variadicResult;
        if (!this->variadicArgumentsMatcher.match(variadicNodesArray, variadicResult))
            return false;
        variadicArguments = variadicResult.as<ArrayData>();

        DictionaryData variadicKeywordArguments;
        std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> variadicKeywordNodes;
        for (const auto &keyNodePair : *nodeKeyword) {
            const auto &key = keyNodePair.first;
            auto isStandardArgument = [&key](const auto &arg) { return arg.getName() == key; };
            if (std::find_if(this->argumentsSpecification.begin(), this->argumentsSpecification.end(),
                             isStandardArgument) == this->argumentsSpecification.end())
            {
                variadicKeywordNodes.emplace_back(keyNodePair);
            }
        }
        auto keywordVariadicNodesDictionary = ast::NodeDictionary::create(variadicKeywordNodes);
        Any variadicKeywordResult;
        if (!this->variadicKeywordArgumentsMatcher.match(keywordVariadicNodesDictionary, variadicKeywordResult))
            return false;
        variadicKeywordArguments = variadicKeywordResult.as<DictionaryData>();

        std::vector<StandardArgument> standardArgumentsVec;
        standardArgumentsVec.reserve(this->argumentsSpecification.size());
        std::size_t maxSize = std::min(nodePositional->size(), this->argumentsSpecification.size());
        for (std::size_t i{}; i < maxSize; i++) {
            const auto &argumentSpecification = this->argumentsSpecification[i];
            if (nodeKeyword->hasKey(argumentSpecification.getName()))
                return false;

            Any argumentValue;
            if (argumentSpecification.hasMatcher()) {
                if (!argumentSpecification.getMatcher()->match(nodePositional->at(i), argumentValue))
                    return false;
            } else {
                argumentValue = nodePositional->at(i);
            }
            standardArgumentsVec.emplace_back(argumentSpecification.getName(), argumentValue);
        }

        for (std::size_t i = nodePositional->size(); i < this->argumentsSpecification.size(); i++) {
            const auto &argumentSpecification = this->argumentsSpecification[i];
            if (nodeKeyword->hasKey(argumentSpecification.getName())) {
                const auto &argumentNode = nodeKeyword->at(argumentSpecification.getName());
                Any argumentValue;
                if (argumentSpecification.hasMatcher()) {
                    if (!argumentSpecification.getMatcher()->match(argumentNode, argumentValue))
                        return false;
                } else {
                    argumentValue = argumentNode;
                }
                standardArgumentsVec.emplace_back(argumentSpecification.getName(), argumentValue);
            } else if (argumentSpecification.hasDefaultValue()) {
                standardArgumentsVec.emplace_back(argumentSpecification.getName(),
                                                  *argumentSpecification.getDefaultValue());
            } else {
                return false;
            }

        }

        StandardArguments standardArguments(std::move(standardArgumentsVec));

        DataclassData classData(standardArguments, variadicArguments, variadicKeywordArguments);

        for (const auto &filter: this->filters)
            if (!filter(classData))
                return false;

        result = this->mapping(classData);
        return true;
    }

    MatcherDataclass &MatcherDataclass::mapTo(const std::function<Any(const DataclassData &)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherDataclass &MatcherDataclass::filter(const std::function<bool(const DataclassData &)> &filter) {
        this->filters.emplace_back(filter);
        return *this;
    }

    MatcherDataclass::MatcherDataclass(std::string className,
                                       std::vector<StandardArgumentSpecification> argumentsSpecification)
            : MatcherDataclass(std::move(className))
    {
        this->arguments(std::move(argumentsSpecification));
    }

    MatcherDataclass::MatcherDataclass(std::string className) : name{std::move(className)} {
        if (this->name.empty())
            throw DataclassException("Dataclass name cannot be empty");
    }

    MatcherDataclass &MatcherDataclass::arguments(std::vector<StandardArgumentSpecification> argumentsSpecification_) {
        for (std::size_t i = 1; i < argumentsSpecification_.size(); i++) {
            const auto &prevArg = argumentsSpecification_[i - 1];
            const auto &currArg = argumentsSpecification_[i];
            if (prevArg.getDefaultValue().has_value() && !currArg.getDefaultValue().has_value()) {
                throw DataclassException("pyon::matcher::MatcherDataclass::arguments: argument with default value: "
                                         + currArg.getName() + " follows argument withour default value: "
                                         + prevArg.getName());
            }
        }

        this->argumentsSpecification = std::move(argumentsSpecification_);
        return *this;
    }
} // matcher