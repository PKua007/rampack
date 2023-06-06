/*
XenoCollide Collision Detection and Physics Library
Copyright (c) 2007-2014 Gary Snethen http://xenocollide.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/*
 * Heavily adapted by Michal Ciesla and Piotr Kubala
 */

#include <utility>

#include "XCOperations.h"
#include "utils/Exceptions.h"


XCSum::XCSum(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1, const Matrix<3, 3> &rot1,
             std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2, const Matrix<3, 3> &rot2)
{
    this->add(std::move(geom1), pos1, rot1);
    this->add(std::move(geom2), pos2, rot2);
}

Vector<3> XCSum::getSupportPoint(const Vector<3>& n) const {
    Vector<3> p;
    for (const auto &entry : this->entries)
        p += entry.rot * (entry.geometry->getSupportPoint(entry.rot.transpose() * n)) + entry.pos;
    return p;
}

Vector<3> XCSum::getCenter() const {
    Vector<3> p;
    for (const auto &entry : this->entries)
        p += entry.rot * entry.geometry->getCenter() + entry.pos;
    return p;
}

void XCSum::add(std::shared_ptr<AbstractXCGeometry> geom, const Vector<3> &pos, const Matrix<3, 3> &rot) {
    this->entries.emplace_back(std::move(geom), pos, rot);
    this->recalculateGeometry();
}

void XCSum::recalculateGeometry() {
    Vector<3> centerSum;
    double circumsphereRadiusSum{};
    double insphereRadiusSum{};
    for (const auto &entry : this->entries) {
        centerSum += entry.pos;
        circumsphereRadiusSum += entry.geometry->getCircumsphereRadius();
        insphereRadiusSum += entry.geometry->getInsphereRadius();
    }

    double centerDisplacement = centerSum.norm();
    this->circumsphereRadius = centerDisplacement + circumsphereRadiusSum;
    this->insphereRadius = std::max(0., insphereRadiusSum - centerDisplacement);
}

XCDiff::XCDiff(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
               std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3, 3> &rot2, const Vector<3> &pos2)
        : rot1{rot1}, rot2{rot2}, pos1{pos1}, pos2{pos2}, geom1{std::move(g1)}, geom2{std::move(g2)}
{
    double circumsphere1 = this->geom1->getCircumsphereRadius();
    double circumsphere2 = this->geom2->getCircumsphereRadius();
    double insphere1 = this->geom1->getInsphereRadius();
    double insphere2 = this->geom2->getInsphereRadius();
    double centerDisplacement = (this->pos1 - this->pos2).norm();
    this->circumsphereRadius = centerDisplacement + circumsphere1 + circumsphere2;
    this->insphereRadius = std::max(0., insphere1 + insphere2 - centerDisplacement);
}

Vector<3> XCDiff::getSupportPoint(const Vector<3> &n) const{
    Vector<3> p1 = this->rot1 * (this->geom1->getSupportPoint(this->rot1.transpose() * n)) + this->pos1;
    Vector<3> p2 = this->rot2 * (this->geom2->getSupportPoint(this->rot2.transpose() * -n)) + this->pos2;
    return p1 - p2;
}

Vector<3> XCDiff::getCenter() const {
    Vector<3> p1 = this->rot1 * (geom1->getCenter()) + pos1;
    Vector<3> p2 = this->rot2 * (geom2->getCenter()) + pos2;
    return p1 - p2;
}

XCMax::XCMax(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1, const Matrix<3, 3> &rot1,
             std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2, const Matrix<3, 3> &rot2)
{
    this->add(std::move(geom1), pos1, rot1);
    this->add(std::move(geom2), pos2, rot2);
}

Vector<3> XCMax::getSupportPoint(const Vector<3> &n) const {
    const auto &firstEntry = this->entries.front();
    Vector<3> v = firstEntry.rot * firstEntry.geometry->getSupportPoint(firstEntry.rot.transpose()*n) + firstEntry.pos;
    double maxProjection = v * n;

    for (std::size_t i = 1; i < this->entries.size(); i++) {
        const auto &entry = this->entries[i];
        Vector<3> v1 = entry.rot * entry.geometry->getSupportPoint(entry.rot.transpose() * n) + entry.pos;
        double projection = v1 * n;
        if (projection > maxProjection) {
            v = v1;
            maxProjection = projection;
        }
    }

    return v;
}

Vector<3> XCMax::getCenter() const{
    Vector<3> p;
    for (const auto &entry : this->entries)
        p += entry.rot * entry.geometry->getCenter() + entry.pos;
    return p / static_cast<double>(this->entries.size());
}

double XCMax::calculateInsphereRadius() const {
    if (this->entries.empty()) {
        return 0;
    } else if (this->entries.size() == 1) {
        const auto &entry = this->entries.front();
        return std::max(0.0, entry.geometry->getInsphereRadius() - entry.pos.norm());
    } else if (this->entries.size() == 2) {
        return this->calculateInsphereRadius2();
    } else {
        // Currently, we do not support calculating optimal insphere radius for >= 2 wrapped geometries
        return 0;
    }
}

void XCMax::recalculateGeometry() {
    const auto &firstEntry = this->entries.front();
    this->circumsphereRadius = firstEntry.pos.norm() + firstEntry.geometry->getCircumsphereRadius();;
    for (const auto &entry: this->entries) {
        double newRadius = entry.pos.norm() + entry.geometry->getCircumsphereRadius();
        this->circumsphereRadius = std::max(this->circumsphereRadius, newRadius);
    }

    this->insphereRadius = this->calculateInsphereRadius();
}

void XCMax::add(std::shared_ptr<AbstractXCGeometry> geom, const Vector<3> &pos, const Matrix<3, 3> &rot) {
    this->entries.emplace_back(std::move(geom), pos, rot);
    this->recalculateGeometry();
}

double XCMax::calculateInsphereRadius2() const {
    Assert(this->entries.size() == 2);

    const auto &entry1 = this->entries[0];
    const auto &entry2 = this->entries[1];

    double insphere1 = entry1.geometry->getInsphereRadius();
    double insphere2 = entry2.geometry->getInsphereRadius();

    Vector<3> diff = (entry2.pos - entry1.pos);
    double diffNorm = diff.norm();
    Vector<3> normalDiff = diff/diffNorm;
    if (diffNorm / std::max(insphere1, insphere2) < 1e-14)
        return std::min(insphere1, insphere2);

    double originT = -(entry1.pos*normalDiff)/diffNorm;
    if (originT < 0)
        return 0;
    if (originT > 1)
        return 0;

    // Offset of the origin from the line joining shapes decreases insphere radius
    double originOffset = std::sqrt(std::max(0.0, entry1.pos.norm2() - std::pow(entry1.pos * normalDiff, 2)));
    // Sphere radius is interpolated linearly between the shapes
    double interpolatedR = (1 - originT)*insphere1 + originT*insphere2;
    return std::max(0.0, interpolatedR - originOffset);
}
