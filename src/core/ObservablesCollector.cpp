//
// Created by Piotr Kubala on 22/03/2021.
//

#include <sstream>
#include <ostream>
#include <iterator>

#include "ObservablesCollector.h"
#include "utils/Utils.h"

void ObservablesCollector::addObservable(std::unique_ptr<Observable> observable, std::size_t observableType) {
    if (!this->snapshotValues.empty())
        ExpectsMsg(snapshotValues.front().empty(), "Cannot add a new observable if snapshots are already captured");
    if (!this->averagingValues.empty())
        ExpectsMsg(averagingValues.front().empty(), "Cannot add a new observable if snapshots are already captured");

    auto intervalHeader = observable->getIntervalHeader();
    auto nominalHeader = observable->getNominalHeader();

    this->observables.push_back(std::move(observable));
    std::size_t observableIndex = this->observables.size() - 1;

    if (observableType & ObservableType::SNAPSHOT) {
        this->snapshotHeader.insert(this->snapshotHeader.end(), intervalHeader.begin(), intervalHeader.end());
        this->snapshotHeader.insert(this->snapshotHeader.end(), nominalHeader.begin(), nominalHeader.end());
        this->snapshotValues.resize(this->snapshotHeader.size());
        this->snapshotObservablesIndices.push_back(observableIndex);
    }

    if (observableType & ObservableType::AVERAGING) {
        this->averagingHeader.insert(this->averagingHeader.end(), intervalHeader.begin(), intervalHeader.end());
        this->averagingValues.resize(this->averagingHeader.size());
        this->averagingObservablesIndices.push_back(observableIndex);
    }

    if (observableType & ObservableType::INLINE)
        this->inlineObservablesIndices.push_back(observableIndex);
}

void ObservablesCollector::addSnapshot(const Packing &packing, std::size_t cycleNumber, const ShapeTraits &shapeTraits)
{
    this->snapshotCycleNumbers.push_back(cycleNumber + this->cycleOffset);
    std::size_t valueIndex{};
    for (std::size_t observableIndex : this->snapshotObservablesIndices) {
        auto &observable = *this->observables[observableIndex];
        observable.calculate(packing, this->temperature, this->pressure, shapeTraits);
        auto intervalValues = observable.getIntervalValues();
        for (double value : intervalValues) {
            Assert(valueIndex < this->snapshotValues.size());
            std::ostringstream ostr;
            ostr.precision(std::numeric_limits<double>::max_digits10);
            ostr << value;
            this->snapshotValues[valueIndex].push_back(ostr.str());
            valueIndex++;
        }

        auto nominalValues = observable.getNominalValues();
        for (const auto &value : nominalValues) {
            Assert(valueIndex < this->snapshotValues.size());
            this->snapshotValues[valueIndex].push_back(value);
            valueIndex++;
        }
    }
    Assert(valueIndex == this->snapshotValues.size());
}

void ObservablesCollector::addAveragingValues(const Packing &packing, const ShapeTraits &shapeTraits) {
    std::size_t valueIndex{};
    for (std::size_t observableIndex : this->averagingObservablesIndices) {
        auto &observable = *this->observables[observableIndex];
        observable.calculate(packing, this->temperature, this->pressure, shapeTraits);
        auto values = observable.getIntervalValues();
        for (double value : values) {
            Assert(valueIndex < this->averagingValues.size());
            this->averagingValues[valueIndex].push_back(value);
            valueIndex++;
        }
    }
    Assert(valueIndex == this->averagingValues.size());
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
    auto intervalHeader = observable.getIntervalHeader();
    auto intervalValues = observable.getIntervalValues();
    auto nominalHeader = observable.getNominalHeader();
    auto nominalValues = observable.getNominalValues();
    std::size_t totalValues = intervalHeader.size() + nominalHeader.size();
    Assert(totalValues > 0);
    Assert(intervalHeader.size() == intervalValues.size());
    Assert(nominalHeader.size() == nominalValues.size());

    std::size_t valueNum = 1;
    for (std::size_t i{}; i < intervalHeader.size(); i++) {
        out << intervalHeader[i] << ": " << intervalValues[i];
        if (valueNum < totalValues)
            out << ", ";
        valueNum++;
    }

    for (std::size_t i{}; i < nominalHeader.size(); i++) {
        out << nominalHeader[i] << ": " << nominalValues[i];
        if (valueNum < totalValues)
            out << ", ";
        valueNum++;
    }
}

void ObservablesCollector::clearValues() {
    this->snapshotCycleNumbers.clear();
    for (auto &singleSet : this->snapshotValues)
        singleSet.clear();
    for (auto &singleSet : this->averagingValues)
        singleSet.clear();
}

void ObservablesCollector::printSnapshots(std::ostream &out, bool printHeader) const {
    Expects(!this->snapshotValues.empty());

    // Consistency check - all observables should have the same number of entries as number of recorder cycles
    std::size_t numSnapshots = this->snapshotCycleNumbers.size();
    for (const auto &singleSet : this->snapshotValues)
        Assert(singleSet.size() == numSnapshots);

    if (printHeader) {
        out << "cycle ";
        std::copy(this->snapshotHeader.begin(), this->snapshotHeader.end(),
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
    std::vector<ObservableData> flatValues(this->averagingHeader.size());
    for (std::size_t i{}; i < flatValues.size(); i++) {
        flatValues[i].name = this->averagingHeader[i];
        flatValues[i].quantity.calculateFromSamples(this->averagingValues[i]);
    }
    return flatValues;
}

std::vector<ObservablesCollector::ObservableGroupData> ObservablesCollector::getGroupedAverageValues() const {
    std::vector<ObservableGroupData> groupedValues;
    groupedValues.reserve(this->averagingObservablesIndices.size());
    auto flatValues = this->getFlattenedAverageValues();

    std::size_t flatIndex{};
    for (std::size_t observableIdx : this->averagingObservablesIndices) {
        const auto &observable = *this->observables[observableIdx];
        auto header = observable.getIntervalHeader();
        Assert(flatIndex + header.size() <= flatValues.size());
        groupedValues.push_back({
            observable.getName(),
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
