//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_SMOOTHWEDGETRAITS_H
#define RAMPACK_SMOOTHWEDGETRAITS_H

#include "XenoCollideTraits.h"


class SmoothWedgeTraits : public XenoCollideTraits<SmoothWedgeTraits>, public ShapePrinter {
public:
    class CollideGeometry {
    private:
        double R{};
        double r{};
        double l{};
        double Rminusr{};
        double Rpos{};
        double rpos{};
        double circumsphereRadius{};

        friend SmoothWedgeTraits;

    public:
        CollideGeometry(double R, double r, double l);

        [[nodiscard]] Vector<3> getCenter() const { return {}; }

        [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const {
            Vector<3> nNorm = n.normalized();
            if (this->Rminusr > nNorm[2]*this->l)
                return this->R * nNorm + Vector<3>{0, 0, this->Rpos};
            else
                return this->r * nNorm + Vector<3>{0, 0, this->rpos};
        }

        [[nodiscard]] double getCircumsphereRadius() const { return this->circumsphereRadius; }
    };

private:
    double R{};
    double r{};
    double l{};
    std::vector<CollideGeometry> shapeModel;
    std::vector<Vector<3>> interactionCentres;

    static double getVolume(double R, double r, double l);

public:
    SmoothWedgeTraits(double R, double r, double l, std::size_t subdivisions = 0);

    [[nodiscard]] const CollideGeometry &getCollideGeometry([[maybe_unused]] std::size_t idx = 0) const {
        return this->shapeModel[idx];
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return this->interactionCentres; }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }
    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;
};


#endif //RAMPACK_SMOOTHWEDGETRAITS_H
