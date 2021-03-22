//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_OBSERVABLESCOLLECTOR_H
#define RAMPACK_OBSERVABLESCOLLECTOR_H

#include <memory>
#include <vector>
#include <iosfwd>

#include "Observable.h"
#include "utils/Quantity.h"

class ObservablesCollector {
private:
    std::vector<std::unique_ptr<Observable>> observables;
    std::vector<std::string> observableHeader;
    std::vector<std::size_t> snapshotCycleNumbers;
    std::vector<std::vector<double>> snapshotValues;
    std::vector<std::vector<double>> averagingValues;
    std::vector<std::size_t> inlineObservableIndices;

    void printInlineObservable(unsigned long observableIdx, const Packing &packing, std::ostringstream &out) const;
    void addObservablesToContainer(const Packing &packing, std::vector<std::vector<double>> &container);

public:
    struct ObservableDescription {
        std::string observableName;
        std::string observableValues;
    };

    void addObservable(std::unique_ptr<Observable> observable, bool shouldDisplayInline);

    void addSnapshot(const Packing &packing, std::size_t cycleNumber);
    void addAveragingValues(const Packing &packing);
    void clearValues();

    void printSnapshots(std::ostream &out) const;
    [[nodiscard]] std::string generateInlineObservablesString(const Packing &packing) const;
    [[nodiscard]] std::vector<ObservableDescription> generateObservablesAverageValueDescription() const;
    [[nodiscard]] const std::vector<std::string> &getObservableHeader() const { return this->observableHeader; }
    [[nodiscard]] std::vector<Quantity> getAverageValues() const;
};


#endif //RAMPACK_OBSERVABLESCOLLECTOR_H
