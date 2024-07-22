//
// Created by Piotr Kubala on 22/07/2024.
//

#include "printers/PolydisperseXCObjShapePrinter.h"
#include "geometry/xenocollide/XCPrimitives.h"


// PolysphereTraits::HardInteraction ###################################################################################

template<typename ConcreteTraits>
bool PolysphereTraits<ConcreteTraits>::HardInteraction
    ::overlapBetween(const Vector<3> &pos1, [[maybe_unused]] const Matrix<3, 3> &orientation1, const std::byte *data1,
                     std::size_t idx1, const Vector<3> &pos2, [[maybe_unused]] const Matrix<3, 3> &orientation2,
                     const std::byte *data2, std::size_t idx2, const BoundaryConditions &bc) const
{
    const auto &concreteTraits = static_cast<const ConcreteTraits &>(this->traits);

    const auto &sphereData1 = concreteTraits.getSphereData(data1)[idx1];
    const auto &sphereData2 = concreteTraits.getSphereData(data2)[idx2];

    double r = sphereData1.radius + sphereData2.radius;
    return bc.getDistance2(pos1, pos2) < r * r;
}

template<typename ConcreteTraits>
std::vector<Vector<3>> PolysphereTraits<ConcreteTraits>::HardInteraction
    ::getInteractionCentres(const std::byte *data) const
{
    return this->traits.getInteractionCentres(data);
}

template<typename ConcreteTraits>
double PolysphereTraits<ConcreteTraits>::HardInteraction
    ::getRangeRadius(const std::byte *data) const
{
    const auto &concreteTraits = static_cast<const ConcreteTraits &>(this->traits);

    const auto &sphereData = concreteTraits.getSphereData(data);
    auto comparator = [](const SphereData &sd1, const SphereData &sd2) {
        return sd1.radius < sd2.radius;
    };
    return 2 * std::max_element(sphereData.begin(), sphereData.end(), comparator)->radius;
}

template<typename ConcreteTraits>
bool PolysphereTraits<ConcreteTraits>::HardInteraction
    ::overlapWithWall(const Vector<3> &pos, [[maybe_unused]] const Matrix<3, 3> &orientation, const std::byte *data,
                      std::size_t idx, const Vector<3> &wallOrigin, const Vector<3> &wallVector) const
{
    const auto &concreteTraits = static_cast<const ConcreteTraits &>(this->traits);

    const auto &sphereData = concreteTraits.getSphereData(data)[idx];
    double dotProduct = wallVector * (pos - wallOrigin);
    return dotProduct < sphereData.radius;
}


// PolysphereTraits::WolframPrinter ####################################################################################

template<typename ConcreteTraits>
std::string PolysphereTraits<ConcreteTraits>::WolframPrinter
    ::print(const Shape &shape) const
{
    const auto &concreteTraits = static_cast<const ConcreteTraits &>(this->traits);
    const auto &sphereData = concreteTraits.getSphereData(shape.getData().raw());

    std::ostringstream out;
    out << std::fixed;
    out << "{";
    for (std::size_t i{}; i < sphereData.size() - 1; i++) {
        const auto &data = sphereData[i];
        data.toWolfram(out, shape);
        out << ",";
    }
    sphereData.back().toWolfram(out, shape);
    out << "}";
    return out.str();
}


// PolysphereTraits ####################################################################################################

template<typename ConcreteTraits>
PolysphereTraits<ConcreteTraits>::PolysphereTraits()
        : interaction{std::make_shared<HardInteraction>(*this)}, centralInteraction{nullptr},
          wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{ }

template<typename ConcreteTraits>
PolysphereTraits<ConcreteTraits>::PolysphereTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
        : interaction{centralInteraction}, centralInteraction{centralInteraction},
          wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{
    this->centralInteraction->installCentresProvider([this](const std::byte *data) {
        return this->getInteractionCentres(data);
    });
}

template<typename ConcreteTraits>
PolysphereTraits<ConcreteTraits>::~PolysphereTraits() {
    if (this->centralInteraction)
        this->centralInteraction->detach();
}

template<typename ConcreteTraits>
std::shared_ptr<const ShapePrinter>
PolysphereTraits<ConcreteTraits>::getPrinter(const std::string &format,
                                             const std::map<std::string, std::string> &params) const
{
    std::size_t meshSubdivisions = DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->wolframPrinter;
    else if (format == "obj")
        return this->createObjPrinter(meshSubdivisions);
    else
        throw NoSuchShapePrinterException("PolysphereTraits: unknown printer format: " + format);
}

template<typename ConcreteTraits>
std::shared_ptr<ShapePrinter> PolysphereTraits<ConcreteTraits>::createObjPrinter(std::size_t subdivisions) const {
    PolydisperseXCShapePrinter::GeometryComplexProvider provider = [this](const ShapeData &data) {
        const auto &thisConcreteTraits = static_cast<const ConcreteTraits &>(*this);
        const auto &sphereData = thisConcreteTraits.getSphereData(data.raw());

        PolydisperseXCShapePrinter::GeometryComplex xcSpheres;
        xcSpheres.reserve(sphereData.size());
        for (const auto &sphereDataEntry : sphereData) {
            double radius = sphereDataEntry.radius;
            auto polymorphicSphere = std::make_shared<PolymorphicXCAdapter<XCSphere>>(XCSphere(radius));
            xcSpheres.emplace_back(std::move(polymorphicSphere), sphereDataEntry.position);
        }

        return xcSpheres;
    };

    return std::make_shared<PolydisperseXCObjShapePrinter>(std::move(provider), subdivisions);
}

template<typename ConcreteTraits>
std::vector<Vector<3>> PolysphereTraits<ConcreteTraits>::getInteractionCentres(const std::byte *data) const {
    const auto &thisConcreteTraits = static_cast<const ConcreteTraits &>(*this);
    const auto &sphereData = thisConcreteTraits.getSphereData(data);

    std::vector<Vector<3>> interactionCenters;
    interactionCenters.reserve(sphereData.size());
    for (const auto &[pos, radius] : sphereData)
        interactionCenters.push_back(pos);

    return interactionCenters;
}