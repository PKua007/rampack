//
// Created by Piotr Kubala on 03/01/2023.
//

#include <utility>
#include <fstream>

#include "ArrangementMatcher.h"
#include "LatticeMatcher.h"
#include "frontend/PackingFactory.h"
#include "utils/Assertions.h"

using namespace pyon::matcher;


namespace {
    class PresimulatedPackingFactory : public PackingFactory {
    private:
        std::string filename;

    public:
        explicit PresimulatedPackingFactory(std::string filename) : filename{std::move(filename)} { }

        std::unique_ptr<Packing> createPacking(std::unique_ptr<BoundaryConditions> bc, const ShapeTraits &shapeTraits,
                                               std::size_t moveThreads, std::size_t scalingThreads) override
        {
            std::ifstream packingFile(this->filename);
            ValidateOpenedDesc(packingFile, this->filename, "to load initial configuration");

            auto packing = std::make_unique<Packing>(std::move(bc), moveThreads, scalingThreads);
            packing->restore(packingFile, shapeTraits.getInteraction());
            return packing;
        }
    };

    MatcherDataclass create_presimulated() {
        return MatcherDataclass("presimulated")
            .arguments({{"file", MatcherString{}.nonEmpty()}})
            .mapTo([](const DataclassData &presimulated) -> std::shared_ptr<PackingFactory> {
                auto filename = presimulated["file"].as<std::string>();
                return std::make_shared<PresimulatedPackingFactory>(filename);
            });
    }
}


MatcherAlternative ArrangementMatcher::create() {
    return create_presimulated() | LatticeMatcher::create();
}
