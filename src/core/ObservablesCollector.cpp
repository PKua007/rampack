//
// Created by Piotr Kubala on 22/03/2021.
//

#include <sstream>
#include <ostream>
#include <iterator>

#include "ObservablesCollector.h"

void ObservablesCollector::addObservable(std::unique_ptr<Observable> observable, bool shouldDisplayInline) {
    ExpectsMsg(this->snapshotValues.empty(), "Cannot add a new observable if snapshots are already captured");

    auto header = observable->getHeader();
    this->observableHeader.insert(this->observableHeader.end(), header.begin(), header.end());
    this->snapshotValues.resize(this->snapshotValues.size() + header.size());
    this->averagingValues.resize(this->averagingValues.size() + header.size());
    this->observables.push_back(std::move(observable));
    if (shouldDisplayInline)
        this->inlineObservableIndices.push_back(this->observables.size() - 1);
}

void ObservablesCollector::addSnapshot(const Packing &packing, std::size_t cycleNumber) {
    this->snapshotCycleNumbers.push_back(cycleNumber);
    this->addObservablesToContainer(packing, this->snapshotValues);
}

void ObservablesCollector::addObservablesToContainer(const Packing &packing, std::vector<std::vector<double>> &container) {
    std::size_t valueIndex{};
    for (auto &observable : observables) {
        observable->calculate(packing);
        auto values = observable->getValues();
        for (double value : values) {
            Assert(valueIndex < container.size());
            container[valueIndex].push_back(value);
        }
    }
    Assert(valueIndex == container.size());
}

std::string ObservablesCollector::generateInlineObservablesString(const Packing &packing) const {
    if (this->inlineObservableIndices.empty())
        return "";

    std::ostringstream stream;
    for (std::size_t i{}; i < this->inlineObservableIndices.size() - 1; i++) {
        this->printInlineObservable(this->inlineObservableIndices[i], packing, stream);
        stream << ", ";
    }
    this->printInlineObservable(this->inlineObservableIndices.back(), packing, stream);

    return stream.str();
}

void ObservablesCollector::printInlineObservable(unsigned long observableIdx, const Packing &packing,
                                                 std::ostringstream &out) const
{
    auto &observable = *this->observables[observableIdx];
    observable.calculate(packing);
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

void ObservablesCollector::printSnapshots(std::ostream &out) {
    std::size_t numSnapshots = this->snapshotCycleNumbers.size();
    for (const auto &singleSet : this->snapshotValues)
        Assert(singleSet.size() == numSnapshots);

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

void ObservablesCollector::addAveragingValues(const Packing &packing) {
    this->addObservablesToContainer(packing, this->averagingValues);
}

std::vector<Quantity> ObservablesCollector::getAverageValues() const {
    std::vector<Quantity> quantities(this->observableHeader.size());
    for (std::size_t i{}; i < quantities.size(); i++)
        quantities[i].calculateFromSamples(this->averagingValues[i]);
    return quantities;
}
