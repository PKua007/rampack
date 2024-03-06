//
// Created by Piotr Kubala on 05/03/2024.
//

#include "RampackParameters.h"
#include "utils/Utils.h"


namespace {
    template <typename Base>
    class BaseAccess {
    public:
        [[nodiscard]] static const Base &of(const Run &run) {
            auto converter = [](auto &&run) -> const Base & {
                using RunType = std::decay_t<decltype(run)>;
                if constexpr (!std::is_convertible_v<RunType, Base>) {
                    std::string baseClassName = demangle(typeid(Base).name());
                    std::string runClassName = demangle(typeid(RunType).name());
                    throw BadParametersCast(baseClassName + " is not a base of " + runClassName);
                } else {
                    return static_cast<const Base &>(run);
                }
            };
            return std::visit(converter, run);
        }

        [[nodiscard]] static Base &of(Run &run) {
            return const_cast<Base &>(Base::of(std::as_const(run)));
        }

        [[nodiscard]] static bool isInstance(const Run &run) {
            auto baseChecker = [](auto &&run) {
                using RunType = std::decay_t<decltype(run)>;
                return std::is_convertible_v<RunType, Base>;
            };
            return std::visit(baseChecker, run);
        }
    };
}

const RunBase &RunBase::of(const Run &run) {
    return BaseAccess<RunBase>::of(run);
}

RunBase &RunBase::of(Run &run) {
    return BaseAccess<RunBase>::of(run);
}

const SimulatingRun &SimulatingRun::of(const Run &run) {
    return BaseAccess<SimulatingRun>::of(run);
}

SimulatingRun &SimulatingRun::of(Run &run) {
    return BaseAccess<SimulatingRun>::of(run);
}

bool SimulatingRun::isInstance(const Run &run) {
    return BaseAccess<SimulatingRun>::isInstance(run);
}

const IntegrationRun &IntegrationRun::of(const Run &run) {
    return BaseAccess<IntegrationRun>::of(run);
}

IntegrationRun &IntegrationRun::of(Run &run) {
    return BaseAccess<IntegrationRun>::of(run);
}

bool IntegrationRun::isInstance(const Run &run) {
    return BaseAccess<IntegrationRun>::isInstance(run);
}

const OverlapRelaxationRun &OverlapRelaxationRun::of(const Run &run) {
    return BaseAccess<OverlapRelaxationRun>::of(run);
}

OverlapRelaxationRun &OverlapRelaxationRun::of(Run &run) {
    return BaseAccess<OverlapRelaxationRun>::of(run);
}

bool OverlapRelaxationRun::isInstance(const Run &run) {
    return BaseAccess<OverlapRelaxationRun>::isInstance(run);
}

const TransformationRun &TransformationRun::of(const Run &run) {
    return BaseAccess<TransformationRun>::of(run);
}

TransformationRun &TransformationRun::of(Run &run) {
    return BaseAccess<TransformationRun>::of(run);
}

bool TransformationRun::isInstance(const Run &run) {
    return BaseAccess<TransformationRun>::isInstance(run);
}

void combine_environment(Simulation::Environment &env, const Run &run) {
    auto runEnv = RunBase::of(run).environment;
    env.combine(runEnv);
}
