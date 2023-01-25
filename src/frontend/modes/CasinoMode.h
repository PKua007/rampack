//
// Created by Piotr Kubala on 25/01/2023.
//

#ifndef RAMPACK_CASINOMODE_H
#define RAMPACK_CASINOMODE_H

#include <memory>

#include "frontend/ModeBase.h"
#include "frontend/RampackParameters.h"
#include "frontend/PackingLoader.h"


class CasinoMode : public ModeBase {
private:
    [[nodiscard]] Simulation::Environment recreateEnvironment(const RampackParameters &params, const PackingLoader &loader) const;
    void verifyDynamicParameter(const DynamicParameter &dynamicParameter, const std::string &parameterName,
                                const IntegrationRun &run, std::size_t cycleOffset) const;
    void performIntegration(Simulation &simulation, Simulation::Environment &env, const IntegrationRun &run,
                            const ShapeTraits &shapeTraits, std::size_t cycleOffset, bool isContinuation);
    void performOverlapRelaxation(Simulation &simulation, Simulation::Environment &env, const OverlapRelaxationRun &run,
                                  std::shared_ptr<ShapeTraits> shapeTraits, std::size_t cycleOffset,
                                  bool isContinuation);
    void overwriteMoveStepSizes(Simulation::Environment &env,
                                const std::map<std::string, std::string> &packingAuxInfo) const;
    void attachSnapshotOut(ObservablesCollector &collector, const std::string& filename, bool isContinuation) const;
    void printPerformanceInfo(const Simulation &simulation);
    void printAverageValues(const ObservablesCollector &collector);
    void printMoveStatistics(const Simulation &simulation) const;
    static std::unique_ptr<Packing> recreatePacking(PackingLoader &loader, const BaseParameters &params,
                                                    const ShapeTraits &traits, std::size_t maxThreads);
    static std::string formatMoveKey(const std::string &groupName, const std::string &moveName);
    static bool isStepSizeKey(const std::string &key);

public:
    explicit CasinoMode(Logger &logger) : ModeBase(logger) { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_CASINOMODE_H
