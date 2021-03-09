/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2008-2020, Alliance for Sustainable Energy, LLC, and other contributors. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
*  disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote products
*  derived from this software without specific prior written permission from the respective party.
*
*  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative works
*  may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without specific prior
*  written permission from Alliance for Sustainable Energy, LLC.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND ANY CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S), ANY CONTRIBUTORS, THE UNITED STATES GOVERNMENT, OR THE UNITED
*  STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
*  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************************************************/

#include "Polygon.hpp"
#include "Vector3d.hpp"
#include "Geometry.hpp"

namespace openstudio {

Polygon3d::Polygon3d() {}

Polygon3d::Polygon3d(const Point3dVector& outerPath) {
  for (const auto& p : outerPath) {
    m_outerPath.emplace_back(p);
  }
}

Polygon3d::Polygon3d(const Point3dVector& outerPath, const Point3dVectorVector& innerPaths) {
  for (const auto& p : outerPath) {
    m_outerPath.emplace_back(p);
  }

  for (const auto& innerPath : innerPaths) {
    addHole(innerPath);
  }
}

/// Adds a point to the polygon perimeter
void Polygon3d::addPoint(const Point3d& point) {
  m_outerPath.emplace_back(point);
}

/// Sets the perimeter of the polygon
void Polygon3d::setOuterPath(const Point3dVector& outerPath) {
  m_outerPath = outerPath;
}

Point3dVector Polygon3d::getOuterPath() const {
  return m_outerPath;
}

Point3dVectorVector Polygon3d::getInnerPaths() const {
  return m_innerPaths;
}

void Polygon3d::addHole(const Point3dVector& hole) {
  m_innerPaths.emplace_back(hole);
}

Vector3d Polygon3d::newallVector() {
  if (OptionalVector3d v = openstudio::getNewallVector(m_outerPath)) {
    return v.get();
  }

  return Vector3d();
}

Vector3d Polygon3d::outwardNormal() {
  return openstudio::getOutwardNormal(m_outerPath).get();
}

double Polygon3d::grossArea() {

  if (boost::optional<double> area = openstudio::getArea(m_outerPath)) {
    return area.get();
  }

  return 0;
}

double Polygon3d::netArea() {

  double netArea = grossArea();
  for (const auto& hole : m_innerPaths) {
    if (boost::optional<double> area = openstudio::getArea(hole)) {
      netArea -= area.get();
    }
  }

  return netArea;
}

double Polygon3d::perimeter() {

  double perimeter = 0;
  for (size_t i = 0; i < m_outerPath.size(); i++) {
    Point3d p1 = m_outerPath[i];
    Point3d p2 = m_outerPath[(i + 1) % m_outerPath.size()];
    perimeter += openstudio::getDistance(p1, p2);
  }

  return perimeter;
}

bool Polygon3d::isClockwise() {
  if (OptionalVector3d normal = getOutwardNormal(m_outerPath)) {
    return normal.get().z() > 0;
  }

  return true;
}

Point3d Polygon3d::centroid() {
  if (auto p = openstudio::getCentroid(m_outerPath)) {
    return p.get();
  }

  return Point3d();
}

//bool Polygon3d::PointInPolygon(Point3d testPoint) {
//  return true;
//}

//typedef boost::geometry::model::polygon<BoostPoint> BoostPolygon;
//typedef boost::geometry::model::d2::point_xy<double> BoostPoint;

// TODO: I want to inlude all the boolean joining etc code here

}  // namespace openstudio
