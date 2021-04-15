//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_OBSERVABLESCOLLECTOR_H
#define RAMPACK_OBSERVABLESCOLLECTOR_H

#include <memory>
#include <vector>
#include <iosfwd>

#include "ShapeTraits.h"
#include "Observable.h"
#include "utils/Quantity.h"

class ObservablesCollector {
private:
    double temperature{};
    double pressure{};
    std::size_t cycleOffset{};

    std::vector<std::unique_ptr<Observable>> observables;
    std::vector<std::string> observableHeader;
    std::vector<std::size_t> inlineObservablesIndices;
    std::vector<std::size_t> snapshotCycleNumbers;
    std::vector<std::vector<double>> snapshotValues;
    std::vector<std::vector<double>> averagingValues;

    void printInlineObservable(unsigned long observableIdx, const Packing &packing, const ShapeTraits &shapeTraits,
                               std::ostringstream &out) const;
    void addObservablesToContainer(const Packing &packing, const ShapeTraits &shapeTraits,
                                   std::vector<std::vector<double>> &container);

public:
    struct ObservableData {
        std::string name;
        Quantity quantity;
    };

    struct ObservableGroupData {
        std::string groupName;
        std::vector<ObservableData> observableData;
    };

    void addObservable(std::unique_ptr<Observable> observable, bool shouldDisplayInline);
    void setThermodynamicParameters(double temperature_, double pressure_);
    void setCycleOffset(std::size_t offset);

    void addSnapshot(const Packing &packing, std::size_t cycleNumber, const ShapeTraits &shapeTraits);
    void addAveragingValues(const Packing &packing, const ShapeTraits &shapeTraits);
    [[nodiscard]] std::string generateInlineObservablesString(const Packing &packing,
                                                              const ShapeTraits &shapeTraits) const;
    void clearValues();

    void printSnapshots(std::ostream &out, bool printHeader = true) const;
    [[nodiscard]] std::vector<ObservableData> getFlattenedAverageValues() const;
    [[nodiscard]] std::vector<ObservableGroupData> getGroupedAverageValues() const;

    [[nodiscard]] std::size_t getMemoryUsage() const;
};


#endif //RAMPACK_OBSERVABLESCOLLECTOR_H
