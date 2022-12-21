//
// Created by Piotr Kubala on 12/12/2020.
//

#include <sstream>

#include "ShapeFactory.h"

#include "utils/Assertions.h"
#include "utils/ParseUtils.h"
#include "utils/Utils.h"

#include "core/shapes/SphereTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/PolysphereBananaTraits.h"
#include "core/shapes/KMerTraits.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/shapes/PolysphereLollipopTraits.h"
#include "core/shapes/PolysphereWedgeTraits.h"
#include "core/shapes/SmoothWedgeTraits.h"
#include "core/shapes/GenericXenoCollideTraits.h"

#include "core/interactions/LennardJonesInteraction.h"
#include "core/interactions/RepulsiveLennardJonesInteraction.h"
#include "core/interactions/SquareInverseCoreInteraction.h"

#include "geometry/xenocollide/XCBodyBuilder.h"


#define GENERIC_XENO_COLLIDE_USAGE "Malformed GenericXenoCollide attributes. Usage:\n" \
                                   "    [param 1] [value 1] ... [param n] [value n]\n" \
                                   "Params can be in any order, but all of the following have to be specified\n" \
                                   "    primaryAxis [x] [y] [z]\n" \
                                   "    secondaryAxis [x] [y] [z]\n" \
                                   "    geometricOrigin [x] [y] [z]\n" \
                                   "    volume [volume]\n" \
                                   "    script [&-separated  commands] \n" \
                                   "There is also one optional\n" \
                                   "    namedPoints [name 1] [x1] [y1] [z1] [name 2] [x2] [y2] [z2] ..."

#define GENERIC_POLYSPHERE_USAGE "Malformed Polysphere attributes. Usage:\n" \
                                 "    [param 1] [value 1] ... [param n] [value n]\n" \
                                 "Params can be in any order, but all of the following have to be specified\n" \
                                 "    primaryAxis [x] [y] [z]\n" \
                                 "    secondaryAxis [x] [y] [z]\n" \
                                 "    geometricOrigin [x] [y] [z]\n" \
                                 "    volume [volume]\n" \
                                 "    spheres [x1] [y1] [z1] [r1] [x2] [y2] [z2] [r2] ... \n" \
                                 "There is also one optional\n" \
                                 "    namedPoints [name 1] [x1] [y1] [z1] [name 2] [x2] [y2] [z2] ..."

#define GENERIC_POLYSPHEROCYLINDER_USAGE \
    "Malformed Polyspherocylnder attributes. Usage:\n" \
    "    [param 1] [value 1] ... [param n] [value n]\n" \
    "Params can be in any order, but all of the following have to be specified\n" \
    "    primaryAxis [x] [y] [z]\n" \
    "    secondaryAxis [x] [y] [z]\n" \
    "    geometricOrigin [x] [y] [z]\n" \
    "    volume [volume]\n" \
    "and at least one of the following\n" \
    "    spherocylinders [spherocylinder spec 1] [spherocylinder spec 2] ...\n" \
    "    chain [r] [x1] [y1] [z1] [x2] [y2] [z2] ...\n" \
    "where\n" \
    "    [spherocylinder spec i] := [ri] [beg xi] [beg yi] [beg zi] [end xi] [end yi] [end zi]\n" \
    "There is also one optional\n" \
    "    namedPoints [name 1] [x1] [y1] [z1] [name 2] [x2] [y2] [z2] ..."


namespace {
    using SpherocylinderData = PolyspherocylinderTraits::SpherocylinderData;


    template <typename ConcreteTraits, typename... Args>
    auto parse_polysphere_traits(const std::string &shapeName, const std::string &interactionName,
                                 std::istream &interactionAttrStream, Args&&... args)
    {
        if (interactionName.empty() || interactionName == "hard") {
            return std::make_shared<ConcreteTraits>(std::forward<Args>(args)...);
        } else if (interactionName == "lj") {
            double epsilon, sigma;
            interactionAttrStream >> epsilon >> sigma;
            ValidateMsg(interactionAttrStream, "Malformed Lennard Jones attributes. Usage: lj [epsilon] [sigma]");
            Validate(epsilon > 0);
            Validate(sigma > 0);
            return std::make_shared<ConcreteTraits>(
                std::forward<Args>(args)..., std::make_unique<LennardJonesInteraction>(epsilon, sigma)
            );
        } else if (interactionName == "repulsive_lj") {
            double epsilon, sigma;
            interactionAttrStream >> epsilon >> sigma;
            ValidateMsg(interactionAttrStream, "Malformed repulsive Lennard Jones attributes. Usage: lj_repulsive "
                                               "[epsilon] [sigma]");
            Validate(epsilon > 0);
            Validate(sigma > 0);
            return std::make_shared<ConcreteTraits>(
                    std::forward<Args>(args)..., std::make_unique<RepulsiveLennardJonesInteraction>(epsilon, sigma)
            );
        } else if (interactionName == "square_inverse_core") {
            double epsilon, sigma;
            interactionAttrStream >> epsilon >> sigma;
            ValidateMsg(interactionAttrStream, "Malformed square inverse core attributes. Usage: square_inverse_core "
                                               "[epsilon] [sigma]");
            Validate(epsilon != 0);
            Validate(sigma > 0);
            return std::make_shared<ConcreteTraits>(
                    std::forward<Args>(args)..., std::make_unique<SquareInverseCoreInteraction>(epsilon, sigma)
            );
        } else {
            throw ValidationException(shapeName + " supports interactions: hard, lj (Lennard Jones), repulsive_lj "
                                                  "(Lennard Jones cut at the minimum), square_inverse_core "
                                                  "(dipole-like short-range interaction)");
        }
    }

    bool has_all_fields(const std::map<std::string, std::string> &fieldsMap, const std::vector<std::string> &fields) {
        return std::all_of(fields.begin(), fields.end(), [&fieldsMap](const auto &field) {
            return fieldsMap.find(field) != fieldsMap.end();
        });
    }

    Vector<3> parse_vector(const std::string &coords) {
        std::istringstream coordsStream(coords);
        Vector<3> v;
        coordsStream >> v[0] >> v[1] >> v[2];
        ValidateMsg(coordsStream, "Malformed vector coordinates");
        return v;
    }

    Vector<3> parse_axis(const std::string &coords) {
        Vector<3> axis = parse_vector(coords);
        ValidateMsg(axis.norm() >= 1e-8, "Axis has too small norm - at least 10^(-8) is required");
        return axis.normalized();
    }

    ShapeGeometry::NamedPoints parse_named_points(const std::string &pointsStr) {
        ShapeGeometry::NamedPoints namedPoints;
        std::istringstream pointsStream(pointsStr);
        while (pointsStream.good()) {
            std::string name;
            Vector<3> pos;
            pointsStream >> name >> pos[0] >> pos[1] >> pos[2];
            ValidateMsg(pointsStream, "Malformed named point");
            namedPoints.emplace_back(name, pos);
            if (pointsStream.good())
                pointsStream >> std::ws;
        }
        return namedPoints;
    }

    auto parse_generic_shape_traits(std::istream &in, const std::string &usage,
                                    const std::vector<std::string> &requiredCustomFields,
                                    const std::vector<std::string> &optionalCustomFields = {})
    {
        std::vector<std::string> requiredFields{"primaryAxis", "secondaryAxis", "geometricOrigin", "volume"};
        requiredFields.insert(requiredFields.end(), requiredCustomFields.begin(), requiredCustomFields.end());
        std::vector<std::string> allFields = requiredFields;
        allFields.emplace_back("namedPoints");
        allFields.insert(allFields.end(), optionalCustomFields.begin(), optionalCustomFields.end());
        auto fieldsMap = ParseUtils::parseFields(allFields, in);
        ValidateMsg(has_all_fields(fieldsMap, requiredFields), usage);

        Vector<3> primaryAxis = parse_axis(fieldsMap.at("primaryAxis"));
        Vector<3> secondaryAxis = parse_axis(fieldsMap.at("secondaryAxis"));
        Vector<3> geometricOrigin = parse_vector(fieldsMap.at("geometricOrigin"));
        double volume = std::stod(fieldsMap.at("volume"));
        Validate(volume > 0);
        ShapeGeometry::NamedPoints namedPoints;
        if (fieldsMap.find("namedPoints") != fieldsMap.end())
            namedPoints = parse_named_points(fieldsMap.at("namedPoints"));

        return std::make_tuple(fieldsMap, primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints);
    }

    std::shared_ptr<ShapeTraits> parse_generic_xeno_collide(std::istream &in) {
        auto [fieldsMap, primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints]
            = parse_generic_shape_traits(in, GENERIC_XENO_COLLIDE_USAGE, {"script"});
        auto commands = explode(fieldsMap.at("script"), '&');
        ValidateMsg(!commands.empty(), "At least one script command should be passed");

        XCBodyBuilder builder;
        for (const auto &command : commands)
            builder.processCommand(command);

        auto collideGeometry = builder.releaseCollideGeometry();
        return std::make_shared<GenericXenoCollideTraits>(
            std::move(collideGeometry), primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints
        );
    }

    PolysphereTraits::PolysphereGeometry parse_polysphere_geometry(std::istream &in) {
        auto [fieldsMap, primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints]
            = parse_generic_shape_traits(in, GENERIC_POLYSPHERE_USAGE, {"spheres"});

        std::vector<PolysphereTraits::SphereData> sphereData;
        auto spheres = ParseUtils::tokenize<double>(fieldsMap.at("spheres"));
        ValidateMsg(!spheres.empty() && (spheres.size() % 4 == 0), GENERIC_POLYSPHERE_USAGE);
        for (std::size_t i{}; i < spheres.size(); i += 4) {
            double r = spheres[i];
            Vector<3> pos{spheres[i + 1], spheres[i + 2], spheres[i + 3]};
            ValidateMsg(r > 0, "Radius of sphere " + std::to_string(i/4) + " is <= 0");
            sphereData.emplace_back(pos, r);
        }
        return PolysphereTraits::PolysphereGeometry(
            std::move(sphereData), primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints
        );
    }

    void parse_chain(const std::string &chainStr, std::vector<SpherocylinderData> &spherocylinderData) {
        auto tokens = ParseUtils::tokenize<double>(chainStr);
        ValidateMsg((tokens.size() >= 7) && (tokens.size() % 3 == 1), GENERIC_POLYSPHEROCYLINDER_USAGE);

        double r = tokens[0];
        ValidateMsg(r > 0, "Chain radius is <= 0");
        std::vector<Vector<3>> chainPos;
        for (std::size_t i = 1; i < tokens.size(); i += 3)
            chainPos.push_back({tokens[i], tokens[i + 1], tokens[i + 2]});

        auto spherocylinderCreator = [r](const Vector<3> &beg, const Vector<3> &end) -> SpherocylinderData {
            Vector<3> pos = (beg + end)/2.;
            Vector<3> halfAxis = end - pos;
            return {pos, halfAxis, r};
        };
        std::transform(chainPos.begin(), std::prev(chainPos.end()), std::next(chainPos.begin()),
                       std::back_inserter(spherocylinderData), spherocylinderCreator);
    }

    void parse_spherocylinders(const std::string &spherocylindersStr,
                               std::vector<SpherocylinderData> &spherocylinderData)
    {
        auto tokens = ParseUtils::tokenize<double>(spherocylindersStr);
        ValidateMsg(!tokens.empty() && (tokens.size() % 7 == 0), GENERIC_POLYSPHEROCYLINDER_USAGE);

        for (std::size_t i{}; i < tokens.size(); i += 7) {
            double r = tokens[i];
            Vector<3> beg{tokens[i + 1], tokens[i + 2], tokens[i + 3]};
            Vector<3> end{tokens[i + 4], tokens[i + 5], tokens[i + 6]};
            Vector<3> pos = (beg + end)/2.;
            Vector<3> halfAxis = end - pos;
            ValidateMsg(r > 0, "Radius of spherocylinder " + std::to_string(i/7) + " is <= 0");
            spherocylinderData.emplace_back(pos, halfAxis, r);
        }
    }

    std::shared_ptr<ShapeTraits> parse_polyspherocylinder(std::istream &in) {
        auto [fieldsMap, primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints]
                = parse_generic_shape_traits(in, GENERIC_POLYSPHEROCYLINDER_USAGE, {}, {"chain", "spherocylinders"});

        std::vector<SpherocylinderData> spherocylinderData;
        if (fieldsMap.find("chain") != fieldsMap.end())
            parse_chain(fieldsMap.at("chain"), spherocylinderData);
        if (fieldsMap.find("spherocylinders") != fieldsMap.end())
            parse_spherocylinders(fieldsMap.at("spherocylinders"), spherocylinderData);
        ValidateMsg(!spherocylinderData.empty(), GENERIC_POLYSPHEROCYLINDER_USAGE);

        PolyspherocylinderTraits::PolyspherocylinderGeometry geometry(
            std::move(spherocylinderData), primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints
        );
        return std::make_unique<PolyspherocylinderTraits>(std::move(geometry));
    }

    std::shared_ptr<ShapeTraits> parse_polysphere_wedge(const std::string &shapeName,
                                                        const std::string &interactionName,
                                                        std::istringstream &shapeAttrStream,
                                                        std::istringstream &interactionAttrStream)
    {
        double bottomSphereRadius, topSphereRadius, spherePenetration;
        std::size_t sphereNum;
        shapeAttrStream >> sphereNum >> bottomSphereRadius >> topSphereRadius >> spherePenetration;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereWedge attributes; expected: "
                                     "[number of spheres] [bottom sphere radius] [top sphere radius] "
                                     "[spheres penetration]");
        Validate(sphereNum >= 2);
        Validate(bottomSphereRadius > 0);
        Validate(topSphereRadius > 0);
        Validate(spherePenetration < 2 * std::min(bottomSphereRadius, topSphereRadius));

        return parse_polysphere_traits<PolysphereWedgeTraits>(
            shapeName, interactionName, interactionAttrStream,
            sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration
        );
    }

    std::shared_ptr<ShapeTraits> parse_polysphere_wedge_legacy(const std::string &shapeName,
                                                               const std::string &interactionName,
                                                               std::istringstream &shapeAttrStream,
                                                               std::istringstream &interactionAttrStream)
    {
        double smallSphereRadius, largeSphereRadius, spherePenetration;
        std::size_t sphereNum;
        shapeAttrStream >> sphereNum >> smallSphereRadius >> largeSphereRadius >> spherePenetration;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereWedge attributes; expected: "
                                     "[number of spheres] [small sphere radius] [large sphere radius] "
                                     "[spheres penetration]");
        Validate(sphereNum >= 2);
        Validate(smallSphereRadius > 0);
        Validate(largeSphereRadius > 0);
        Validate(spherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

        return parse_polysphere_traits<legacy::PolysphereWedgeTraits>(
            shapeName, interactionName, interactionAttrStream,
            sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration
        );
    }
}


std::shared_ptr<ShapeTraits> ShapeFactory::shapeTraitsFor(const std::string &shapeName,
                                                          const std::string &shapeAttributes,
                                                          const std::string &interaction, const Version &version)
{
    std::istringstream shapeAttrStream(shapeAttributes);
    std::istringstream interactionAttrStream(interaction);
    std::string interactionName;
    interactionAttrStream >> interactionName;
    if (shapeName == "Sphere") {
        double r;
        shapeAttrStream >> r;
        ValidateMsg(shapeAttrStream, "Malformed Sphere attributes; expected: [radius]");
        Validate(r > 0);
        return parse_polysphere_traits<SphereTraits>(shapeName, interactionName, interactionAttrStream, r);
    } else if (shapeName == "PolysphereBanana") {
        double arcRadius, arcAngle, sphereRadius;
        std::size_t sphereNum;
        shapeAttrStream >> arcRadius >> arcAngle >> sphereNum >> sphereRadius;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereBanana attributes; expected: "
                                     "[arc radius] [arc angle] [number of spheres] [sphere radius]");
        Validate(arcRadius > 0);
        Validate(arcAngle > 0);
        Validate(sphereNum > 0);
        Validate(sphereRadius > 0);

        if (version >= Version{0, 2, 0}) {
            return parse_polysphere_traits<PolysphereBananaTraits>(shapeName, interactionName, interactionAttrStream,
                                                                   arcRadius, arcAngle, sphereNum, sphereRadius);
        } else {
            return parse_polysphere_traits<legacy::PolysphereBananaTraits>(
                shapeName, interactionName, interactionAttrStream,
                arcRadius, arcAngle, sphereNum, sphereRadius
            );
        }
    } else if (shapeName == "PolyspherocylinderBanana") {
        double arcRadius, arcAngle, radius;
        std::size_t segmentNum, subdivisions;
        shapeAttrStream >> arcRadius >> arcAngle >> segmentNum >> radius;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereBanana attributes; expected: "
                                     "[arc radius] [arc angle] [number of segments] [radius] (subdivisions = 1)");
        Validate(arcRadius > 0);
        Validate(arcAngle > 0);
        Validate(segmentNum > 0);
        Validate(radius > 0);

        shapeAttrStream >> subdivisions;
        if (!shapeAttrStream)
            subdivisions = 1;
        else
            Validate(subdivisions > 0);

        ValidateMsg(interactionName == "hard" || interactionName.empty(),
                    "SpherocylinderBanana supports only hard interactions");

        return std::make_shared<PolyspherocylinderBananaTraits>(arcRadius, arcAngle, segmentNum, radius, subdivisions);
    } else if (shapeName == "KMer") {
        double sphereRadius, distance;
        std::size_t sphereNum;
        shapeAttrStream >> sphereNum >> sphereRadius >> distance;
        ValidateMsg(shapeAttrStream, "Malformed KMer attributes; expected: "
                                     "[number of spheres] [sphere radius] [distance between spheres]");
        Validate(sphereNum >= 2);
        Validate(sphereRadius > 0);
        Validate(distance > 0);

        return parse_polysphere_traits<KMerTraits>(shapeName, interactionName, interactionAttrStream, sphereNum,
                                                   sphereRadius, distance);
    } else if (shapeName == "PolysphereLollipop") {
        double smallSphereRadius, largeSphereRadius, smallSpherePenetration, largeSpherePenetration;
        std::size_t sphereNum;
        shapeAttrStream >> sphereNum >> smallSphereRadius >> largeSphereRadius >> smallSpherePenetration;
        shapeAttrStream >> largeSpherePenetration;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereLollipop attributes; expected: "
                                     "[number of spheres] [small sphere radius] [large sphere radius] "
                                     "[small spheres penetration] [large sphere penetration]");
        Validate(sphereNum >= 2);
        Validate(smallSphereRadius > 0);
        Validate(largeSphereRadius > 0);
        Validate(smallSpherePenetration < 2*smallSphereRadius);
        Validate(largeSpherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

        if (version >= Version{0, 2, 0}) {
            return parse_polysphere_traits<PolysphereLollipopTraits>(
                shapeName, interactionName, interactionAttrStream,
                sphereNum, smallSphereRadius, largeSphereRadius, smallSpherePenetration, largeSpherePenetration
            );
        } else {
            return parse_polysphere_traits<legacy::PolysphereLollipopTraits>(
                shapeName, interactionName, interactionAttrStream,
                sphereNum, smallSphereRadius, largeSphereRadius, smallSpherePenetration, largeSpherePenetration
            );
        }
    } else if (shapeName == "PolysphereWedge") {
        if (version >= Version{0, 2, 0})
            return parse_polysphere_wedge(shapeName, interactionName, shapeAttrStream, interactionAttrStream);
        else
            return parse_polysphere_wedge_legacy(shapeName, interactionName, shapeAttrStream, interactionAttrStream);
    } else if (shapeName == "Spherocylinder") {
        double r, length;
        shapeAttrStream >> length >> r;
        ValidateMsg(shapeAttrStream, "Malformed Spherocylinder attributes; expected: [length] [radius]");
        Validate(r > 0);
        Validate(length >= 0);
        ValidateMsg(interactionName == "hard" || interactionName.empty(),
                    "Spherocylinder supports only hard interactions");
        return std::make_shared<SpherocylinderTraits>(length, r);
    } else if (shapeName == "SmoothWedge") {
        double R, r, length;
        shapeAttrStream >> length >> R >> r;
        ValidateMsg(shapeAttrStream, "Malformed SmoothWedge attributes; expected: [length] [bottom radius] "
                                     "[top radius] ([subdivisions = 1])");
        std::size_t subdivisions;
        shapeAttrStream >> subdivisions;
        if (!shapeAttrStream)
            subdivisions = 1;

        Validate(r > 0);
        Validate(R > 0);
        Validate(length >= std::abs(R - r));
        ValidateMsg(interactionName == "hard" || interactionName.empty(), "SmoothWedge supports only hard interactions");
        return std::make_shared<SmoothWedgeTraits>(R, r, length, subdivisions);
    } else if (shapeName == "GenericXenoCollide") {
        ValidateMsg(interactionName == "hard" || interactionName.empty(),
                    "GenericXenoCollide supports only hard interactions");
        return parse_generic_xeno_collide(shapeAttrStream);
    } else if (shapeName == "Polysphere") {
        auto geometry = parse_polysphere_geometry(shapeAttrStream);
        return parse_polysphere_traits<PolysphereTraits>(shapeName, interactionName, interactionAttrStream, geometry);
    } else if (shapeName == "Polyspherocylinder") {
        ValidateMsg(interactionName == "hard" || interactionName.empty(),
                    "Polyspherocylinder supports only hard interactions");
        return parse_polyspherocylinder(shapeAttrStream);
    } else {
        throw ValidationException("Unknown particle name: " + shapeName);
    }
}