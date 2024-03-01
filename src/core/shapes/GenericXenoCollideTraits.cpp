//
// Created by Piotr Kubala on 28/02/2024.
//

#include <utility>

#include "GenericXenoCollideTraits.h"


GenericXenoCollideShape::GenericXenoCollideShape(std::shared_ptr<AbstractXCGeometry> geometry, double volume,
                                                 OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                 const Vector<3> &geometricOrigin,
                                                 const std::map<std::string, Vector<3>> &customNamedPoints)
        : geometries{std::move(geometry)}, interactionCentres{}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}, volume{volume}, customNamedPoints{customNamedPoints}, convex{true}
{
    Expects(this->geometries.front());
    Expects(volume > 0);
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();
}

GenericXenoCollideShape::GenericXenoCollideShape(const std::vector<GeometryData> &geometries, double volume,
                                                 OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                 const Vector<3> &geometricOrigin,
                                                 const std::map<std::string, Vector<3>> &customNamedPoints,
                                                 bool forceConvex)
        : primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis}, geometricOrigin{geometricOrigin},
          customNamedPoints{customNamedPoints}
{
    Expects(!geometries.empty());
    ExpectsMsg(geometries.size() != 1, "For a single interaction center, use the other constructor");

    this->geometries.reserve(geometries.size());
    this->interactionCentres.reserve(geometries.size());
    for (const auto &[geometry, center] : geometries) {
        Expects(geometry);
        this->geometries.push_back(geometry);
        this->interactionCentres.push_back(center);
    }

    Expects(volume > 0);
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();

    this->convex = forceConvex || (this->geometries.size() == 1);
}

Vector<3> GenericXenoCollideShape::getPrimaryAxis() const {
    if (!this->primaryAxis.has_value())
        throw std::runtime_error("GenericXenoCollideShape::getPrimaryAxis: primary axis not defined");
    return this->primaryAxis.value();
}

Vector<3> GenericXenoCollideShape::getSecondaryAxis() const {
    if (!this->secondaryAxis.has_value())
        throw std::runtime_error("GenericXenoCollideShape::getSecondaryAxis: secondary axis not defined");
    return this->secondaryAxis.value();
}

void GenericXenoCollideTraits::imbueFakeInteractionCenter(GenericXenoCollideShape &shape) {
    if (!shape.interactionCentres.empty())
        return;

    shape.interactionCentres.push_back({0, 0, 0});
}

GenericXenoCollideTraits::GenericXenoCollideTraits(const GenericXenoCollideShape &shape) {
    this->addSpecies("A", shape);
    this->setDefaultShapeData({{"species", "A"}});
}

std::vector<Vector<3>> GenericXenoCollideTraits::getInteractionCentres(const std::byte *data) const {
    return this->speciesFor(data).getInteractionCentres();
}

bool GenericXenoCollideTraits::isConvex() const {
    const auto &allSpecies = this->getAllSpecies();
    return std::all_of(allSpecies.begin(), allSpecies.end(), [](const GenericXenoCollideShape &shape) {
        return shape.isConvex();
    });
}

ShapeData GenericXenoCollideTraits::addSpecies(const std::string &speciesName, const GenericXenoCollideShape &species) {
    std::size_t numCentres = species.interactionCentres.size();
    if (numCentres == 0) {
        if (!this->isMulticentre_)
            return GenericShapeRegistry::addSpecies(speciesName, species);

        // Add {0, 0, 0} center is any other shape is multi-center
        auto speciesCopy = species;
        GenericXenoCollideTraits::imbueFakeInteractionCenter(speciesCopy);
        return GenericShapeRegistry::addSpecies(speciesName, speciesCopy);
    } else {
        // If it was single-center before, add the {0, 0, 0} center retrospectively to all species
        if (!this->isMulticentre_) {
            for (std::size_t speciesIdx{}; speciesIdx < this->getNumOfSpecies(); speciesIdx++) {
                auto &species_ = this->modifySpecies(speciesIdx);
                GenericXenoCollideTraits::imbueFakeInteractionCenter(species_);
            }
        }

        this->isMulticentre_ = true;
        return GenericShapeRegistry::addSpecies(speciesName, species);
    }
}