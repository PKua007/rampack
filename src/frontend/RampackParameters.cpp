//
// Created by Piotr Kubala on 05/03/2024.
//

#include "RampackParameters.h"


const RunBase &RunBase::of(const Run &run) {
    return std::visit([](auto &&run) -> const RunBase & { return static_cast<const RunBase &>(run); }, run);
}

RunBase &RunBase::of(Run &run) {
    return const_cast<RunBase &>(RunBase::of(std::as_const(run)));
}

const SnapshotCollectorRun &SnapshotCollectorRun::of(const Run &run) {
    auto retriever = [](auto &&run) -> const SnapshotCollectorRun & {
        using RunType = std::decay_t<decltype(run)>;
        if constexpr (!std::is_convertible_v<RunType, SnapshotCollectorRun>) {
            std::string className = demangle(typeid(RunType).name());
            throw UnsupportedParametersSection("SnapshotCollectorRun is not a base of " + className);
        } else {
            return static_cast<const SnapshotCollectorRun &>(run);
        }
    };
    return std::visit(retriever, run);
}

SnapshotCollectorRun &SnapshotCollectorRun::of(Run &run) {
    return const_cast<SnapshotCollectorRun &>(SnapshotCollectorRun::of(std::as_const(run)));
}

void combine_environment(Simulation::Environment &env, const Run &run) {
    auto environmentGetter = [](const auto &run) {
        return run.environment;
    };
    auto runEnv = std::visit(environmentGetter, run);
    env.combine(runEnv);
}