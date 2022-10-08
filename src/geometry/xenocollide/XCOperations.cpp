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

#include "XCOperations.h"


CollideSum::CollideSum(std::shared_ptr<AbstractXCGeometry> geom1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
                       std::shared_ptr<AbstractXCGeometry> geom2, const Matrix<3, 3> &rot2, const Vector<3> &pos2)
        : rot1{rot1}, rot2{rot2}, pos1{pos1}, pos2{pos2}, geom1{std::move(geom1)}, geom2{std::move(geom2)}
{
    double circumsphere1 = this->geom1->getCircumsphereRadius();
    double circumsphere2 = this->geom2->getCircumsphereRadius();
    this->circumsphereRadius = (this->pos1 + this->pos2).norm() + circumsphere1 + circumsphere2;
}

Vector<3> CollideSum::getSupportPoint(const Vector<3>& n) const {
    Vector<3> p1 = this->rot1 * (this->geom1->getSupportPoint(this->rot1.transpose() * n)) + this->pos1;
    Vector<3> p2 = this->rot2 * (this->geom2->getSupportPoint(this->rot2.transpose() * n)) + this->pos2;
    return p1 + p2;
}

Vector<3> CollideSum::getCenter() const {
    Vector<3> p1 = this->rot1 * (geom1->getCenter()) + pos1;
    Vector<3> p2 = this->rot2 * (geom2->getCenter()) + pos2;
    return p1 + p2;
}

CollideDiff::CollideDiff(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
                         std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3, 3> &rot2, const Vector<3> &pos2)
        : rot1{rot1}, rot2{rot2}, pos1{pos1}, pos2{pos2}, geom1{std::move(g1)}, geom2{std::move(g2)}
{
    double circumsphere1 = this->geom1->getCircumsphereRadius();
    double circumsphere2 = this->geom2->getCircumsphereRadius();
    this->circumsphereRadius = (this->pos1 - this->pos2).norm() + circumsphere1 + circumsphere2;
}

Vector<3> CollideDiff::getSupportPoint(const Vector<3> &n) const{
    Vector<3> p1 = this->rot1 * (this->geom1->getSupportPoint(this->rot1.transpose() * n)) + this->pos1;
    Vector<3> p2 = this->rot2 * (this->geom2->getSupportPoint(this->rot2.transpose() * -n)) + this->pos2;
    return p1 - p2;
}

Vector<3> CollideDiff::getCenter() const {
    Vector<3> p1 = this->rot1 * (geom1->getCenter()) + pos1;
    Vector<3> p2 = this->rot2 * (geom2->getCenter()) + pos2;
    return p1 - p2;
}

CollideMax::CollideMax(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
                       std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3, 3> &rot2, const Vector<3> &pos2)
        : rot1{rot1}, rot2{rot2}, pos1{pos1}, pos2{pos2}, geom1{std::move(g1)}, geom2{std::move(g2)}
{
    double circumsphere1 = this->pos1.norm() + this->geom1->getCircumsphereRadius();
    double circumsphere2 = this->pos2.norm() + this->geom2->getCircumsphereRadius();
    this->circumsphereRadius = std::max(circumsphere1, circumsphere2);
}

Vector<3> CollideMax::getSupportPoint(const Vector<3> &n) const {
    Vector<3> v1 = this->rot1 * this->geom1->getSupportPoint(this->rot1.transpose() * n) + this->pos1;
    Vector<3> v2 = this->rot2 * this->geom2->getSupportPoint(this->rot2.transpose() * n) + this->pos2;
    if ((v2 - v1) * n > 0 )
        return v2;
    else
        return v1;
}

Vector<3> CollideMax::getCenter() const{
    Vector<3> p1 = this->rot1 * (geom1->getCenter()) + pos1;
    Vector<3> p2 = this->rot2 * (geom2->getCenter()) + pos2;
    return (p1 + p2) / 2.;
}