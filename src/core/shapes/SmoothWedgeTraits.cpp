//
// Created by ciesla on 8/9/22.
//

#include "SmoothWedgeTraits.h"
#include "utils/Exceptions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"


SmoothWedgeShape::CollideGeometry::CollideGeometry(double R, double r, double l)
        : R{R}, r{r}, l{l}, Rminusr{R - r}, Rpos{(-l + R - r)/2}, rpos{(l + R - r)/2},
          circumsphereRadius{(l + R + r)/2}, insphereRadius{0.5*(R + r + Rminusr*Rminusr/l)}
{
    Expects(R > 0);
    Expects(r > 0);
    Expects(l > 0);
}

double SmoothWedgeShape::computeVolume(double R, double r, double l) {
    double r2 = r*r;
    double r3 = r2*r;
    double R2 = R*R;
    double R3 = R2*R;
    double Rr = R*r;
    double epsilon{};
    // Prevent nan if R == r and l == 0
    if (R == r)
        epsilon = 0;
    else
        epsilon = (R - r)/l;
    double epsilon2 = epsilon*epsilon;

    return M_PI*l/3*(R2 + Rr + r2)*(1 + epsilon2) + 2*M_PI/3*(R3 + r3);
}

std::vector<double> SmoothWedgeShape::calculateRelativeSpherePositions() const {
    double A{};
    // Prevent nan if R == r and l == 0
    if (this->bottomR == this->topR)
        A = 1;
    else
        A = (this->l - this->topR + this->bottomR) / (this->l + this->topR - this->bottomR);

    std::vector<double> alphas;
    alphas.reserve(this->subdivisions + 1);
    alphas.push_back(0);
    for (std::size_t i{}; i < this->subdivisions; i++)
        alphas.push_back(A*alphas.back() + 1);
    for (auto &alpha : alphas)
        alpha /= alphas.back();
    return alphas;
}

SmoothWedgeShape::SmoothWedgeShape(double bottomR, double topR, double l, std::size_t subdivisions)
        : bottomR{bottomR}, topR{topR}, l{l}, volume{SmoothWedgeShape::computeVolume(bottomR, topR, l)},
          begNamedPoint{0, 0, (-l + bottomR - topR)/2}, endNamedPoint{0, 0, (l + bottomR - topR)/2}
{
    Expects(bottomR > 0);
    Expects(topR > 0);
    Expects(l >= std::abs(bottomR - topR));

    this->subdivisions = subdivisions == 0 ? 1 : subdivisions;
    if (this->subdivisions == 1) {
        this->interactionCentres = {};
        this->shapeParts.emplace_back(bottomR, topR, l);
        return;
    }

    std::vector<double> alphas = this->calculateRelativeSpherePositions();

    double begZ = (-this->l + this->bottomR - this->topR) / 2;
    this->shapeParts.reserve(subdivisions);
    this->interactionCentres.reserve(subdivisions);
    for (std::size_t i{}; i < subdivisions; i++) {
        double alpha0 = alphas[i];
        double alpha1 = alphas[i + 1];

        double r0 = alpha0*this->topR + (1 - alpha0) * this->bottomR;
        double r1 = alpha1*this->topR + (1 - alpha1) * this->bottomR;
        double l01 = (alpha1 - alpha0)*this->l;

        this->shapeParts.emplace_back(r0, r1, l01);

        double centreZ = begZ + this->l*(alpha0 + alpha1)/2 - (r0 - r1)/2;
        this->interactionCentres.push_back({0, 0, centreZ});
    }
}

bool SmoothWedgeShape::equal(double bottomR_, double topR_, double l_, std::size_t subdivisions_) const {
    return std::tie(this->bottomR, this->topR, this->l, this->subdivisions)
           == std::tie(bottomR_, topR_, l_, subdivisions_);
}

std::shared_ptr<const ShapePrinter>
SmoothWedgeTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
    // We override the function from XenoCollideTraits not to redundantly print subdivisions

    std::size_t meshSubdivisions = XenoCollideTraits::DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->createPrinter<PolydisperseXCWolframShapePrinter>(meshSubdivisions);
    else if (format == "obj")
        return this->createPrinter<PolydisperseXCObjShapePrinter>(meshSubdivisions);
    else
        throw NoSuchShapePrinterException("XenoCollideTraits: unknown printer format: " + format);
}

const SmoothWedgeShape &SmoothWedgeTraits::speciesFor(const ShapeData &data) const {
    std::size_t speciesIdx = data.as<Data>().speciesIdx;
    Expects(speciesIdx < this->species.size());
    return this->species[speciesIdx];
}

void SmoothWedgeTraits::validateShapeData(const ShapeData &data) const {
    const auto &wedgeData = data.as<Data>();
    ShapeDataValidateMsg(wedgeData.speciesIdx < this->species.size(), "Shape index out of range");
}

TextualShapeData SmoothWedgeTraits::serialize(const ShapeData &data) const {
    const auto &shape = speciesFor(data);
    ShapeDataSerializer serializer;
    serializer["bottom_r"] = shape.getBottomR();
    serializer["top_r"] = shape.getTopR();
    serializer["l"] = shape.getL();
    serializer["subdivisions"] = shape.getSubdivisions();
    return serializer.toTextualShapeData();
}

ShapeData SmoothWedgeTraits::deserialize(const TextualShapeData &data) const {
    ShapeDataDeserializer deserializer(data);
    auto bottomR = deserializer.as<double>("bottom_r");
    auto topR = deserializer.as<double>("top_r");
    auto l = deserializer.as<double>("l");
    auto subdivisions = deserializer.as<std::size_t>("subdivisions");
    deserializer.throwIfNotAccessed();

    ShapeDataValidateMsg(bottomR > 0, "Bottom sphere radius must be > 0");
    ShapeDataValidateMsg(topR > 0, "Top sphere radius must be > 0");
    ShapeDataValidateMsg(l > std::abs(topR - bottomR),
                         "Length must be > than the difference between top and bottom sphere radii");
    if (subdivisions == 0)
        subdivisions = 1;
    return this->shapeDataFor(bottomR, topR, l, subdivisions);
}

SmoothWedgeTraits::SmoothWedgeTraits(std::optional<double> defaultBottomR, std::optional<double> defaultTopR,
                                     std::optional<double> defaultL, std::size_t defaultSubdivisions)
         : defaultBottomR{defaultBottomR}, defaultTopR{defaultTopR}, defaultL{defaultL},
           defaultSubdivisions{defaultSubdivisions}
{
    if (this->defaultBottomR)
        Expects(*this->defaultBottomR > 0);
    if (this->defaultTopR)
        Expects(*this->defaultTopR >= 0);
    if (this->defaultL)
        Expects(*this->defaultL > 0);
    if (this->defaultTopR && this->defaultBottomR && this->defaultL)
        Expects(*this->defaultL >= std::abs(*this->defaultTopR - *this->defaultBottomR));

    ShapeDataSerializer serializer;
    if (this->defaultBottomR)
        serializer["bottom_r"] = *this->defaultBottomR;
    if (this->defaultTopR)
        serializer["top_r"] = *this->defaultTopR;
    if (this->defaultL)
        serializer["l"] = *this->defaultL;
    serializer["subdivisions"] = this->defaultSubdivisions;
    this->setDefaultShapeData(serializer.toTextualShapeData());

    this->registerDynamicNamedPoint("beg", [this](const ShapeData &data) {
        return this->speciesFor(data).getBegNamedPoint();
    });
    this->registerDynamicNamedPoint("end", [this](const ShapeData &data) {
        return this->speciesFor(data).getEndNamedPoint();
    });
}

ShapeData SmoothWedgeTraits::shapeDataFor(double bottomR, double topR, double l, std::size_t subdivisions) const {
    if (subdivisions == 0)
        subdivisions = 1;

    for (std::size_t i{}; i < this->species.size(); i++) {
        const auto &shape = this->species[i];
        if (shape.equal(bottomR, topR, l, subdivisions))
            return ShapeData(Data{i});
    }

    this->species.emplace_back(bottomR, topR, l, subdivisions);
    return ShapeData(Data{this->species.size() - 1});
}
