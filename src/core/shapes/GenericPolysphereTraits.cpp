//
// Created by Piotr Kubala on 22/07/2024.
//

#include "GenericPolysphereTraits.h"


// PolysphereShape #####################################################################################################

PolysphereShape::PolysphereShape(std::vector<SphereData> sphereData, OptionalAxis primaryAxis,
                                 OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin,
                                 std::optional<double> volume,
                                 const std::map<std::string, Vector<3>> &customNamedPoints)
        : sphereData{std::move(sphereData)}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}, namedPoints{customNamedPoints}
{
    Expects(!this->sphereData.empty());
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();

    if (volume.has_value())
        this->volume = *volume;
    else
        this->volume = this->calculateVolume();

    for (std::size_t i{}; i < this->sphereData.size(); i++)
        this->namedPoints["s" + std::to_string(i)] = this->sphereData[i].position;
}

Vector<3> PolysphereShape::getPrimaryAxis() const {
    if (!this->primaryAxis.has_value())
        throw std::runtime_error("PolysphereShape::getPrimaryAxis: primary axis not defined");
    return this->primaryAxis.value();
}

Vector<3> PolysphereShape::getSecondaryAxis() const {
    if (!this->secondaryAxis.has_value())
        throw std::runtime_error("PolysphereShape::getSecondaryAxis: secondary axis not defined");
    return this->secondaryAxis.value();
}

std::vector<Vector<3>> PolysphereShape::getInteractionCentres() const {
    std::vector<Vector<3>> centres;
    centres.reserve(this->sphereData.size());
    std::transform(this->sphereData.begin(), this->sphereData.end(), std::back_inserter(centres),
                   std::mem_fn(&SphereData::position));
    return centres;
}

double PolysphereShape::calculateVolume() const {
    ExpectsMsg(!this->spheresOverlap(),
               "PolysphereShape::calculateVolume: automatic volume not supported for overlapping spheres");

    auto volumeAccumulator = [](double volume_, const SphereData &data) {
        return volume_ + 4 * M_PI / 3 * data.radius * data.radius * data.radius;
    };
    return std::accumulate(this->sphereData.begin(), this->sphereData.end(), 0., volumeAccumulator);
}

void PolysphereShape::normalizeMassCentre() {
    Vector<3> massCentre = this->calculateMassCentre();

    auto massCentreShifter = [massCentre](const SphereData &data) {
        return SphereData(data.position - massCentre, data.radius);
    };

    std::vector<SphereData> newSphereData;
    newSphereData.reserve(this->sphereData.size());
    std::transform(this->sphereData.begin(), this->sphereData.end(), std::back_inserter(newSphereData),
                   massCentreShifter);

    this->sphereData = std::move(newSphereData);
    this->geometricOrigin -= massCentre;

    for (auto &[name, point] : this->namedPoints)
        point -= massCentre;
}

Vector<3> PolysphereShape::calculateMassCentre() const {
    auto massCentreAccumulator = [](const Vector<3> &sum, const SphereData &data) {
        double r = data.radius;
        return sum + (r*r*r) * data.position;
    };

    auto weightAccumulator = [](double sum, const SphereData &data) {
        double r = data.radius;
        return sum + r*r*r;
    };

    Vector<3> massCentre = std::accumulate(this->sphereData.begin(), this->sphereData.end(), Vector<3>{},
                                           massCentreAccumulator);
    double weightSum = std::accumulate(this->sphereData.begin(), this->sphereData.end(), 0., weightAccumulator);
    massCentre /= weightSum;
    return massCentre;
}

bool PolysphereShape::spheresOverlap() const {
    for (std::size_t i{}; i < this->sphereData.size(); i++) {
        for (std::size_t j = i + 1; j < this->sphereData.size(); j++) {
            const auto &data1 = this->sphereData[i];
            const auto &data2 = this->sphereData[j];

            constexpr double EPSILON = 1e-12;
            double distance2 = (data1.position - data2.position).norm2();
            double radii2 = std::pow(data1.radius + data2.radius, 2);
            if (distance2 * (EPSILON + 1) < radii2)
                return true;
        }
    }

    return false;
}

void PolysphereShape::addCustomNamedPoints(std::map<std::string, Vector<3>> customNamedPoints) {
    customNamedPoints.merge(std::move(this->namedPoints));
    this->namedPoints = std::move(customNamedPoints);
}

bool operator==(const PolysphereShape &lhs, const PolysphereShape &rhs) {
    return std::tie(
        lhs.sphereData, lhs.primaryAxis, lhs.secondaryAxis, lhs.geometricOrigin, lhs.volume, lhs.namedPoints
    ) == std::tie(
        rhs.sphereData, rhs.primaryAxis, rhs.secondaryAxis, rhs.geometricOrigin, rhs.volume, rhs.namedPoints
    );
}


// GenericPolysphereTraits #############################################################################################

GenericPolysphereTraits::GenericPolysphereTraits() : PolysphereTraits<GenericPolysphereTraits>() {

}

GenericPolysphereTraits::GenericPolysphereTraits(const PolysphereShape &defaultSpecies)
        : GenericPolysphereTraits()
{
    this->addSpecies("A", defaultSpecies);
    this->setDefaultSpecies("A");
}

GenericPolysphereTraits::GenericPolysphereTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
        : PolysphereTraits<GenericPolysphereTraits>(centralInteraction)
{
    this->getCentralInteraction().installCentresProvider([this](const std::byte *data) {
        return this->speciesFor(data).getInteractionCentres();
    });
}

GenericPolysphereTraits::GenericPolysphereTraits(const PolysphereShape &polysphereShape,
                                                 const std::shared_ptr<CentralInteraction> &centralInteraction)
        : GenericPolysphereTraits(centralInteraction)
{
    this->addSpecies("A", polysphereShape);
    this->setDefaultShapeData({{"species", "A"}});
}

