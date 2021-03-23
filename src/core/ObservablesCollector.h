//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_OBSERVABLESCOLLECTOR_H
#define RAMPACK_OBSERVABLESCOLLECTOR_H

#include <memory>
#include <vector>
#include <iosfwd>

#include "Interaction.h"
#include "Observable.h"
#include "utils/Quantity.h"

class ObservablesCollector {
private:
    double temperature{};
    double pressure{};

    std::vector<std::unique_ptr<Observable>> observables;
    std::vector<std::string> observableHeader;
    std::vector<std::size_t> snapshotCycleNumbers;
    std::vector<std::vector<double>> snapshotValues;
    std::vector<std::vector<double>> averagingValues;
    std::vector<std::size_t> inlineObservableIndices;

    void printInlineObservable(unsigned long observableIdx, const Packing &packing, const Interaction &interaction,
                               std::ostringstream &out) const;
    void addObservablesToContainer(const Packing &packing, std::vector<std::vector<double>> &container,
                                   const Interaction &interaction);

public:
    struct ObservableDescription {
        std::string observableName;
        std::string observableValues;
    };

    void addObservable(std::unique_ptr<Observable> observable, bool shouldDisplayInline);
    void setThermodynamicParameters(double temperature_, double pressure_);

    void addSnapshot(const Packing &packing, std::size_t cycleNumber, const Interaction &interaction);
    void addAveragingValues(const Packing &packing, const Interaction &interaction);
    void clearValues();

    void printSnapshots(std::ostream &out) const;
    [[nodiscard]] std::string generateInlineObservablesString(const Packing &packing,
                                                              const Interaction &interaction) const;
    [[nodiscard]] std::vector<ObservableDescription> generateObservablesAverageValueDescription() const;
    [[nodiscard]] const std::vector<std::string> &getObservableHeader() const { return this->observableHeader; }
    [[nodiscard]] std::vector<Quantity> getAverageValues() const;
};


#endif //RAMPACK_OBSERVABLESCOLLECTOR_H
