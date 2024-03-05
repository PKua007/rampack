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
    // Helper class for managing and validating on-the-fly output channels
    class OnTheFlyOutput {
    private:
        std::size_t absoluteCyclesNumber{};
        std::size_t roundedCyclesNumber{};
        std::size_t snapshotEvery{};
        bool isContinuation{};
        Logger &logger;

        void attachSnapshotOut(const std::string &filename) const;
        void verifyLastCycleNumbers(const std::vector<std::pair<std::string, std::size_t>> &lastCycleNumbers) const;
        void throwInconsistencyException(const std::vector<std::pair<std::string, std::size_t>> &lastCycleNumbers) const;

    public:
        std::shared_ptr<ObservablesCollector> collector;
        std::vector<std::unique_ptr<SimulationRecorder>> recorders;

        OnTheFlyOutput(const SnapshotCollectorRun &run, std::size_t numParticles, std::size_t absoluteCyclesNumber,
                       bool isContinuation, Logger &logger);
    };


    [[nodiscard]] Simulation::Environment recreateEnvironment(const RampackParameters &params, const PackingLoader &loader) const;
    void verifyDynamicParameter(const DynamicParameter &dynamicParameter, const std::string &parameterName,
                                const IntegrationRun &run, std::size_t cycleOffset) const;
    void performIntegration(Simulation &simulation, const Simulation::Environment &env, const IntegrationRun &run,
                            const ShapeTraits &shapeTraits, std::size_t cycleOffset, bool isContinuation);
    void performOverlapRelaxation(Simulation &simulation, const Simulation::Environment &env, const OverlapRelaxationRun &run,
                                  std::shared_ptr<ShapeTraits> shapeTraits, std::size_t cycleOffset,
                                  bool isContinuation);
    void performTransformationRun(Simulation &simulation, const TransformationRun &run, const ShapeTraits &shapeTraits);
    void overwriteMoveStepSizes(Simulation::Environment &env,
                                const std::map<std::string, std::string> &packingAuxInfo) const;
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
