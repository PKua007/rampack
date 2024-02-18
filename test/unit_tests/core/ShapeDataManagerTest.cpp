//
// Created by Piotr Kubala on 18/02/2024.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeDataManager.h"

#include "core/ShapeDataManager.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"

namespace {
    struct TheData {
        int defaultInt;
        double specificDouble;

        friend bool operator==(const TheData &lhs, const TheData &rhs) {
            return std::tie(lhs.defaultInt, rhs.specificDouble) == std::tie(rhs.defaultInt, rhs.specificDouble);
        }
    };

    TextualShapeData do_serialize(const ShapeData &data) {
        const auto &theData = data.as<TheData>();
        ShapeDataSerializer serializer;
        serializer["default_int"] = theData.defaultInt;
        serializer["specific_double"] = theData.specificDouble;
        return serializer.toTextualShapeData();
    }

    ShapeData do_deserialize(const TextualShapeData &textData) {
        ShapeDataDeserializer deserializer(textData);
        TheData theData{deserializer.as<int>("default_int"), deserializer.as<double>("specific_double")};
        deserializer.throwIfNotAccessed();
        return ShapeData(theData);
    }
}


TEST_CASE("ShapeDataManager") {
    using trompeloeil::_;

    MockShapeDataManager manager({{"default_int", "5"}});
    ALLOW_CALL(manager, serialize(_)).RETURN(do_serialize(_1));
    ALLOW_CALL(manager, deserialize(_)).RETURN(do_deserialize(_1));

    SECTION("defaultSerialize") {
        SECTION("identical with default") {
            auto data = manager.defaultSerialize(ShapeData{TheData{5, 2.5}});

            CHECK(data == std::map<std::string, std::string>{{"specific_double", "2.5"}});
        }

        SECTION("not identical with default") {
            auto data = manager.defaultSerialize(ShapeData{TheData{10, 2.5}});

            CHECK(data == std::map<std::string, std::string>{{"default_int", "10"}, {"specific_double", "2.5"}});
        }
    }

    SECTION("defaultDeserialize") {
        SECTION("use default") {
            auto data = manager.defaultDeserialize({{"specific_double", "2.5"}});

            CHECK(data == ShapeData{TheData{5, 2.5}});
        }

        SECTION("override default") {
            auto data = manager.defaultDeserialize({{"default_int", "10"}, {"specific_double", "2.5"}});

            CHECK(data == ShapeData{TheData{10, 2.5}});
        }
    }
}