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
        this->hasVariadicArguments = true;
        return *this;
    }

    MatcherDataclass &MatcherDataclass::variadicKeywordArguments(const MatcherDictionary &variadicMatcher) {
        this->variadicKeywordArgumentsMatcher = variadicMatcher;
        this->variadicKeywordArgumentsMatcher.mapToDefault();
        this->hasKeywordVariadicArguments = true;
        return *this;
    }

    MatchReport MatcherDataclass::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::DATACLASS)
            return this->generateDataclassUnmatchedReport("Got incorrect node type: " + node->getNodeName());

        const auto &nodeClass = node->as<ast::NodeDataclass>();
        if (nodeClass->getClassName() != this->name) {
            return this->generateDataclassUnmatchedReport("Got incorrect class name: "
                                                         + quoted(nodeClass->getClassName()));
        }

        const auto &nodePositional = nodeClass->getPositionalArguments();
        const auto &nodeKeyword = nodeClass->getKeywordArguments();

        MatchReport matchedReport;

        StandardArguments standardArguments;
        matchedReport = this->matchStandardArguments(standardArguments, nodePositional, nodeKeyword);
        if (!matchedReport)
            return matchedReport.getReason();

        ArrayData variadicArguments;
        matchedReport = this->matchVariadicArguments(variadicArguments, nodePositional);
        if (!matchedReport)
            return matchedReport.getReason();

        DictionaryData variadicKeywordArguments;
        matchedReport = this->matchKeywordVariadicArguments(variadicKeywordArguments, nodeKeyword);
        if (!matchedReport)
            return matchedReport.getReason();

        DataclassData classData(standardArguments, variadicArguments, variadicKeywordArguments);

        for (const auto &filter: this->filters)
            if (!filter.predicate(classData))
                return this->generateDataclassUnmatchedReport("Condition not satisfied: " + filter.description);

        result = this->mapping(classData);
        return true;
    }

    std::string MatcherDataclass::outline(std::size_t indent) const {
        std::ostringstream out;
        std::string spaces(indent, ' ');
        out << spaces << "class \"" << this->name << "\":";

        this->outlineArgumentsSpecification(out, indent, true);
        this->outlineFilters(out, indent);

        return out.str();
    }

    void MatcherDataclass::outlineArgumentsSpecification(std::ostringstream &out, std::size_t indent,
                                                         bool verbose) const
    {
        this->outlineStandardArguments(out, indent, verbose);
        this->outlineVariadicArguments(out, indent, verbose);
        this->outlineKeywordVariadicArguments(out, indent, verbose);
    }

    void MatcherDataclass::outlineStandardArguments(std::ostringstream &out, std::size_t indent, bool verbose) const {
        std::string spaces(indent, ' ');
        out << std::endl << spaces << "- standard arguments:";

        if (this->argumentsSpecification.empty()) {
            out << " empty";
            return;
        }

        for (const auto &argument: this->argumentsSpecification)
            this->outlineArgument(argument, out, indent, verbose);
    }

    void MatcherDataclass::outlineArgument(const StandardArgumentSpecification &argument, std::ostringstream &out,
                                           std::size_t indent, bool verbose) const
    {
        std::string spaces(indent, ' ');
        out << std::endl << spaces << "  - " << argument.getName();
        if (argument.hasDefaultValue())
            out << " (=" << argument.getDefaultValueString() << ")";

        if (!verbose)
            return;

        out << ": ";
        if (!argument.hasMatcher())
            out << "any expression";
        else
            out << argument.getMatcher()->outline(indent + 4).substr(indent + 4);
    }

    void MatcherDataclass::outlineVariadicArguments(std::ostringstream &out, std::size_t indent, bool verbose) const {
        if (!this->hasVariadicArguments)
            return;

        std::string spaces(indent, ' ');
        out << std::endl << spaces << "- *args: ";
        if (verbose)
            out << this->variadicArgumentsMatcher.outline(indent + 2).substr(indent + 2);
        else
            out << this->variadicArgumentsMatcher.synopsis();
    }

    void MatcherDataclass::outlineKeywordVariadicArguments(std::ostringstream &out, std::size_t indent,
                                                           bool verbose) const
    {
        if (!this->hasKeywordVariadicArguments)
            return;

        std::string spaces(indent, ' ');
        out << std::endl << spaces << "- **kwargs: ";
        if (verbose)
            out << this->variadicKeywordArgumentsMatcher.outline(indent + 2).substr(indent + 2);
        else
            out << this->variadicKeywordArgumentsMatcher.synopsis();
    }

    void MatcherDataclass::outlineFilters(std::ostringstream &out, std::size_t indent) const {
        std::string spaces(indent, ' ');
        for (const auto &filter: this->filters)
            out << std::endl << spaces << "- " << filter.description;
    }

    MatcherDataclass &MatcherDataclass::mapTo(const std::function<Any(const DataclassData &)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherDataclass &MatcherDataclass::filter(const std::function<bool(const DataclassData &)> &filter) {
        this->filters.push_back({filter, "<undefined filter>"});
        return *this;
    }

    MatcherDataclass &MatcherDataclass::describe(const std::string &description) {
        Expects(!this->filters.empty());
        this->filters.back().description = description;
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
                                         + currArg.getName() + " follows argument without default value: "
                                         + prevArg.getName());
            }
        }

        this->argumentsSpecification = std::move(argumentsSpecification_);
        return *this;
    }

    MatchReport MatcherDataclass::matchVariadicArguments(ArrayData &arguments,
                                                         const std::shared_ptr<const ast::NodeArray> &nodePositional) const
    {
        std::vector<std::shared_ptr<const ast::Node>> variadicNodes;
        if (nodePositional->size() > this->argumentsSpecification.size())
            variadicNodes.assign(nodePositional->begin() + this->argumentsSpecification.size(), nodePositional->end());
        auto variadicNodesArray = ast::NodeArray::create(std::move(variadicNodes));
        Any result;
        auto matched = this->variadicArgumentsMatcher.match(variadicNodesArray, result);
        if (!matched)
            return this->generateArgumentUnmatchedReport("*args", matched.getReason());
        arguments = result.as<ArrayData>();
        return true;
    }

    MatchReport
    MatcherDataclass::matchKeywordVariadicArguments(DictionaryData &arguments,
                                                    const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const
    {
        std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> variadicKeywordNodes;
        for (const auto &keyNodePair : *nodeKeyword)
            if (!this->isStandardArgument(keyNodePair.first))
                variadicKeywordNodes.emplace_back(keyNodePair);

        auto variadicNodesDictionary = ast::NodeDictionary::create(variadicKeywordNodes);
        Any result;
        auto matched = this->variadicKeywordArgumentsMatcher.match(variadicNodesDictionary, result);
        if (!matched)
            return this->generateArgumentUnmatchedReport("**kwargs", matched.getReason());
        arguments = result.as<DictionaryData>();
        return true;
    }

    MatchReport
    MatcherDataclass::matchStandardArguments(StandardArguments &arguments,
                                             const std::shared_ptr<const ast::NodeArray> &nodePositional,
                                             const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const
    {
        auto matchReport = this->validateArguments(nodePositional, nodeKeyword);
        if (!matchReport)
            return matchReport.getReason();

        std::vector<StandardArgument> standardArgumentsVec;
        standardArgumentsVec.reserve(this->argumentsSpecification.size());

        // Construct and match explicitly given positional arguments
        std::size_t numPositionalGiven = std::min(nodePositional->size(), this->argumentsSpecification.size());
        for (std::size_t i{}; i < numPositionalGiven; i++) {
            const auto &argumentSpecification = this->argumentsSpecification[i];
            auto matched = this->emplaceArgument(standardArgumentsVec, argumentSpecification, nodePositional->at(i));
            if (!matched)
                return matched.getReason();
        }

        // Construct and match the rest by default values or keyword arguments
        for (std::size_t i = nodePositional->size(); i < this->argumentsSpecification.size(); i++) {
            const auto &argumentSpecification = this->argumentsSpecification[i];
            if (nodeKeyword->hasKey(argumentSpecification.getName())) {
                const auto &argumentNode = nodeKeyword->at(argumentSpecification.getName());
                Any argumentValue;
                auto matched = this->emplaceArgument(standardArgumentsVec, argumentSpecification, argumentNode);
                if (!matched)
                    return matched.getReason();
            } else if (argumentSpecification.hasDefaultValue()) {
                standardArgumentsVec.emplace_back(argumentSpecification.getName(),
                                                  *argumentSpecification.getDefaultValue());
            } else {
                throw AssertionException("Missing argument should've been caught earlier");
            }
        }

        arguments = StandardArguments(std::move(standardArgumentsVec));
        return true;
    }

    MatchReport MatcherDataclass::emplaceArgument(std::vector<StandardArgument> &standardArgumentsVec,
                                                  const StandardArgumentSpecification &argumentSpecification,
                                                  const std::shared_ptr<const ast::Node> &argumentNode) const
    {
        if (!argumentSpecification.hasMatcher()) {
            standardArgumentsVec.emplace_back(argumentSpecification.getName(), argumentNode);
            return true;
        }

        Any argumentValue;
        auto matched = argumentSpecification.getMatcher()->match(argumentNode, argumentValue);
        if (!matched) {
            return this->generateArgumentUnmatchedReport("argument " + quoted(argumentSpecification.getName()),
                                                         matched.getReason());
        }
        standardArgumentsVec.emplace_back(argumentSpecification.getName(), argumentValue);
        return true;
    }

    std::string MatcherDataclass::generateDataclassUnmatchedReport(const std::string &reason) const {
        std::ostringstream out;
        out << "Matching class \"" << this->name << "\" failed:" << std::endl;
        out << "✖ " << reason;
        return out.str();
    }

    std::string MatcherDataclass::generateArgumentsReport(const std::string &reason) const {
        std::ostringstream out;
        out << "Matching class \"" << this->name << "\" failed:" << std::endl;
        out << "✖ " << reason << std::endl;
        out << "✓ Arguments specification:";
        this->outlineArgumentsSpecification(out, 2, false);
        return out.str();
    }

    std::string MatcherDataclass::generateArgumentUnmatchedReport(const std::string &argumentName,
                                                                  const std::string &reason) const
    {
        std::ostringstream out;
        out << "Matching class \"" << this->name << "\" failed: Matching " << argumentName << " failed:";
        out << std::endl;
        out << "✖ " << replaceAll(reason, "\n", "\n  ");
        return out.str();
    }

    std::pair<std::size_t, std::size_t> MatcherDataclass::countRequiredArguments() const {
        auto hasDefault = [](const StandardArgumentSpecification &argument) {
            return argument.hasDefaultValue();
        };
        auto firstDefaultIt = std::find_if(this->argumentsSpecification.begin(), this->argumentsSpecification.end(),
                                        hasDefault);

        std::size_t minArguments = firstDefaultIt - this->argumentsSpecification.begin();
        std::size_t maxArguments = this->hasVariadicArguments
                ? std::numeric_limits<std::size_t>::max()
                : this->argumentsSpecification.size();
        return {minArguments, maxArguments};
    }

    bool MatcherDataclass::isStandardArgument(const std::string &argumentName) const {
        auto isStandardArgument = [&argumentName](const auto &arg) {
            return arg.getName() == argumentName;
        };

        return std::find_if(this->argumentsSpecification.begin(), this->argumentsSpecification.end(),
                            isStandardArgument)
               != this->argumentsSpecification.end();
    }

    MatchReport MatcherDataclass::validateArguments(const std::shared_ptr<const ast::NodeArray> &nodePositional,
                                                    const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const
    {
        MatchReport matchReport = this->validateExcessiveArguments(nodePositional);
        if (!matchReport)
            return matchReport;

        matchReport = this->validateMissingArguments(nodePositional, nodeKeyword);
        if (!matchReport)
            return matchReport;

        matchReport = this->validateRedefinedArguments(nodePositional, nodeKeyword);
        if (!matchReport)
            return matchReport;

        matchReport = this->validateUnknownKeywordArguments(nodeKeyword);
        if (!matchReport)
            return matchReport;

        return true;
    }

    MatchReport
    MatcherDataclass::validateExcessiveArguments(const std::shared_ptr<const ast::NodeArray> &nodePositional) const {
        auto[minArguments, maxArguments] = this->countRequiredArguments();
        std::size_t argumentsSize = nodePositional->size();
        if (argumentsSize <= maxArguments)
            return true;

        std::ostringstream out;
        out << "Expected ";
        if (minArguments == maxArguments) {
            if (minArguments == 1)
                out << "1 positional argument";
            else
                out << minArguments << " positional arguments";
        } else {
            out << "from " << minArguments << " to " << maxArguments << " positional arguments";
        }
        out << ", but " << argumentsSize;
        out << (argumentsSize < 2 ? " was given" : " were given");

        return this->generateArgumentsReport(out.str());
    }

    MatchReport
    MatcherDataclass::validateMissingArguments(const std::shared_ptr<const ast::NodeArray> &nodePositional,
                                               const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const
    {
        std::vector<std::string> missingArguments;
        for (std::size_t i = nodePositional->size(); i < this->argumentsSpecification.size(); i++) {
            const auto &argumentSpecification = this->argumentsSpecification[i];
            if (nodeKeyword->hasKey(argumentSpecification.getName()))
                continue;
            if (argumentSpecification.hasDefaultValue())
                continue;
            missingArguments.push_back(argumentSpecification.getName());
        }

        if (missingArguments.empty())
            return true;

        std::transform(missingArguments.begin(), missingArguments.end(), missingArguments.begin(), quoted);
        std::ostringstream out;
        if (missingArguments.size() == 1)
            out << "Missing 1 required positional argument: ";
        else
            out << "Missing " << missingArguments.size() << " required positional arguments: ";
        out << implode(missingArguments, ", ");

        return this->generateArgumentsReport(out.str());
    }

    MatchReport
    MatcherDataclass::validateRedefinedArguments(const std::shared_ptr<const ast::NodeArray> &nodePositional,
                                                 const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const
    {
        std::size_t numPositionalGiven = std::min(nodePositional->size(), this->argumentsSpecification.size());
        for (std::size_t i{}; i < numPositionalGiven; i++) {
            const auto &argumentSpecification = this->argumentsSpecification[i];
            if (nodeKeyword->hasKey(argumentSpecification.getName())) {
                return this->generateArgumentsReport("Positional argument " + quoted(argumentSpecification.getName())
                                                     + " redefined with keyword argument");
            }
        }

        return true;
    }

    MatchReport MatcherDataclass
        ::validateUnknownKeywordArguments(const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const
    {
        if (this->hasKeywordVariadicArguments)
            return true;

        for (const auto &[key, value]: *nodeKeyword)
            if (!isStandardArgument(key))
                return this->generateArgumentsReport("Unknown argument " + quoted(key));

        return true;
    }
} // matcher