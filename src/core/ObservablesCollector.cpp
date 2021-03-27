//
// Created by Piotr Kubala on 22/03/2021.
//

#include <sstream>
#include <ostream>
#include <iterator>

#include "ObservablesCollector.h"

void ObservablesCollector::addObservable(std::unique_ptr<Observable> observable, bool shouldDisplayInline) {
    if (!this->snapshotValues.empty())
        ExpectsMsg(snapshotValues.front().empty(), "Cannot add a new observable if snapshots are already captured");

    auto header = observable->getHeader();
    this->observableHeader.insert(this->observableHeader.end(), header.begin(), header.end());
    this->snapshotValues.resize(this->snapshotValues.size() + header.size());
    this->averagingValues.resize(this->averagingValues.size() + header.size());
    this->observables.push_back(std::move(observable));
    if (shouldDisplayInline)
        this->inlineObservableIndices.push_back(this->observables.size() - 1);
}

void ObservablesCollector::addSnapshot(const Packing &packing, std::size_t cycleNumber, const ShapeTraits &shapeTraits)
{
    this->snapshotCycleNumbers.push_back(cycleNumber);
    this->addObservablesToContainer(packing, this->snapshotValues, shapeTraits);
}

void ObservablesCollector::addObservablesToContainer(const Packing &packing,
                                                     std::vector<std::vector<double>> &container,
                                                     const ShapeTraits &shapeTraits)
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
    if (this->inlineObservableIndices.empty())
        return "";

    std::ostringstream stream;
    for (std::size_t i{}; i < this->inlineObservableIndices.size() - 1; i++) {
        this->printInlineObservable(this->inlineObservableIndices[i], packing, shapeTraits, stream);
        stream << ", ";
    }
    this->printInlineObservable(this->inlineObservableIndices.back(), packing, shapeTraits, stream);

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

    for (std::size_t i{}; i < header.size() - 1; i++) {
        out << header[i] << ": " << values[i] << ", ";
    }
    out << header.back() << ": " << values.back();
}

void ObservablesCollector::clearValues() {
    this->snapshotCycleNumbers.clear();
    for (auto &singleSet : this->snapshotValues)
        singleSet.clear();
}

void ObservablesCollector::printSnapshots(std::ostream &out) const {
    Expects(!this->observableHeader.empty());
    Expects(!this->observableHeader.front().empty());

    std::size_t numSnapshots = this->snapshotCycleNumbers.size();
    for (const auto &singleSet : this->snapshotValues)
        Assert(singleSet.size() == numSnapshots);

    out << "cycle ";
    std::copy(this->observableHeader.begin(), this->observableHeader.end(),
              std::ostream_iterator<std::string>(out, " "));
    out << std::endl;
    for (std::size_t i{}; i < numSnapshots; i++) {
        out << this->snapshotCycleNumbers[i] << " ";
        for (const auto &observableValues : this->snapshotValues)
            out << observableValues[i] << " ";
        out << std::endl;
    }
}

void ObservablesCollector::addAveragingValues(const Packing &packing, const ShapeTraits &shapeTraits) {
    this->addObservablesToContainer(packing, this->averagingValues, shapeTraits);
}

std::vector<ObservablesCollector::ObservableData> ObservablesCollector::getFlattenedAverageValues() const {
    std::vector<ObservableData> data(this->observableHeader.size());
    for (std::size_t i{}; i < data.size(); i++) {
        data[i].name = this->observableHeader[i];
        data[i].value.calculateFromSamples(this->averagingValues[i]);
    }
    return data;
}

std::vector<ObservablesCollector::ObservableGroupData> ObservablesCollector::getGroupedAverageValues() const {
    std::vector<ObservableGroupData> result;
    result.reserve(this->observables.size());
    auto averageValues = this->getFlattenedAverageValues();

    std::size_t index{};
    for (const auto &observable : this->observables) {
        auto header = observable->getHeader();
        Assert(index + header.size() <= averageValues.size());
        result.push_back({
            observable->getName(),
            std::vector<ObservableData>(averageValues.begin() + index, averageValues.begin() + index + header.size())
        });
        index += header.size();
    }
    Assert(index == averageValues.size());

    return result;
}

void ObservablesCollector::setThermodynamicParameters(double temperature_, double pressure_) {
    Expects(temperature_ > 0);
    Expects(pressure_ > 0);

    this->temperature = temperature_;
    this->pressure = pressure_;
}
