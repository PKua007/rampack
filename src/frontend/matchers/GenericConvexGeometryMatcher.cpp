//
// Created by Piotr Kubala on 01/06/2023.
//

#include "GenericConvexGeometryMatcher.h"


using namespace pyon::matcher;

namespace {
    MatcherAlternative create_script(const RecursiveMatcher &shapeRecursion);
    MatcherDataclass create_point();
    MatcherDataclass create_segment();
    MatcherDataclass create_rectangle();
    MatcherDataclass create_cuboid();
    MatcherDataclass create_disk();
    MatcherDataclass create_sphere();
    MatcherDataclass create_ellipse();
    MatcherDataclass create_ellipsoid();
    MatcherDataclass create_saucer();
    MatcherDataclass create_football();
    MatcherDataclass create_sum(const RecursiveMatcher &shapeRecursion);
    MatcherDataclass create_diff(const RecursiveMatcher &shapeRecursion);
    MatcherDataclass create_wrap(const RecursiveMatcher &shapeRecursion);
    bool is_non_zero(const std::array<double, 3> &array);


    auto position = MatcherArray(MatcherFloat{}, 3).mapToVector<3>();
    auto rotation = MatcherArray(MatcherFloat{}, 3).mapToStdArray<double, 3>();


    MatcherDataclass create_point() {
        return MatcherDataclass("point")
            .arguments({{"pos", position}})
            .mapTo([](const DataclassData &point) -> XCBodyBuilderScript {
                auto pos = point["pos"].as<Vector<3>>();
                return [pos](XCBodyBuilder &builder) {
                    builder.point(pos[0], pos[1], pos[2]);
                };
            });
    }

    MatcherDataclass create_segment() {
        return MatcherDataclass("segment")
            .arguments({{"l", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &segment) -> XCBodyBuilderScript {
                auto length = segment["l"].as<double>();
                auto pos = segment["pos"].as<Vector<3>>();
                auto rot = segment["rot"].as<std::array<double, 3>>();
                return [length, pos, rot](XCBodyBuilder &builder) {
                    builder.segment(length);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_rectangle() {
        return MatcherDataclass("rectangle")
            .arguments({{"ax", MatcherFloat{}.positive()},
                        {"ay", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &rectangle) -> XCBodyBuilderScript {
                auto sideX = rectangle["ax"].as<double>();
                auto sideY = rectangle["ay"].as<double>();
                auto pos = rectangle["pos"].as<Vector<3>>();
                auto rot = rectangle["rot"].as<std::array<double, 3>>();
                return [sideX, sideY, pos, rot](XCBodyBuilder &builder) {
                    builder.rectangle(sideX, sideY);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_cuboid() {
        return MatcherDataclass("cuboid")
            .arguments({{"ax", MatcherFloat{}.positive()},
                        {"ay", MatcherFloat{}.positive()},
                        {"az", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &cuboid) -> XCBodyBuilderScript {
                auto sideX = cuboid["ax"].as<double>();
                auto sideY = cuboid["ay"].as<double>();
                auto sideZ = cuboid["az"].as<double>();
                auto pos = cuboid["pos"].as<Vector<3>>();
                auto rot = cuboid["rot"].as<std::array<double, 3>>();
                return [sideX, sideY, sideZ, pos, rot](XCBodyBuilder &builder) {
                    builder.cuboid(sideX, sideY, sideZ);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_disk() {
        return MatcherDataclass("disk")
            .arguments({{"r", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &disk) -> XCBodyBuilderScript {
                auto radius = disk["r"].as<double>();
                auto pos = disk["pos"].as<Vector<3>>();
                auto rot = disk["rot"].as<std::array<double, 3>>();
                return [radius, pos, rot](XCBodyBuilder &builder) {
                    builder.disk(radius);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_sphere() {
        return MatcherDataclass("sphere")
            .arguments({{"r", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &sphere) -> XCBodyBuilderScript {
                auto radius = sphere["r"].as<double>();
                auto pos = sphere["pos"].as<Vector<3>>();
                return [radius, pos](XCBodyBuilder &builder) {
                    builder.sphere(radius);
                    builder.move(pos[0], pos[1], pos[2]);
                };
            });
    }

    MatcherDataclass create_ellipse() {
        return MatcherDataclass("ellipse")
            .arguments({{"rx", MatcherFloat{}.positive()},
                        {"ry", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &ellipse) -> XCBodyBuilderScript {
                auto semiAxisX = ellipse["rx"].as<double>();
                auto semiAxisY = ellipse["ry"].as<double>();
                auto pos = ellipse["pos"].as<Vector<3>>();
                auto rot = ellipse["rot"].as<std::array<double, 3>>();
                return [semiAxisX, semiAxisY, pos, rot](XCBodyBuilder &builder) {
                    builder.ellipse(semiAxisX, semiAxisY);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_ellipsoid() {
        return MatcherDataclass("ellipsoid")
            .arguments({{"rx", MatcherFloat{}.positive()},
                        {"ry", MatcherFloat{}.positive()},
                        {"rz", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &ellipsoid) -> XCBodyBuilderScript {
                auto semiAxisX = ellipsoid["rx"].as<double>();
                auto semiAxisY = ellipsoid["ry"].as<double>();
                auto semiAxisZ = ellipsoid["rz"].as<double>();
                auto pos = ellipsoid["pos"].as<Vector<3>>();
                auto rot = ellipsoid["rot"].as<std::array<double, 3>>();
                return [semiAxisX, semiAxisY, semiAxisZ, pos, rot](XCBodyBuilder &builder) {
                    builder.ellipsoid(semiAxisX, semiAxisY, semiAxisZ);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_saucer() {
        return MatcherDataclass("saucer")
            .arguments({{"r", MatcherFloat{}.positive()},
                        {"l", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .filter([](const DataclassData &football) {
                return football["l"].as<double>() <= 2*football["r"].as<double>();
            })
            .describe("l <= 2*r")
            .mapTo([](const DataclassData &saucer) -> XCBodyBuilderScript {
                auto radius = saucer["r"].as<double>();
                auto thickness = saucer["l"].as<double>();
                auto pos = saucer["pos"].as<Vector<3>>();
                auto rot = saucer["rot"].as<std::array<double, 3>>();
                return [radius, thickness, pos, rot](XCBodyBuilder &builder) {
                    builder.saucer(radius, thickness);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_football() {
        return MatcherDataclass("football")
            .arguments({{"r", MatcherFloat{}.positive()},
                        {"l", MatcherFloat{}.positive()},
                        {"pos", position, "[0, 0, 0]"},
                        {"rot", rotation, "[0, 0, 0]"}})
            .filter([](const DataclassData &football) {
                return football["l"].as<double>() >= 2*football["r"].as<double>();
            })
            .describe("l >= 2*r")
            .mapTo([](const DataclassData &football) -> XCBodyBuilderScript {
                auto radius = football["r"].as<double>();
                auto length = football["l"].as<double>();
                auto pos = football["pos"].as<Vector<3>>();
                auto rot = football["rot"].as<std::array<double, 3>>();
                return [radius, length, pos, rot](XCBodyBuilder &builder) {
                    builder.football(length, radius);
                    builder.move(pos[0], pos[1], pos[2]);
                    if (is_non_zero(rot))
                        builder.rot(rot[0], rot[1], rot[2]);
                };
            });
    }

    MatcherDataclass create_sum(const RecursiveMatcher &shapeRecursion) {
        return MatcherDataclass("sum")
            .variadicArguments(MatcherArray{}.elementsMatch(shapeRecursion).sizeAtLeast(2))
            .mapTo([](const DataclassData &sum) -> XCBodyBuilderScript {
                auto scripts = sum.getVariadicArguments().asStdVector<XCBodyBuilderScript>();
                return [scripts](XCBodyBuilder &builder) {
                    for (const auto &script : scripts)
                        script(builder);
                    builder.sum(scripts.size());
                };
            });
    }

    MatcherDataclass create_diff(const RecursiveMatcher &shapeRecursion) {
        return MatcherDataclass("diff")
            .arguments({{"g1", shapeRecursion},
                        {"g2", shapeRecursion}})
            .mapTo([](const DataclassData &diff) -> XCBodyBuilderScript {
                auto script1 = diff["g1"].as<XCBodyBuilderScript>();
                auto script2 = diff["g2"].as<XCBodyBuilderScript>();
                return [script1, script2](XCBodyBuilder &builder) {
                    script1(builder);
                    script2(builder);
                    builder.diff();
                };
            });
    }

    MatcherDataclass create_wrap(const RecursiveMatcher &shapeRecursion) {
        return MatcherDataclass("wrap")
            .variadicArguments(MatcherArray{}.elementsMatch(shapeRecursion).sizeAtLeast(2))
            .mapTo([](const DataclassData &wrap) -> XCBodyBuilderScript {
                auto scripts = wrap.getVariadicArguments().asStdVector<XCBodyBuilderScript>();
                return [scripts](XCBodyBuilder &builder) {
                    for (const auto &script : scripts)
                        script(builder);
                    builder.wrap(scripts.size());
                };
            });
    }
    
    MatcherAlternative create_script(const RecursiveMatcher &shapeRecursion) {
        return create_point()
            | create_segment()
            | create_rectangle()
            | create_cuboid()
            | create_disk()
            | create_sphere()
            | create_ellipse()
            | create_ellipsoid()
            | create_saucer()
            | create_football()
            | create_sum(shapeRecursion)
            | create_diff(shapeRecursion)
            | create_wrap(shapeRecursion);
    }

    bool is_non_zero(const std::array<double, 3> &array) {
        return array != std::array<double, 3>{0, 0, 0};
    }
}

RecursiveMatcher GenericConvexGeometryMatcher::shapeRecursion;
MatcherAlternative GenericConvexGeometryMatcher::script = GenericConvexGeometryMatcher::create();

MatcherAlternative GenericConvexGeometryMatcher::create() {
    auto matcher = create_script(GenericConvexGeometryMatcher::shapeRecursion);
    GenericConvexGeometryMatcher::shapeRecursion.attach(matcher);
    return matcher;
}

XCBodyBuilderScript GenericConvexGeometryMatcher::match(const std::string &expression) {
    auto scriptAST = pyon::Parser::parse(expression);
    Any writer;
    auto matchReport = GenericConvexGeometryMatcher::script.match(scriptAST, writer);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return writer.as<XCBodyBuilderScript>();
}
