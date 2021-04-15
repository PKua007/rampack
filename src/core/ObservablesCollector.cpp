//
// Created by Piotr Kubala on 22/03/2021.
//

#include <sstream>
#include <ostream>
#include <iterator>

#include "ObservablesCollector.h"
#include "utils/Utils.h"

void ObservablesCollector::addObservable(std::unique_ptr<Observable> observable, bool shouldDisplayInline) {
    if (!this->snapshotValues.empty())
        ExpectsMsg(snapshotValues.front().empty(), "Cannot add a new observable if snapshots are already captured");

    auto header = observable->getHeader();
    this->observableHeader.insert(this->observableHeader.end(), header.begin(), header.end());
    this->snapshotValues.resize(this->snapshotValues.size() + header.size());
    this->averagingValues.resize(this->averagingValues.size() + header.size());
    this->observables.push_back(std::move(observable));
    if (shouldDisplayInline)
        this->inlineObservablesIndices.push_back(this->observables.size() - 1);
}

void ObservablesCollector::addSnapshot(const Packing &packing, std::size_t cycleNumber, const ShapeTraits &shapeTraits)
{
    this->snapshotCycleNumbers.push_back(cycleNumber + this->cycleOffset);
    this->addObservablesToContainer(packing, shapeTraits, this->snapshotValues);
}

void ObservablesCollector::addAveragingValues(const Packing &packing, const ShapeTraits &shapeTraits) {
    this->addObservablesToContainer(packing, shapeTraits, this->averagingValues);
}

void ObservablesCollector::addObservablesToContainer(const Packing &packing, const ShapeTraits &shapeTraits,
                                                     std::vector<std::vector<double>> &container)
{
    std::size_t valueIndex{};
    for (auto &observable : observables) {
        observable->calculate(packing, this->temperature, this->pressure, shapeTraits);
        auto values = observable->getValues();
        for (double value : values) {
            Assert(valueIndex < container.size());
            container[valueIndex].push_back(value);
            valueIndex++;
        }
    }
    Assert(valueIndex == container.size());
}

std::string ObservablesCollector::generateInlineObservablesString(const Packing &packing,
                                                                  const ShapeTraits &shapeTraits) const
{
    if (this->inlineObservablesIndices.empty())
        return "";

    std::ostringstream stream;
    for (std::size_t i{}; i < this->inlineObservablesIndices.size() - 1; i++) {
        this->printInlineObservable(this->inlineObservablesIndices[i], packing, shapeTraits, stream);
        stream << ", ";
    }
    this->printInlineObservable(this->inlineObservablesIndices.back(), packing, shapeTraits, stream);

    return stream.str();
}

void ObservablesCollector::printInlineObservable(unsigned long observableIdx, const Packing &packing,
                                                 const ShapeTraits &shapeTraits, std::ostringstream &out) const
{
    auto &observable = *this->observables[observableIdx];
    observable.calculate(packing, this->temperature, this->pressure, shapeTraits);
    auto header = observable.getHeader();
    auto values = observable.getValues();
    Assert(!header.empty());
    Assert(header.size() == values.size());

    for (std::size_t i{}; i < header.size() - 1; i++)
        out << header[i] << ": " << values[i] << ", ";
    out << header.back() << ": " << values.back();
}

void ObservablesCollector::clearValues() {
    this->snapshotCycleNumbers.clear();
    for (auto &singleSet : this->snapshotValues)
        singleSet.clear();
}

void ObservablesCollector::printSnapshots(std::ostream &out, bool printHeader) const {
    Expects(!this->observableHeader.empty());
    Expects(!this->observableHeader.front().empty());

    std::size_t numSnapshots = this->snapshotCycleNumbers.size();
    for (const auto &singleSet : this->snapshotValues)
        Assert(singleSet.size() == numSnapshots);

    if (printHeader) {
        out << "cycle ";
        std::copy(this->observableHeader.begin(), this->observableHeader.end(),
                  std::ostream_iterator<std::string>(out, " "));
        out << std::endl;
    }

    for (std::size_t i{}; i < numSnapshots; i++) {
        out << this->snapshotCycleNumbers[i] << " ";
        for (const auto &observableValues : this->snapshotValues)
            out << observableValues[i] << " ";
        out << std::endl;
    }
}

std::vector<ObservablesCollector::ObservableData> ObservablesCollector::getFlattenedAverageValues() const {
    std::vector<ObservableData> flatValues(this->observableHeader.size());
    for (std::size_t i{}; i < flatValues.size(); i++) {
        flatValues[i].name = this->observableHeader[i];
        flatValues[i].quantity.calculateFromSamples(this->averagingValues[i]);
    }
    return flatValues;
}

std::vector<ObservablesCollector::ObservableGroupData> ObservablesCollector::getGroupedAverageValues() const {
    std::vector<ObservableGroupData> groupedValues;
    groupedValues.reserve(this->observables.size());
    auto flatValues = this->getFlattenedAverageValues();

    std::size_t flatIndex{};
    for (const auto &observable : this->observables) {
        auto header = observable->getHeader();
        Assert(flatIndex + header.size() <= flatValues.size());
        groupedValues.push_back({
            observable->getName(),
            std::vector<ObservableData>(flatValues.begin() + flatIndex, flatValues.begin() + flatIndex + header.size())
        });
        flatIndex += header.size();
    }
    Assert(flatIndex == flatValues.size());

    return groupedValues;
}

void ObservablesCollector::setThermodynamicParameters(double temperature_, double pressure_) {
    Expects(temperature_ > 0);
    Expects(pressure_ > 0);

    this->temperature = temperature_;
    this->pressure = pressure_;
}

void ObservablesCollector::setCycleOffset(std::size_t offset) {
    this->cycleOffset = offset;
}

std::size_t ObservablesCollector::getMemoryUsage() const {
    std::size_t bytes{};
    for (const auto &vec : this->snapshotValues)
        bytes += get_vector_memory_usage(vec);
    for (const auto &vec : this->averagingValues)
        bytes += get_vector_memory_usage(vec);
    return bytes;
}
