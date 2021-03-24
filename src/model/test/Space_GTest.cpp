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

#include <gtest/gtest.h>

#include "ModelFixture.hpp"

#include "../Model_Impl.hpp"
#include "../Building.hpp"
#include "../Building_Impl.hpp"
#include "../ThermalZone.hpp"
#include "../ThermalZone_Impl.hpp"
#include "../SpaceType.hpp"
#include "../SpaceType_Impl.hpp"
#include "../Space.hpp"
#include "../Space_Impl.hpp"
#include "../BuildingStory.hpp"
#include "../ScheduleCompact.hpp"
#include "../Surface.hpp"
#include "../Surface_Impl.hpp"
#include "../SubSurface.hpp"
#include "../SubSurface_Impl.hpp"
#include "../ElectricEquipment.hpp"
#include "../ElectricEquipmentDefinition.hpp"
#include "../GasEquipment.hpp"
#include "../GasEquipmentDefinition.hpp"
#include "../Lights.hpp"
#include "../Lights_Impl.hpp"
#include "../LightsDefinition.hpp"
#include "../People.hpp"
#include "../PeopleDefinition.hpp"
#include "../Schedule.hpp"
#include "../LifeCycleCost.hpp"
#include "../LifeCycleCost_Impl.hpp"
#include "../SpaceInfiltrationDesignFlowRate.hpp"
#include "../AirLoopHVACSupplyPlenum.hpp"
#include "../AirLoopHVACReturnPlenum.hpp"

#include "../../utilities/core/UUID.hpp"

#include "../../utilities/geometry/Point3d.hpp"
#include "../../utilities/geometry/Vector3d.hpp"
#include "../../utilities/geometry/Transformation.hpp"
#include "../../utilities/geometry/Geometry.hpp"
#include "../../utilities/geometry/BoundingBox.hpp"
#include "../../utilities/idf/WorkspaceObjectWatcher.hpp"
#include "../../utilities/core/Compare.hpp"
#include "../../osversion/VersionTranslator.hpp"
#include "../../utilities/geometry/Intersection.hpp"

#include <iostream>

using namespace openstudio;
using namespace openstudio::model;

TEST_F(ModelFixture, Space) {
  Model model;

  Space space1(model);
  Space space2(model);

  ASSERT_TRUE(space1.name());
  ASSERT_TRUE(space2.name());
  EXPECT_NE("", space1.name().get());
  EXPECT_NE("", space2.name().get());
  EXPECT_NE(space1.name().get(), space2.name().get());
}

TEST_F(ModelFixture, Space_Clone) {
  Model model;
  Space space(model);
  ASSERT_TRUE(space.name());
  EXPECT_EQ("Space 1", space.name().get());

  Point3dVector points;
  points.push_back(Point3d(0, 4, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(4, 0, 0));
  points.push_back(Point3d(4, 4, 0));

  Surface surface(points, model);
  surface.setParent(space);

  points.clear();
  points.push_back(Point3d(1, 3, 0));
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(3, 1, 0));
  points.push_back(Point3d(3, 3, 0));

  SubSurface subSurface(points, model);
  subSurface.setParent(surface);

  EXPECT_EQ(static_cast<unsigned>(1), model.getModelObjects<Space>().size());
  EXPECT_EQ(static_cast<unsigned>(1), model.getModelObjects<Surface>().size());
  EXPECT_EQ(static_cast<unsigned>(1), model.getModelObjects<SubSurface>().size());

  EXPECT_FALSE(space.parent());
  ASSERT_TRUE(surface.parent());
  EXPECT_EQ(space.handle(), surface.parent()->handle());
  ASSERT_TRUE(subSurface.parent());
  EXPECT_EQ(surface.handle(), subSurface.parent()->handle());

  ModelObject clone = space.clone(model);

  ASSERT_TRUE(clone.optionalCast<Space>());

  Space spaceClone = clone.cast<Space>();
  ASSERT_TRUE(spaceClone.name());
  EXPECT_EQ("Space 2", spaceClone.name().get());

  EXPECT_EQ(static_cast<unsigned>(2), model.getModelObjects<Space>().size());
  EXPECT_EQ(static_cast<unsigned>(2), model.getModelObjects<Surface>().size());
  EXPECT_EQ(static_cast<unsigned>(2), model.getModelObjects<SubSurface>().size());

  ASSERT_EQ(static_cast<unsigned>(1), spaceClone.surfaces().size());
  EXPECT_NE(surface.handle(), spaceClone.surfaces()[0].handle());
  ASSERT_EQ(static_cast<unsigned>(1), spaceClone.surfaces()[0].subSurfaces().size());
  EXPECT_NE(subSurface.handle(), spaceClone.surfaces()[0].subSurfaces()[0].handle());
}

TEST_F(ModelFixture, Space_FloorArea) {
  Model model;
  Space space(model);

  EXPECT_EQ(0, space.floorArea());

  Point3dVector points;

  // walls
  points.clear();
  points.push_back(Point3d(0, 0, 1));
  points.push_back(Point3d(0, 1, 1));
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(0, 0, 0));
  Surface westWall(points, model);
  westWall.setParent(space);
  EXPECT_EQ("Wall", westWall.surfaceType());

  EXPECT_EQ(0, space.floorArea());

  points.clear();
  points.push_back(Point3d(0, 1, 1));
  points.push_back(Point3d(1, 1, 1));
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(0, 1, 0));
  Surface northWall(points, model);
  northWall.setParent(space);
  EXPECT_EQ("Wall", northWall.surfaceType());

  EXPECT_EQ(0, space.floorArea());

  points.clear();
  points.push_back(Point3d(1, 1, 1));
  points.push_back(Point3d(1, 0, 1));
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(1, 1, 0));
  Surface eastWall(points, model);
  eastWall.setParent(space);
  EXPECT_EQ("Wall", eastWall.surfaceType());

  EXPECT_EQ(0, space.floorArea());

  points.clear();
  points.push_back(Point3d(1, 0, 1));
  points.push_back(Point3d(0, 0, 1));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(1, 0, 0));
  Surface southWall(points, model);
  southWall.setParent(space);
  EXPECT_EQ("Wall", southWall.surfaceType());

  EXPECT_EQ(0, space.floorArea());

  // roof
  points.clear();
  points.push_back(Point3d(1, 1, 1));
  points.push_back(Point3d(0, 1, 1));
  points.push_back(Point3d(0, 0, 1));
  points.push_back(Point3d(1, 0, 1));
  Surface roof(points, model);
  roof.setParent(space);
  EXPECT_EQ("RoofCeiling", roof.surfaceType());

  EXPECT_EQ(0, space.floorArea());

  // floor
  points.clear();
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(0, 0, 0));
  Surface floor(points, model);
  floor.setParent(space);
  EXPECT_EQ("Floor", floor.surfaceType());

  EXPECT_NEAR(1, space.floorArea(), 0.0001);

  space.setPartofTotalFloorArea(false);

  EXPECT_NEAR(1, space.floorArea(), 0.0001);

  space.setPartofTotalFloorArea(true);

  EXPECT_NEAR(1, space.floorArea(), 0.0001);

  floor.setSurfaceType("Wall");

  EXPECT_EQ(0, space.floorArea());

  floor.setSurfaceType("Floor");
  westWall.setSurfaceType("Floor");
  northWall.setSurfaceType("Floor");
  eastWall.setSurfaceType("Floor");
  southWall.setSurfaceType("Floor");
  roof.setSurfaceType("Floor");

  EXPECT_NEAR(6, space.floorArea(), 0.0001);

  points.clear();
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(0, 0, 0));
  SubSurface floorWindow(points, model);
  floorWindow.setSurface(floor);
  EXPECT_EQ("Skylight", floorWindow.subSurfaceType());

  EXPECT_NEAR(6, space.floorArea(), 0.0001);
}

TEST_F(ModelFixture, Space_ThermalZone) {
  Model model;
  Space space(model);
  ThermalZone thermalZone1(model);
  ThermalZone thermalZone2(model);

  EXPECT_FALSE(space.thermalZone());
  EXPECT_TRUE(space.setThermalZone(thermalZone1));
  ASSERT_TRUE(space.thermalZone());
  EXPECT_EQ(thermalZone1.handle(), space.thermalZone()->handle());
  EXPECT_TRUE(space.setThermalZone(thermalZone2));
  ASSERT_TRUE(space.thermalZone());
  EXPECT_EQ(thermalZone2.handle(), space.thermalZone()->handle());

  EXPECT_EQ(space.multiplier(), thermalZone2.multiplier());
  EXPECT_FALSE(thermalZone2.setMultiplier(-1));
  EXPECT_TRUE(thermalZone2.setMultiplier(3));
  EXPECT_EQ(3, thermalZone2.multiplier());
  EXPECT_EQ(space.multiplier(), thermalZone2.multiplier());
}

TEST_F(ModelFixture, Space_Lights) {
  Model model;
  Space space(model);

  EXPECT_EQ(0, space.lightingPower());
  EXPECT_EQ(0, space.lightingPowerPerFloorArea());

  // floor
  Point3dVector points;
  points.push_back(Point3d(0, 10, 0));
  points.push_back(Point3d(10, 10, 0));
  points.push_back(Point3d(10, 0, 0));
  points.push_back(Point3d(0, 0, 0));
  Surface floor(points, model);
  floor.setParent(space);
  EXPECT_EQ("Floor", floor.surfaceType());
  EXPECT_NEAR(100, space.floorArea(), 0.0001);

  EXPECT_EQ(0, space.lightingPower());
  EXPECT_EQ(0, space.lightingPowerPerFloorArea());

  // add lights
  LightsDefinition definition(model);
  Lights light(definition);
  EXPECT_TRUE(light.setSpace(space));

  EXPECT_EQ(0, space.lightingPower());
  EXPECT_EQ(0, space.lightingPowerPerFloorArea());

  EXPECT_TRUE(definition.setLightingLevel(100));
  EXPECT_EQ(100, space.lightingPower());
  EXPECT_EQ(1, space.lightingPowerPerFloorArea());

  EXPECT_TRUE(definition.setWattsperSpaceFloorArea(1));
  EXPECT_EQ(100, space.lightingPower());
  EXPECT_EQ(1, space.lightingPowerPerFloorArea());

  EXPECT_TRUE(definition.setWattsperPerson(100));
  EXPECT_EQ(0, space.lightingPower());
  EXPECT_EQ(0, space.lightingPowerPerFloorArea());

  PeopleDefinition peopleDefinition(model);
  People people(peopleDefinition);
  EXPECT_TRUE(people.setSpace(space));
  EXPECT_TRUE(peopleDefinition.setNumberofPeople(1));
  EXPECT_EQ(100, space.lightingPower());
  EXPECT_EQ(1, space.lightingPowerPerFloorArea());

  EXPECT_TRUE(definition.setLightingLevel(100));
  EXPECT_EQ(100, space.lightingPower());
  EXPECT_EQ(1, space.lightingPowerPerFloorArea());

  EXPECT_FALSE(light.setMultiplier(-1));
  EXPECT_TRUE(light.isMultiplierDefaulted());

  EXPECT_TRUE(light.setMultiplier(0));
  EXPECT_FALSE(light.isMultiplierDefaulted());
  EXPECT_EQ(0, space.lightingPower());
  EXPECT_EQ(0, space.lightingPowerPerFloorArea());

  EXPECT_TRUE(light.setMultiplier(1));
  EXPECT_FALSE(light.isMultiplierDefaulted());
  EXPECT_EQ(100, space.lightingPower());
  EXPECT_EQ(1, space.lightingPowerPerFloorArea());

  EXPECT_TRUE(light.setMultiplier(2));
  EXPECT_FALSE(light.isMultiplierDefaulted());
  EXPECT_EQ(200, space.lightingPower());
  EXPECT_EQ(2, space.lightingPowerPerFloorArea());

  definition.remove();
  EXPECT_EQ(0, space.lightingPower());
  EXPECT_EQ(0, space.lightingPowerPerFloorArea());
}

TEST_F(ModelFixture, Space_Transformation) {
  Model model;
  Space space(model);
  Point3d origin(0, 0, 0);
  Point3d x1(1, 0, 0);

  Transformation transformation = space.transformation();

  Point3d test = transformation * origin;
  EXPECT_DOUBLE_EQ(0, test.x());
  EXPECT_DOUBLE_EQ(0, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());

  test = transformation * x1;
  EXPECT_DOUBLE_EQ(1, test.x());
  EXPECT_DOUBLE_EQ(0, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());

  EXPECT_TRUE(space.setTransformation(Transformation::translation(Vector3d(1, 0, 0))));
  EXPECT_DOUBLE_EQ(1, space.xOrigin());
  EXPECT_DOUBLE_EQ(0, space.yOrigin());
  EXPECT_DOUBLE_EQ(0, space.zOrigin());
  EXPECT_DOUBLE_EQ(0, space.directionofRelativeNorth());

  transformation = space.transformation();

  test = transformation * origin;
  EXPECT_DOUBLE_EQ(1, test.x());
  EXPECT_DOUBLE_EQ(0, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());

  test = transformation * x1;
  EXPECT_DOUBLE_EQ(2, test.x());
  EXPECT_DOUBLE_EQ(0, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());

  EXPECT_TRUE(
    space.setTransformation(Transformation::translation(Vector3d(1, 0, 0)) * Transformation::rotation(Vector3d(0, 0, 1), -openstudio::degToRad(90))));
  EXPECT_DOUBLE_EQ(1, space.xOrigin());
  EXPECT_DOUBLE_EQ(0, space.yOrigin());
  EXPECT_DOUBLE_EQ(0, space.zOrigin());
  EXPECT_DOUBLE_EQ(90, space.directionofRelativeNorth());

  transformation = space.transformation();
  test = transformation * origin;
  EXPECT_EQ(1, test.x());
  EXPECT_EQ(0, test.y());
  EXPECT_EQ(0, test.z());

  test = transformation * x1;
  EXPECT_DOUBLE_EQ(1, test.x());
  EXPECT_DOUBLE_EQ(-1, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());

  EXPECT_TRUE(space.setTransformation(Transformation()));

  transformation = space.transformation();
  test = transformation * origin;
  EXPECT_DOUBLE_EQ(0, test.x());
  EXPECT_DOUBLE_EQ(0, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());

  test = transformation * x1;
  EXPECT_DOUBLE_EQ(1, test.x());
  EXPECT_DOUBLE_EQ(0, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());

  EXPECT_TRUE(space.changeTransformation(Transformation::translation(Vector3d(1, 0, 0))
                                         * Transformation::rotation(Vector3d(0, 0, 1), -openstudio::degToRad(90))));
  EXPECT_DOUBLE_EQ(1, space.xOrigin());
  EXPECT_DOUBLE_EQ(0, space.yOrigin());
  EXPECT_DOUBLE_EQ(0, space.zOrigin());
  EXPECT_DOUBLE_EQ(90, space.directionofRelativeNorth());

  transformation = space.transformation();
  test = transformation * origin;
  EXPECT_EQ(1, test.x());
  EXPECT_EQ(0, test.y());
  EXPECT_EQ(0, test.z());

  test = transformation * x1;
  EXPECT_DOUBLE_EQ(1, test.x());
  EXPECT_DOUBLE_EQ(-1, test.y());
  EXPECT_DOUBLE_EQ(0, test.z());
}

TEST_F(ModelFixture, Space_Transformation2) {
  Model model;
  Space space(model);
  Point3dVector points;
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(1, 1, 0));
  Surface surface(points, model);
  surface.setSpace(space);

  EXPECT_TRUE(circularEqual(points, space.transformation() * points));
  EXPECT_TRUE(circularEqual(points, space.transformation() * surface.vertices()));

  EXPECT_TRUE(space.changeTransformation(Transformation::translation(Vector3d(1, 0, 0))
                                         * Transformation::rotation(Vector3d(0, 0, 1), -openstudio::degToRad(90))));

  EXPECT_FALSE(circularEqual(points, space.transformation() * points));
  EXPECT_TRUE(circularEqual(points, space.transformation() * surface.vertices()));
}

TEST_F(ModelFixture, Space_IddAssumptions) {
  Model model;
  Space space(model);

  EXPECT_TRUE(space.partofTotalFloorArea());
}

TEST_F(ModelFixture, Space_BoundingBox) {
  Model model;

  Point3dVector points;
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(1, 1, 0));

  Space space1(model);
  Surface surface1(points, model);
  surface1.setSpace(space1);

  Space space2(model);
  Surface surface2(points, model);
  surface2.setSpace(space2);

  BoundingBox box1 = space1.transformation() * space1.boundingBox();
  BoundingBox box2 = space2.transformation() * space2.boundingBox();

  EXPECT_TRUE(box1.intersects(box2));

  EXPECT_TRUE(space2.setTransformation(Transformation::translation(Vector3d(1, 0, 0))));

  box2 = space2.transformation() * space2.boundingBox();

  EXPECT_TRUE(box1.intersects(box2));

  EXPECT_TRUE(space2.setTransformation(Transformation::translation(Vector3d(1, 1, 0))));

  box2 = space2.transformation() * space2.boundingBox();

  EXPECT_TRUE(box1.intersects(box2));

  EXPECT_TRUE(space2.setTransformation(Transformation::translation(Vector3d(2, 2, 0))));

  box2 = space2.transformation() * space2.boundingBox();

  EXPECT_FALSE(box1.intersects(box2));
}

TEST_F(ModelFixture, Space_hardApplySpaceType_false) {
  Model model;

  SpaceType spaceType(model);

  Building building = model.getUniqueModelObject<Building>();
  building.setSpaceType(spaceType);

  Space space(model);
  EXPECT_TRUE(space.isSpaceTypeDefaulted());
  ASSERT_TRUE(space.spaceType());
  EXPECT_EQ(spaceType.handle(), space.spaceType()->handle());

  EXPECT_EQ(0u, space.lights().size());
  EXPECT_EQ(0, space.lightingPowerPerFloorArea());
  EXPECT_EQ(0u, spaceType.lights().size());

  LightsDefinition definition(model);
  definition.setWattsperSpaceFloorArea(1.0);

  Lights lights(definition);
  lights.setSpaceType(spaceType);

  EXPECT_FALSE(lights.space());
  ASSERT_TRUE(lights.spaceType());
  EXPECT_EQ(spaceType.handle(), lights.spaceType()->handle());

  EXPECT_EQ(0u, space.lights().size());
  EXPECT_EQ(1.0, space.lightingPowerPerFloorArea());
  ASSERT_EQ(1u, spaceType.lights().size());
  EXPECT_EQ(lights.handle(), spaceType.lights()[0].handle());

  space.hardApplySpaceType(false);

  EXPECT_FALSE(space.isSpaceTypeDefaulted());
  ASSERT_TRUE(space.spaceType());
  EXPECT_NE(spaceType.handle(), space.spaceType()->handle());
  EXPECT_EQ(1u, spaceType.lights().size());
  EXPECT_EQ(0u, space.spaceType()->lights().size());

  // existing light is still attached to space type, new clone is attached to space
  EXPECT_EQ(2u, model.getModelObjects<Lights>().size());
  EXPECT_FALSE(lights.space());
  ASSERT_TRUE(lights.spaceType());
  EXPECT_EQ(spaceType.handle(), lights.spaceType()->handle());

  ASSERT_EQ(1u, space.lights().size());
  EXPECT_NE(lights.handle(), space.lights()[0].handle());
  EXPECT_EQ(1.0, space.lightingPowerPerFloorArea());
  EXPECT_EQ(1u, spaceType.lights().size());
}

TEST_F(ModelFixture, Space_SurfaceMatch_2WallsWindow) {
  Model model;
  Space space1(model);
  Space space2(model);

  // outward normal is in -y direction
  Point3dVector points;
  points.push_back(Point3d(0, -10, 10));
  points.push_back(Point3d(0, -10, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(0, 0, 10));
  Surface wall1(points, model);
  wall1.setSpace(space1);

  EXPECT_EQ("Wall", wall1.surfaceType());
  EXPECT_EQ("Outdoors", wall1.outsideBoundaryCondition());
  EXPECT_FALSE(wall1.adjacentSurface());

  points.clear();
  points.push_back(Point3d(0, -7, 7));
  points.push_back(Point3d(0, -7, 3));
  points.push_back(Point3d(0, -3, 3));
  points.push_back(Point3d(0, -3, 7));
  SubSurface window1(points, model);
  window1.setSurface(wall1);

  EXPECT_EQ("FixedWindow", window1.subSurfaceType());
  EXPECT_FALSE(window1.adjacentSubSurface());

  // outward normal in +y
  points.clear();
  points.push_back(Point3d(0, 0, 10));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(0, -10, 0));
  points.push_back(Point3d(0, -10, 10));
  Surface wall2(points, model);
  wall2.setSpace(space2);

  EXPECT_EQ("Wall", wall2.surfaceType());
  EXPECT_EQ("Outdoors", wall2.outsideBoundaryCondition());
  EXPECT_FALSE(wall2.adjacentSurface());

  points.clear();
  points.push_back(Point3d(0, -3, 7));
  points.push_back(Point3d(0, -3, 3));
  points.push_back(Point3d(0, -7, 3));
  points.push_back(Point3d(0, -7, 7));
  SubSurface window2(points, model);
  window2.setSurface(wall2);

  EXPECT_EQ("FixedWindow", window2.subSurfaceType());
  EXPECT_FALSE(window2.adjacentSubSurface());

  space1.matchSurfaces(space2);

  EXPECT_EQ("Wall", wall1.surfaceType());
  EXPECT_EQ("Surface", wall1.outsideBoundaryCondition());
  ASSERT_TRUE(wall1.adjacentSurface());
  EXPECT_EQ(wall2.handle(), wall1.adjacentSurface()->handle());

  EXPECT_EQ("FixedWindow", window1.subSurfaceType());
  ASSERT_TRUE(window1.adjacentSubSurface());
  EXPECT_EQ(window2.handle(), window1.adjacentSubSurface()->handle());

  EXPECT_EQ("Wall", wall2.surfaceType());
  EXPECT_EQ("Surface", wall2.outsideBoundaryCondition());
  ASSERT_TRUE(wall2.adjacentSurface());
  EXPECT_EQ(wall1.handle(), wall2.adjacentSurface()->handle());

  EXPECT_EQ("FixedWindow", window2.subSurfaceType());
  ASSERT_TRUE(window2.adjacentSubSurface());
  EXPECT_EQ(window1.handle(), window2.adjacentSubSurface()->handle());

  space1.unmatchSurfaces();

  EXPECT_EQ("Wall", wall1.surfaceType());
  EXPECT_EQ("Outdoors", wall1.outsideBoundaryCondition());
  EXPECT_FALSE(wall1.adjacentSurface());

  EXPECT_EQ("FixedWindow", window1.subSurfaceType());
  EXPECT_FALSE(window1.adjacentSubSurface());

  EXPECT_EQ("Wall", wall2.surfaceType());
  EXPECT_EQ("Outdoors", wall2.outsideBoundaryCondition());
  EXPECT_FALSE(wall2.adjacentSurface());

  EXPECT_EQ("FixedWindow", window2.subSurfaceType());
  EXPECT_FALSE(window2.adjacentSubSurface());
}

TEST_F(ModelFixture, Space_SurfaceMatch_LargeTest) {
  Model model;

  Point3dVector points;
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(0, 0, 0));

  int Nx = 2;
  int Ny = 2;
  int Nz = 2;

  for (int i = 0; i < Nx; ++i) {
    for (int j = 0; j < Ny; ++j) {
      for (int k = 0; k < Nz; ++k) {
        boost::optional<Space> space = Space::fromFloorPrint(points, 1, model);
        ASSERT_TRUE(space);
        space->setXOrigin(i);
        space->setYOrigin(j);
        space->setZOrigin(k);

        Point3dVector subSurfacePoints;
        std::vector<Surface> searchResults;

        // add window to north wall
        searchResults = space->findSurfaces(0.0, 0.0, 90.0, 90.0);
        ASSERT_EQ(1u, searchResults.size());

        subSurfacePoints.clear();
        subSurfacePoints.push_back(Point3d(0.75, 1.0, 0.75));
        subSurfacePoints.push_back(Point3d(0.75, 1.0, 0.25));
        subSurfacePoints.push_back(Point3d(0.25, 1.0, 0.25));
        subSurfacePoints.push_back(Point3d(0.25, 1.0, 0.75));

        SubSurface northWindow(subSurfacePoints, model);
        northWindow.setSurface(searchResults[0]);

        // add window to south wall
        searchResults = space->findSurfaces(180.0, 180.0, 90.0, 90.0);
        ASSERT_EQ(1u, searchResults.size());

        subSurfacePoints.clear();
        subSurfacePoints.push_back(Point3d(0.25, 0.0, 0.75));
        subSurfacePoints.push_back(Point3d(0.25, 0.0, 0.25));
        subSurfacePoints.push_back(Point3d(0.75, 0.0, 0.25));
        subSurfacePoints.push_back(Point3d(0.75, 0.0, 0.75));

        SubSurface southWindow(subSurfacePoints, model);
        southWindow.setSurface(searchResults[0]);

        // add door to east wall
        searchResults = space->findSurfaces(90.0, 90.0, 90.0, 90.0);
        ASSERT_EQ(1u, searchResults.size());

        subSurfacePoints.clear();
        subSurfacePoints.push_back(Point3d(1.0, 0.25, 0.75));
        subSurfacePoints.push_back(Point3d(1.0, 0.25, 0.25));
        subSurfacePoints.push_back(Point3d(1.0, 0.75, 0.25));
        subSurfacePoints.push_back(Point3d(1.0, 0.75, 0.75));

        SubSurface eastDoor(subSurfacePoints, model);
        eastDoor.setSurface(searchResults[0]);

        // add door to west wall
        searchResults = space->findSurfaces(270.0, 270.0, 90.0, 90.0);
        ASSERT_EQ(1u, searchResults.size());

        subSurfacePoints.clear();
        subSurfacePoints.push_back(Point3d(0.0, 0.75, 0.75));
        subSurfacePoints.push_back(Point3d(0.0, 0.75, 0.25));
        subSurfacePoints.push_back(Point3d(0.0, 0.25, 0.25));
        subSurfacePoints.push_back(Point3d(0.0, 0.25, 0.75));

        SubSurface westDoor(subSurfacePoints, model);
        westDoor.setSurface(searchResults[0]);
      }
    }
  }

  SpaceVector spaces = model.getModelObjects<Space>();
  matchSurfaces(spaces);

  // model.save(toPath("./Space_SurfaceMatch_LargeTest.osm"), true);
}

TEST_F(ModelFixture, Space_FindSurfaces) {
  Model model;

  Point3dVector floorPrint;
  floorPrint.push_back(Point3d(0, 10, 0));
  floorPrint.push_back(Point3d(10, 10, 0));
  floorPrint.push_back(Point3d(10, 0, 0));
  floorPrint.push_back(Point3d(0, 0, 0));

  boost::optional<Space> space1 = Space::fromFloorPrint(floorPrint, 3, model);
  ASSERT_TRUE(space1);

  std::vector<Surface> searchResults;

  // find north wall
  searchResults = space1->findSurfaces(0.0, 0.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 1, 0).dot(searchResults[0].outwardNormal()));

  // find north wall
  searchResults = space1->findSurfaces(359.0, 1.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 1, 0).dot(searchResults[0].outwardNormal()));

  // find east wall
  searchResults = space1->findSurfaces(90.0, 90.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(1, 0, 0).dot(searchResults[0].outwardNormal()));

  // find south wall
  searchResults = space1->findSurfaces(180.0, 180.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, -1, 0).dot(searchResults[0].outwardNormal()));

  // find west wall
  searchResults = space1->findSurfaces(270.0, 270.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(-1, 0, 0).dot(searchResults[0].outwardNormal()));

  // find floor
  searchResults = space1->findSurfaces(boost::none, boost::none, 180.0, 180.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 0, -1).dot(searchResults[0].outwardNormal()));

  // find ceiling
  searchResults = space1->findSurfaces(boost::none, boost::none, 0.0, 0.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 0, 1).dot(searchResults[0].outwardNormal()));
}

TEST_F(ModelFixture, Space_FindSurfaces_Translated) {
  Model model;

  Point3dVector floorPrint;
  floorPrint.push_back(Point3d(0, 10, 0));
  floorPrint.push_back(Point3d(10, 10, 0));
  floorPrint.push_back(Point3d(10, 0, 0));
  floorPrint.push_back(Point3d(0, 0, 0));

  boost::optional<Space> space1 = Space::fromFloorPrint(floorPrint, 3, model);
  ASSERT_TRUE(space1);
  space1->setXOrigin(10);
  space1->setYOrigin(-10);

  std::vector<Surface> searchResults;

  // find north wall
  searchResults = space1->findSurfaces(0.0, 0.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 1, 0).dot(searchResults[0].outwardNormal()));

  // find north wall
  searchResults = space1->findSurfaces(359.0, 1.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 1, 0).dot(searchResults[0].outwardNormal()));

  // find east wall
  searchResults = space1->findSurfaces(90.0, 90.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(1, 0, 0).dot(searchResults[0].outwardNormal()));

  // find south wall
  searchResults = space1->findSurfaces(180.0, 180.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, -1, 0).dot(searchResults[0].outwardNormal()));

  // find west wall
  searchResults = space1->findSurfaces(270.0, 270.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(-1, 0, 0).dot(searchResults[0].outwardNormal()));

  // find floor
  searchResults = space1->findSurfaces(boost::none, boost::none, 180.0, 180.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 0, -1).dot(searchResults[0].outwardNormal()));

  // find ceiling
  searchResults = space1->findSurfaces(boost::none, boost::none, 0.0, 0.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 0, 1).dot(searchResults[0].outwardNormal()));
}

TEST_F(ModelFixture, Space_FindSurfaces_Translated_Rotated) {
  Model model;

  Point3dVector floorPrint;
  floorPrint.push_back(Point3d(0, 10, 0));
  floorPrint.push_back(Point3d(10, 10, 0));
  floorPrint.push_back(Point3d(10, 0, 0));
  floorPrint.push_back(Point3d(0, 0, 0));

  boost::optional<Space> space1 = Space::fromFloorPrint(floorPrint, 3, model);
  ASSERT_TRUE(space1);
  space1->setXOrigin(10.0);
  space1->setYOrigin(-10.0);
  space1->setDirectionofRelativeNorth(20.0);

  std::vector<Surface> searchResults;

  // find north wall
  searchResults = space1->findSurfaces(20.0, 20.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 1, 0).dot(searchResults[0].outwardNormal()));

  // find east wall
  searchResults = space1->findSurfaces(110.0, 110.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(1, 0, 0).dot(searchResults[0].outwardNormal()));

  // find south wall
  searchResults = space1->findSurfaces(200.0, 200.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, -1, 0).dot(searchResults[0].outwardNormal()));

  // find west wall
  searchResults = space1->findSurfaces(290.0, 290.0, 90.0, 90.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(-1, 0, 0).dot(searchResults[0].outwardNormal()));

  // find floor
  searchResults = space1->findSurfaces(boost::none, boost::none, 180.0, 180.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 0, -1).dot(searchResults[0].outwardNormal()));

  // find ceiling
  searchResults = space1->findSurfaces(boost::none, boost::none, 0.0, 0.0);
  ASSERT_TRUE(searchResults.size() == 1);
  EXPECT_DOUBLE_EQ(1, Vector3d(0, 0, 1).dot(searchResults[0].outwardNormal()));
}

TEST_F(ModelFixture, Space_Remove) {
  Model model;

  Space space(model);
  WorkspaceObjectWatcher spaceWatcher(space);
  EXPECT_FALSE(spaceWatcher.dirty());
  EXPECT_FALSE(spaceWatcher.relationshipChanged());
  EXPECT_FALSE(spaceWatcher.removedFromWorkspace());

  Point3dVector points;
  points.push_back(Point3d(0, 4, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(4, 0, 0));
  points.push_back(Point3d(4, 4, 0));

  Surface surface(points, model);
  surface.setParent(space);

  WorkspaceObjectWatcher surfaceWatcher(surface);
  EXPECT_FALSE(surfaceWatcher.dirty());
  EXPECT_FALSE(surfaceWatcher.relationshipChanged());
  EXPECT_FALSE(surfaceWatcher.removedFromWorkspace());

  std::vector<Handle> handlesToRemove;
  handlesToRemove.push_back(space.handle());
  handlesToRemove.push_back(surface.handle());

  model.removeObjects(handlesToRemove);

  EXPECT_FALSE(spaceWatcher.dirty());
  EXPECT_FALSE(spaceWatcher.relationshipChanged());
  EXPECT_TRUE(spaceWatcher.removedFromWorkspace());
  EXPECT_FALSE(surfaceWatcher.dirty());
  EXPECT_FALSE(surfaceWatcher.relationshipChanged());
  EXPECT_TRUE(surfaceWatcher.removedFromWorkspace());
}

TEST_F(ModelFixture, Space_Remove2) {
  Model model;

  Space space(model);
  WorkspaceObjectWatcher spaceWatcher(space);
  EXPECT_FALSE(spaceWatcher.dirty());
  EXPECT_FALSE(spaceWatcher.relationshipChanged());
  EXPECT_FALSE(spaceWatcher.removedFromWorkspace());

  Point3dVector points;
  points.push_back(Point3d(0, 4, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(4, 0, 0));
  points.push_back(Point3d(4, 4, 0));

  Surface surface(points, model);
  surface.setParent(space);

  WorkspaceObjectWatcher surfaceWatcher(surface);
  EXPECT_FALSE(surfaceWatcher.dirty());
  EXPECT_FALSE(surfaceWatcher.relationshipChanged());
  EXPECT_FALSE(surfaceWatcher.removedFromWorkspace());

  std::vector<Handle> handlesToRemove;
  handlesToRemove.push_back(space.handle());
  //handlesToRemove.push_back(surface.handle());

  // this does not remove children
  model.removeObjects(handlesToRemove);

  EXPECT_FALSE(spaceWatcher.dirty());
  EXPECT_FALSE(spaceWatcher.relationshipChanged());
  EXPECT_TRUE(spaceWatcher.removedFromWorkspace());
  EXPECT_TRUE(surfaceWatcher.dirty());
  EXPECT_TRUE(surfaceWatcher.relationshipChanged());
  EXPECT_FALSE(surfaceWatcher.removedFromWorkspace());
}

TEST_F(ModelFixture, Space_FloorPrint1) {
  Model model;

  Space space(model);

  std::vector<Point3d> floorPrint = space.floorPrint();
  EXPECT_EQ(0, floorPrint.size());

  Point3dVector points;
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(0, 1, 0));

  Surface surface1(points, model);
  surface1.setParent(space);

  floorPrint = space.floorPrint();

  ASSERT_EQ(4u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(1, 1, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(1, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 1, 0))) << floorPrint;

  points.clear();
  points.push_back(Point3d(1, 2, 0));
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(0, 2, 0));

  Surface surface2(points, model);
  surface2.setParent(space);

  floorPrint = space.floorPrint();
  ASSERT_EQ(4u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(1, 2, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(1, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 2, 0))) << floorPrint;

  points.clear();
  points.push_back(Point3d(2, 2, 0));
  points.push_back(Point3d(2, 1, 0));
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(1, 2, 0));

  Surface surface3(points, model);
  surface3.setParent(space);

  floorPrint = space.floorPrint();
  ASSERT_EQ(6u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(2, 2, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(2, 1, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(1, 1, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(1, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[4], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[5], Point3d(0, 2, 0))) << floorPrint;

  points.clear();
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(2, 1, 0));
  points.push_back(Point3d(2, 0, 0));
  points.push_back(Point3d(1, 0, 0));

  Surface surface4(points, model);
  surface4.setParent(space);

  floorPrint = space.floorPrint();
  ASSERT_EQ(4u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(2, 2, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(2, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 2, 0))) << floorPrint;

  // now we will make a 3x3 square, then delete the center (surface3)

  points.clear();
  points.push_back(Point3d(1, 3, 0));
  points.push_back(Point3d(1, 2, 0));
  points.push_back(Point3d(0, 2, 0));
  points.push_back(Point3d(0, 3, 0));

  Surface surface5(points, model);
  surface5.setParent(space);

  points.clear();
  points.push_back(Point3d(2, 3, 0));
  points.push_back(Point3d(2, 2, 0));
  points.push_back(Point3d(1, 2, 0));
  points.push_back(Point3d(1, 3, 0));

  Surface surface6(points, model);
  surface6.setParent(space);

  points.clear();
  points.push_back(Point3d(3, 3, 0));
  points.push_back(Point3d(3, 2, 0));
  points.push_back(Point3d(2, 2, 0));
  points.push_back(Point3d(2, 3, 0));

  Surface surface7(points, model);
  surface7.setParent(space);

  points.clear();
  points.push_back(Point3d(3, 2, 0));
  points.push_back(Point3d(3, 1, 0));
  points.push_back(Point3d(2, 1, 0));
  points.push_back(Point3d(2, 2, 0));

  Surface surface8(points, model);
  surface8.setParent(space);

  points.clear();
  points.push_back(Point3d(3, 1, 0));
  points.push_back(Point3d(3, 0, 0));
  points.push_back(Point3d(2, 0, 0));
  points.push_back(Point3d(2, 1, 0));

  Surface surface9(points, model);
  surface9.setParent(space);

  floorPrint = space.floorPrint();
  ASSERT_EQ(4u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(3, 3, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(3, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 3, 0))) << floorPrint;

  surface3.remove();

  floorPrint = space.floorPrint();
  ASSERT_EQ(10u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(3, 3, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(3, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 3, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[4], Point3d(1, 2, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[5], Point3d(1, 1, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[6], Point3d(2, 1, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[7], Point3d(2, 2, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[8], Point3d(1, 2, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[9], Point3d(0, 3, 0))) << floorPrint;
}

TEST_F(ModelFixture, Space_FloorPrint2) {
  Model model;

  Space space(model);

  std::vector<Point3d> floorPrint = space.floorPrint();
  EXPECT_EQ(0, floorPrint.size());

  Point3dVector points;
  points.push_back(Point3d(1, 1, 0));
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(0, 0, 0));
  points.push_back(Point3d(0, 1, 0));

  Surface surface1(points, model);
  surface1.setParent(space);

  floorPrint = space.floorPrint();

  ASSERT_EQ(4u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(1, 1, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(1, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 1, 0))) << floorPrint;

  points.clear();
  points.push_back(Point3d(2, 1, 0));
  points.push_back(Point3d(2, 0, 0));
  points.push_back(Point3d(1, 0, 0));
  points.push_back(Point3d(1, 1, 0));

  Surface surface2(points, model);
  surface2.setParent(space);

  floorPrint = space.floorPrint();

  ASSERT_EQ(4u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(2, 1, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(2, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 1, 0))) << floorPrint;

  points.clear();
  points.push_back(Point3d(2, 2, 0));
  points.push_back(Point3d(2, 1, 0));
  points.push_back(Point3d(0, 1, 0));
  points.push_back(Point3d(0, 2, 0));

  Surface surface3(points, model);
  surface3.setParent(space);

  floorPrint = space.floorPrint();

  ASSERT_EQ(4u, floorPrint.size()) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[0], Point3d(2, 2, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[1], Point3d(2, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[2], Point3d(0, 0, 0))) << floorPrint;
  EXPECT_DOUBLE_EQ(0, getDistance(floorPrint[3], Point3d(0, 2, 0))) << floorPrint;
}

TEST_F(ModelFixture, Space_Cost) {
  Model model;

  Point3dVector floorPrint;
  floorPrint.push_back(Point3d(0, 10, 0));
  floorPrint.push_back(Point3d(10, 10, 0));
  floorPrint.push_back(Point3d(10, 0, 0));
  floorPrint.push_back(Point3d(0, 0, 0));

  boost::optional<Space> space = Space::fromFloorPrint(floorPrint, 3, model);
  ASSERT_TRUE(space);

  boost::optional<LifeCycleCost> cost1 = LifeCycleCost::createLifeCycleCost("Cleaning", *space, 1, "CostPerArea", "Maintenance", 1, 0);
  ASSERT_TRUE(cost1);
  boost::optional<LifeCycleCost> cost2 = LifeCycleCost::createLifeCycleCost("Design", *space, 100, "CostPerEach", "Construction", 0, 0);
  ASSERT_TRUE(cost2);

  EXPECT_DOUBLE_EQ(100, cost1->totalCost());
  EXPECT_DOUBLE_EQ(100, cost2->totalCost());

  ThermalZone thermalZone(model);
  space->setThermalZone(thermalZone);
  thermalZone.setMultiplier(2);

  EXPECT_DOUBLE_EQ(200, cost1->totalCost());
  EXPECT_DOUBLE_EQ(200, cost2->totalCost());
}

TEST_F(ModelFixture, Space_InfiltrationDesignFlowRate) {
  // create from floor print
  Model model;
  Point3dVector floorPrint;
  floorPrint.push_back(Point3d(0, 10, 0));
  floorPrint.push_back(Point3d(10, 10, 0));
  floorPrint.push_back(Point3d(10, 0, 0));
  floorPrint.push_back(Point3d(0, 0, 0));
  boost::optional<Space> ospace = Space::fromFloorPrint(floorPrint, 3.6, model);
  ASSERT_TRUE(ospace);
  Space space = *ospace;

  // check dimensions
  EXPECT_DOUBLE_EQ(100.0, space.floorArea());
  EXPECT_DOUBLE_EQ(360.0, space.volume());
  EXPECT_DOUBLE_EQ(144.0, space.exteriorWallArea());
  EXPECT_DOUBLE_EQ(244.0, space.exteriorArea());  // ground does not count
  EXPECT_DOUBLE_EQ(0.0, space.infiltrationDesignFlowRate());

  // add an infiltration object
  SpaceInfiltrationDesignFlowRate spaceInfiltration(model);
  spaceInfiltration.setSpace(space);
  spaceInfiltration.setAirChangesperHour(1.0);

  // check infiltration getters
  EXPECT_DOUBLE_EQ(0.1, space.infiltrationDesignFlowRate());
  EXPECT_DOUBLE_EQ(0.001, space.infiltrationDesignFlowPerSpaceFloorArea());
  EXPECT_DOUBLE_EQ(4.0983606557377049E-4, space.infiltrationDesignFlowPerExteriorSurfaceArea());
  EXPECT_DOUBLE_EQ(6.9444444444444447E-4, space.infiltrationDesignFlowPerExteriorWallArea());
  EXPECT_DOUBLE_EQ(1.0, space.infiltrationDesignAirChangesPerHour());

  // create a space type with infiltration
  SpaceType spaceType(model);
  SpaceInfiltrationDesignFlowRate spaceTypeInfiltration(model);
  spaceTypeInfiltration.setSpaceType(spaceType);
  spaceTypeInfiltration.setFlowperExteriorWallArea(5.0E-4);
  space.setSpaceType(spaceType);

  // check infiltration getters again
  EXPECT_DOUBLE_EQ(0.172, space.infiltrationDesignFlowRate());
  EXPECT_DOUBLE_EQ(0.00172, space.infiltrationDesignFlowPerSpaceFloorArea());
  EXPECT_DOUBLE_EQ(7.0491803278688531E-4, space.infiltrationDesignFlowPerExteriorSurfaceArea());
  EXPECT_DOUBLE_EQ(11.944444444444446E-4, space.infiltrationDesignFlowPerExteriorWallArea());
  EXPECT_DOUBLE_EQ(1.72, space.infiltrationDesignAirChangesPerHour());

  // go ahead and check building now
  Building building = model.getUniqueModelObject<Building>();
  EXPECT_DOUBLE_EQ(0.172, building.infiltrationDesignFlowRate());
  EXPECT_DOUBLE_EQ(0.00172, building.infiltrationDesignFlowPerSpaceFloorArea());
  EXPECT_DOUBLE_EQ(7.0491803278688531E-4, building.infiltrationDesignFlowPerExteriorSurfaceArea());
  EXPECT_DOUBLE_EQ(11.944444444444446E-4, building.infiltrationDesignFlowPerExteriorWallArea());
  EXPECT_DOUBLE_EQ(1.72, building.infiltrationDesignAirChangesPerHour());
}

TEST_F(ModelFixture, Space_Plenum) {
  model::Model model = openstudio::model::Model();

  model::SpaceType spaceType(model);

  model::Building building = model.getUniqueModelObject<model::Building>();
  EXPECT_TRUE(building.setSpaceType(spaceType));

  EXPECT_EQ(1u, model.getConcreteModelObjects<model::SpaceType>().size());

  model::ThermalZone supplyZone(model);
  model::ThermalZone zone(model);
  model::ThermalZone returnZone(model);

  model::Space supplySpace(model);
  EXPECT_TRUE(supplySpace.setThermalZone(supplyZone));
  model::Space space(model);
  EXPECT_TRUE(space.setThermalZone(zone));
  model::Space returnSpace(model);
  EXPECT_TRUE(returnSpace.setThermalZone(returnZone));

  ASSERT_TRUE(supplySpace.spaceType());
  EXPECT_EQ(spaceType.handle(), supplySpace.spaceType()->handle());
  EXPECT_TRUE(supplySpace.partofTotalFloorArea());
  ASSERT_TRUE(space.spaceType());
  EXPECT_EQ(spaceType.handle(), space.spaceType()->handle());
  EXPECT_TRUE(space.partofTotalFloorArea());
  ASSERT_TRUE(returnSpace.spaceType());
  EXPECT_EQ(spaceType.handle(), returnSpace.spaceType()->handle());
  EXPECT_TRUE(returnSpace.partofTotalFloorArea());

  model::AirLoopHVAC airLoopHVAC = openstudio::model::AirLoopHVAC(model);

  bool result = airLoopHVAC.addBranchForZone(zone);
  EXPECT_TRUE(result);
  result = zone.setSupplyPlenum(supplyZone);
  EXPECT_TRUE(result);
  result = zone.setReturnPlenum(returnZone);
  EXPECT_TRUE(result);

  ASSERT_TRUE(supplySpace.spaceType());
  EXPECT_NE(spaceType.handle(), supplySpace.spaceType()->handle());
  EXPECT_FALSE(supplySpace.partofTotalFloorArea());
  ASSERT_TRUE(space.spaceType());
  EXPECT_EQ(spaceType.handle(), space.spaceType()->handle());
  EXPECT_TRUE(space.partofTotalFloorArea());
  ASSERT_TRUE(returnSpace.spaceType());
  EXPECT_NE(spaceType.handle(), returnSpace.spaceType()->handle());
  EXPECT_FALSE(returnSpace.partofTotalFloorArea());

  EXPECT_EQ(2u, model.getConcreteModelObjects<model::SpaceType>().size());

  SpaceType plenumSpaceType = model.plenumSpaceType();

  EXPECT_EQ(2u, model.getConcreteModelObjects<model::SpaceType>().size());

  ASSERT_TRUE(supplySpace.spaceType());
  EXPECT_EQ(plenumSpaceType.handle(), supplySpace.spaceType()->handle());
  EXPECT_FALSE(supplySpace.partofTotalFloorArea());
  ASSERT_TRUE(space.spaceType());
  EXPECT_EQ(spaceType.handle(), space.spaceType()->handle());
  EXPECT_TRUE(space.partofTotalFloorArea());
  ASSERT_TRUE(returnSpace.spaceType());
  EXPECT_EQ(plenumSpaceType.handle(), returnSpace.spaceType()->handle());
  EXPECT_FALSE(returnSpace.partofTotalFloorArea());

  std::vector<Space> plenumSpaces = plenumSpaceType.spaces();
  ASSERT_EQ(2u, plenumSpaces.size());

  EXPECT_EQ("Plenum Space Type", plenumSpaceType.nameString());
  plenumSpaceType.setNameProtected("Some Other Name");
  EXPECT_EQ("Plenum Space Type", plenumSpaceType.nameString());

  EXPECT_EQ("Plenum Space Type", plenumSpaceType.nameString());
  plenumSpaceType.setName("Some Other Name");
  EXPECT_EQ("Some Other Name", plenumSpaceType.nameString());

  plenumSpaceType = model.plenumSpaceType();
  EXPECT_EQ(3u, model.getConcreteModelObjects<model::SpaceType>().size());

  ASSERT_TRUE(supplySpace.spaceType());
  EXPECT_EQ(plenumSpaceType.handle(), supplySpace.spaceType()->handle());
  EXPECT_FALSE(supplySpace.partofTotalFloorArea());
  ASSERT_TRUE(space.spaceType());
  EXPECT_EQ(spaceType.handle(), space.spaceType()->handle());
  EXPECT_TRUE(space.partofTotalFloorArea());
  ASSERT_TRUE(returnSpace.spaceType());
  EXPECT_EQ(plenumSpaceType.handle(), returnSpace.spaceType()->handle());
  EXPECT_FALSE(returnSpace.partofTotalFloorArea());

  plenumSpaces = plenumSpaceType.spaces();
  ASSERT_EQ(2u, plenumSpaces.size());
}

TEST_F(ModelFixture, Space_Intersect_OneToFour) {

  double areaTol = 0.000001;
  double xOrigin = 20.0;

  // space 1 has one large surface, space 2 has 4 rectangles, test that intersection is correct independent of rotation and intersect order
  for (double rotation = 0; rotation < 360.0; rotation += 10.0) {
    for (unsigned iStart = 0; iStart < 4; ++iStart) {

      Transformation t = Transformation::rotation(Vector3d(0, 0, 1), degToRad(rotation));

      Model model;
      Space space1(model);
      Space space2(model);

      Point3dVector points;
      points.push_back(Point3d(xOrigin, 0, 20));
      points.push_back(Point3d(xOrigin, 0, 0));
      points.push_back(Point3d(xOrigin, 10, 0));
      points.push_back(Point3d(xOrigin, 10, 20));
      Surface surface(t * points, model);
      surface.setSpace(space1);
      EXPECT_NEAR(200.0, surface.grossArea(), areaTol);

      std::vector<Surface> surfaces;
      for (unsigned i = 0; i < 4; ++i) {
        points.clear();
        points.push_back(Point3d(xOrigin, 10, (i + 1) * 5));
        points.push_back(Point3d(xOrigin, 10, i * 5));
        points.push_back(Point3d(xOrigin, 0, i * 5));
        points.push_back(Point3d(xOrigin, 0, (i + 1) * 5));
        Surface tempSurface(t * points, model);
        tempSurface.setSpace(space2);
        EXPECT_NEAR(50.0, tempSurface.grossArea(), areaTol);
        surfaces.push_back(tempSurface);
      }

      space1.intersectSurfaces(space2);
      space1.matchSurfaces(space2);

      EXPECT_EQ(4u, space1.surfaces().size());
      for (const Surface& s : space1.surfaces()) {
        EXPECT_EQ(4u, s.vertices().size());
        EXPECT_NEAR(50.0, s.grossArea(), areaTol);
        EXPECT_TRUE(s.adjacentSurface());
      }

      EXPECT_EQ(4u, space2.surfaces().size());
      for (const Surface& s : space2.surfaces()) {
        EXPECT_EQ(4u, s.vertices().size());
        EXPECT_NEAR(50.0, s.grossArea(), areaTol);
        EXPECT_TRUE(s.adjacentSurface());
      }
    }
  }
}

TEST_F(ModelFixture, Space_Intersect_FourToOne) {

  double areaTol = 0.000001;
  double xOrigin = 20.0;

  // space 1 has one large surface, space 2 has 4 rectangles, test that intersection is correct independent of rotation and intersect order
  for (double rotation = 0; rotation < 360.0; rotation += 10.0) {
    for (unsigned iStart = 0; iStart < 4; ++iStart) {

      Transformation t = Transformation::rotation(Vector3d(0, 0, 1), degToRad(rotation));

      Model model;
      Space space1(model);
      Space space2(model);

      Point3dVector points;
      points.push_back(Point3d(xOrigin, 0, 20));
      points.push_back(Point3d(xOrigin, 0, 0));
      points.push_back(Point3d(xOrigin, 10, 0));
      points.push_back(Point3d(xOrigin, 10, 20));
      Surface surface(t * points, model);
      surface.setSpace(space1);
      EXPECT_NEAR(200.0, surface.grossArea(), areaTol);

      std::vector<Surface> surfaces;
      for (unsigned i = 0; i < 4; ++i) {
        points.clear();
        points.push_back(Point3d(xOrigin, 10, (i + 1) * 5));
        points.push_back(Point3d(xOrigin, 10, i * 5));
        points.push_back(Point3d(xOrigin, 0, i * 5));
        points.push_back(Point3d(xOrigin, 0, (i + 1) * 5));
        Surface tempSurface(t * points, model);
        tempSurface.setSpace(space2);
        EXPECT_NEAR(50.0, tempSurface.grossArea(), areaTol);
        surfaces.push_back(tempSurface);
      }

      space2.intersectSurfaces(space1);
      space2.matchSurfaces(space1);

      EXPECT_EQ(4u, space1.surfaces().size());
      for (const Surface& s : space1.surfaces()) {
        EXPECT_EQ(4u, s.vertices().size());
        EXPECT_NEAR(50.0, s.grossArea(), areaTol);
        EXPECT_TRUE(s.adjacentSurface());
      }

      EXPECT_EQ(4u, space2.surfaces().size());
      for (const Surface& s : space2.surfaces()) {
        EXPECT_EQ(4u, s.vertices().size());
        EXPECT_NEAR(50.0, s.grossArea(), areaTol);
        EXPECT_TRUE(s.adjacentSurface());
      }
    }
  }
}

TEST_F(ModelFixture, Space_LifeCycleCost) {
  Model model;
  Space space(model);
  EXPECT_EQ(0, space.lifeCycleCosts().size());
  EXPECT_EQ(0, model.getConcreteModelObjects<LifeCycleCost>().size());
  boost::optional<LifeCycleCost> lifeCycleCost = LifeCycleCost::createLifeCycleCost("New Cost", space, 1, "CostPerEach", "Construction");
  ASSERT_TRUE(lifeCycleCost);
  EXPECT_EQ(1u, space.lifeCycleCosts().size());
  EXPECT_EQ(1u, model.getConcreteModelObjects<LifeCycleCost>().size());
  space.remove();
  EXPECT_TRUE(lifeCycleCost->handle().isNull());
  EXPECT_EQ(0, model.getConcreteModelObjects<LifeCycleCost>().size());
}

TEST_F(ModelFixture, Space_hardApplySpaceType_Plenum) {
  Model m;
  Space s(m);
  ThermalZone z(m);
  s.setThermalZone(z);

  AirLoopHVACSupplyPlenum a(m);
  a.setThermalZone(z);

  ASSERT_EQ(m.plenumSpaceType().handle(), s.spaceType().get().handle());

  // This should not do anything, space type should change
  s.hardApplySpaceType(true);

  ASSERT_NE(m.plenumSpaceType().handle(), s.spaceType().get().handle());
}

TEST_F(ModelFixture, Space_hardApplySpaceType_Plenum2) {
  Model m;
  Space s1(m);
  Space s2(m);
  ThermalZone z(m);
  s1.setThermalZone(z);
  s2.setThermalZone(z);

  AirLoopHVACSupplyPlenum a(m);
  a.setThermalZone(z);

  ASSERT_EQ(m.plenumSpaceType().handle(), s1.spaceType().get().handle());
  ASSERT_EQ(m.plenumSpaceType().handle(), s2.spaceType().get().handle());

  // This should not do anything, space type should change
  s1.hardApplySpaceType(true);
  s2.hardApplySpaceType(true);

  ASSERT_NE(m.plenumSpaceType().handle(), s1.spaceType().get().handle());
  ASSERT_NE(m.plenumSpaceType().handle(), s2.spaceType().get().handle());
  ASSERT_NE(s1.spaceType().get().handle(), s2.spaceType().get().handle());
}

TEST_F(ModelFixture, Space_intersectSurfaces_degenerate1) {
  Model m;
  std::vector<Point3d> vertices;

  // bottom floor
  BuildingStory bottomStory(m);

  // bottom core
  vertices.clear();
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  boost::optional<Space> bottomCore = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomCore);
  bottomCore->setZOrigin(0);
  bottomCore->setBuildingStory(bottomStory);
  bottomCore->setName("bottomCore");

  // bottom top
  vertices.clear();
  vertices.push_back(Point3d(-17.5976, 11.7318, 0));
  vertices.push_back(Point3d(17.5976, 11.7318, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  boost::optional<Space> bottomTop = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomTop);
  bottomTop->setZOrigin(0);
  bottomTop->setBuildingStory(bottomStory);
  bottomTop->setName("bottomTop");

  // bottom right
  vertices.clear();
  vertices.push_back(Point3d(17.5976, 11.7318, 0));
  vertices.push_back(Point3d(17.5976, -11.7318, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  boost::optional<Space> bottomRight = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomRight);
  bottomRight->setZOrigin(0);
  bottomRight->setBuildingStory(bottomStory);
  bottomRight->setName("bottomRight");

  // bottom bottom
  vertices.clear();
  vertices.push_back(Point3d(17.5976, -11.7318, 0));
  vertices.push_back(Point3d(-17.5976, -11.7318, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  boost::optional<Space> bottomBottom = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomBottom);
  bottomBottom->setZOrigin(0);
  bottomBottom->setBuildingStory(bottomStory);
  bottomBottom->setName("bottomBottom");

  // bottom left
  vertices.clear();
  vertices.push_back(Point3d(-17.5976, 11.7318, 0));
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  vertices.push_back(Point3d(-17.5976, -11.7318, 0));
  boost::optional<Space> bottomLeft = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomLeft);
  bottomLeft->setZOrigin(0);
  bottomLeft->setBuildingStory(bottomStory);
  bottomLeft->setName("bottomLeft");

  // top floor
  BuildingStory topStory(m);

  // top core
  vertices.clear();
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  boost::optional<Space> topCore = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topCore);
  topCore->setZOrigin(3);
  topCore->setBuildingStory(topStory);
  topCore->setName("topCore");

  // top top
  vertices.clear();
  vertices.push_back(Point3d(-12.4434, 8.2956, 0));
  vertices.push_back(Point3d(12.4434, 8.2956, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  boost::optional<Space> topTop = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topTop);
  topTop->setZOrigin(3);
  topTop->setBuildingStory(topStory);
  topTop->setName("topTop");

  // top right
  vertices.clear();
  vertices.push_back(Point3d(12.4434, 8.2956, 0));
  vertices.push_back(Point3d(12.4434, -8.2956, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  boost::optional<Space> topRight = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topRight);
  topRight->setZOrigin(3);
  topRight->setBuildingStory(topStory);
  topRight->setName("topRight");

  // top bottom
  vertices.clear();
  vertices.push_back(Point3d(12.4434, -8.2956, 0));
  vertices.push_back(Point3d(-12.4434, -8.2956, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  boost::optional<Space> topBottom = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topBottom);
  topBottom->setZOrigin(3);
  topBottom->setBuildingStory(topStory);
  topBottom->setName("topBottom");

  // top left
  vertices.clear();
  vertices.push_back(Point3d(-12.4434, 8.2956, 0));
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  vertices.push_back(Point3d(-12.4434, -8.2956, 0));
  boost::optional<Space> topLeft = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topLeft);
  topLeft->setZOrigin(3);
  topLeft->setBuildingStory(topStory);
  topLeft->setName("topLeft");

  // create thermal zones
  std::vector<Space> spaces = m.getConcreteModelObjects<Space>();
  for (auto& space : spaces) {
    ThermalZone z(m);
    space.setThermalZone(z);
  }

  // Rename surfaces to be able to debug what's going on
  for (auto& space : spaces) {
    int n_floors = 0;
    int n_walls = 0;
    int n_roofs = 0;
    for (auto& surface : space.surfaces()) {
      if (istringEqual("Floor", surface.surfaceType())) {
        surface.setName(space.nameString() + " Floor " + std::to_string(n_floors++));
      } else if (istringEqual("RoofCeiling", surface.surfaceType())) {
        surface.setName(space.nameString() + " RoofCeiling " + std::to_string(n_roofs++));
      } else if (istringEqual("Wall", surface.surfaceType())) {
        surface.setName(space.nameString() + " Wall " + std::to_string(n_walls++));
      }
    }
  }

  // TODO: Temp
  openstudio::path outpath = resourcesPath() / toPath("model/Space_intersectSurfaces_degenerate1_before_intersect.osm");
  m.save(outpath, true);

  intersectSurfaces(spaces);
  matchSurfaces(spaces);

  double exteriorFloorArea = 0;
  double interiorFloorArea = 0;
  double exteriorRoofArea = 0;
  double interiorRoofArea = 0;
  double exteriorWallArea = 0;
  double interiorWallArea = 0;
  int numRoofSurfaces = 0;

  std::vector<Surface> surfaces = m.getConcreteModelObjects<Surface>();
  for (auto& surface : surfaces) {
    if (istringEqual(surface.surfaceType(), "RoofCeiling")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Outdoors")) {
        exteriorRoofArea += surface.grossArea();
        ++numRoofSurfaces;
      } else {
        interiorRoofArea += surface.grossArea();
      }
    } else if (istringEqual(surface.surfaceType(), "Floor")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Ground")) {
        exteriorFloorArea += surface.grossArea();
      } else {
        interiorFloorArea += surface.grossArea();
      }
    } else if (istringEqual(surface.surfaceType(), "Wall")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Outdoors")) {
        exteriorWallArea += surface.grossArea();
      } else {
        interiorWallArea += surface.grossArea();
      }
    }
  }

  EXPECT_NEAR(exteriorFloorArea, 825.8048, 0.01);
  EXPECT_NEAR(interiorFloorArea, 412.9019, 0.01);
  EXPECT_NEAR(exteriorRoofArea, 825.8048, 0.01);
  EXPECT_NEAR(interiorRoofArea, 412.9019, 0.01);
  EXPECT_EQ(numRoofSurfaces, 11);

  outpath = resourcesPath() / toPath("model/Space_intersectSurfaces_degenerate1_after_intersect.osm");
  m.save(outpath, true);
  //m.save("intersect1.osm", true);
}

TEST_F(ModelFixture, Space_intersectSurfaces_degenerate2) {
  Model m;
  std::vector<Point3d> vertices;

  // bottom floor

  // bottom core
  vertices.clear();
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  boost::optional<Space> bottomCore = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomCore);
  bottomCore->setZOrigin(0);

  // bottom top
  vertices.clear();
  vertices.push_back(Point3d(-17.5976, 11.7318, 0));
  vertices.push_back(Point3d(17.5976, 11.7318, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  boost::optional<Space> bottomTop = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomTop);
  bottomTop->setZOrigin(0);

  // bottom right
  vertices.clear();
  vertices.push_back(Point3d(17.5976, 11.7318, 0));
  vertices.push_back(Point3d(17.5976, -11.7318, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  boost::optional<Space> bottomRight = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomRight);
  bottomRight->setZOrigin(0);

  // bottom bottom
  vertices.clear();
  vertices.push_back(Point3d(17.5976, -11.7318, 0));
  vertices.push_back(Point3d(-17.5976, -11.7318, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  boost::optional<Space> bottomBottom = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomBottom);
  bottomBottom->setZOrigin(0);

  // bottom left
  vertices.clear();
  vertices.push_back(Point3d(-17.5976, 11.7318, 0));
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  vertices.push_back(Point3d(-17.5976, -11.7318, 0));
  boost::optional<Space> bottomLeft = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomLeft);
  bottomLeft->setZOrigin(0);

  // top floor

  // top core
  vertices.clear();
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  boost::optional<Space> topCore = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topCore);
  topCore->setZOrigin(3);

  // top top
  vertices.clear();
  vertices.push_back(Point3d(-14.1614, 8.2956, 0));
  vertices.push_back(Point3d(14.1614, 8.2956, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  boost::optional<Space> topTop = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topTop);
  topTop->setZOrigin(3);

  // top right
  vertices.clear();
  vertices.push_back(Point3d(14.1614, 8.2956, 0));
  vertices.push_back(Point3d(14.1614, -8.2956, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  boost::optional<Space> topRight = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topRight);
  topRight->setZOrigin(3);

  // top bottom
  vertices.clear();
  vertices.push_back(Point3d(14.1614, -8.2956, 0));
  vertices.push_back(Point3d(-14.1614, -8.2956, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  boost::optional<Space> topBottom = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topBottom);
  topBottom->setZOrigin(3);

  // top left
  vertices.clear();
  vertices.push_back(Point3d(-14.1614, 8.2956, 0));
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  vertices.push_back(Point3d(-14.1614, -8.2956, 0));
  boost::optional<Space> topLeft = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topLeft);
  topLeft->setZOrigin(3);

  // create thermal zones
  std::vector<Space> spaces = m.getConcreteModelObjects<Space>();
  for (auto& space : spaces) {
    ThermalZone z(m);
    space.setThermalZone(z);
  }

  intersectSurfaces(spaces);
  matchSurfaces(spaces);

  double exteriorFloorArea = 0;
  double interiorFloorArea = 0;
  double exteriorRoofArea = 0;
  double interiorRoofArea = 0;
  double exteriorWallArea = 0;
  double interiorWallArea = 0;
  int numRoofSurfaces = 0;

  std::vector<Surface> surfaces = m.getConcreteModelObjects<Surface>();
  for (auto& surface : surfaces) {
    if (istringEqual(surface.surfaceType(), "RoofCeiling")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Outdoors")) {
        exteriorRoofArea += surface.grossArea();
        ++numRoofSurfaces;
      } else {
        interiorRoofArea += surface.grossArea();
      }
    } else if (istringEqual(surface.surfaceType(), "Floor")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Ground")) {
        exteriorFloorArea += surface.grossArea();
      } else {
        interiorFloorArea += surface.grossArea();
      }
    } else if (istringEqual(surface.surfaceType(), "Wall")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Outdoors")) {
        exteriorWallArea += surface.grossArea();
      } else {
        interiorWallArea += surface.grossArea();
      }
    }
  }

  // We know there are two small surfaces that are being generated that don't have opposite
  // surfaces and therefore are being set as Outdoors, the small surfaces are approx 0.008m2 and triangular.
  // Small surfaces but enougb to throw off the area check and the surface count (obviously)
  EXPECT_NEAR(exteriorFloorArea, 825.8048, 0.01);
  //EXPECT_NEAR(interiorFloorArea, 412.9019, 0.01);
  EXPECT_NEAR(exteriorRoofArea, 825.8048, 0.01);
  //EXPECT_NEAR(interiorRoofArea, 412.9019, 0.01);
  EXPECT_EQ(numRoofSurfaces, 9);

  //openstudio::path outpath = resourcesPath() / toPath("model/Space_intersectSurfaces_degenerate2_after_intersect.osm");
  //m.save(outpath, true);
}

TEST_F(ModelFixture, Space_intersectSurfaces_degenerate3) {
  Model m;
  std::vector<Point3d> vertices;

  // bottom floor

  // bottom core
  vertices.clear();
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  boost::optional<Space> bottomCore = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomCore);
  bottomCore->setZOrigin(0);

  // bottom top
  vertices.clear();
  vertices.push_back(Point3d(-17.5976, 11.7318, 0));
  vertices.push_back(Point3d(17.5976, 11.7318, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  boost::optional<Space> bottomTop = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomTop);
  bottomTop->setZOrigin(0);

  // bottom right
  vertices.clear();
  vertices.push_back(Point3d(17.5976, 11.7318, 0));
  vertices.push_back(Point3d(17.5976, -11.7318, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  vertices.push_back(Point3d(13.0256, 7.1598, 0));
  boost::optional<Space> bottomRight = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomRight);
  bottomRight->setZOrigin(0);

  // bottom bottom
  vertices.clear();
  vertices.push_back(Point3d(17.5976, -11.7318, 0));
  vertices.push_back(Point3d(-17.5976, -11.7318, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  vertices.push_back(Point3d(13.0256, -7.1598, 0));
  boost::optional<Space> bottomBottom = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomBottom);
  bottomBottom->setZOrigin(0);

  // bottom left
  vertices.clear();
  vertices.push_back(Point3d(-17.5976, 11.7318, 0));
  vertices.push_back(Point3d(-13.0256, 7.1598, 0));
  vertices.push_back(Point3d(-13.0256, -7.1598, 0));
  vertices.push_back(Point3d(-17.5976, -11.7318, 0));
  boost::optional<Space> bottomLeft = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(bottomLeft);
  bottomLeft->setZOrigin(0);

  // top floor

  // top core
  vertices.clear();
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  boost::optional<Space> topCore = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topCore);
  topCore->setZOrigin(3);

  // top top
  vertices.clear();
  vertices.push_back(Point3d(-15.4434, 8.2956, 0));
  vertices.push_back(Point3d(15.4434, 8.2956, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  boost::optional<Space> topTop = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topTop);
  topTop->setZOrigin(3);

  // top right
  vertices.clear();
  vertices.push_back(Point3d(15.4434, 8.2956, 0));
  vertices.push_back(Point3d(15.4434, -8.2956, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  vertices.push_back(Point3d(7.8714, 3.7236, 0));
  boost::optional<Space> topRight = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topRight);
  topRight->setZOrigin(3);

  // top bottom
  vertices.clear();
  vertices.push_back(Point3d(15.4434, -8.2956, 0));
  vertices.push_back(Point3d(-15.4434, -8.2956, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  vertices.push_back(Point3d(7.8714, -3.7236, 0));
  boost::optional<Space> topBottom = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topBottom);
  topBottom->setZOrigin(3);

  // top left
  vertices.clear();
  vertices.push_back(Point3d(-15.4434, 8.2956, 0));
  vertices.push_back(Point3d(-7.8714, 3.7236, 0));
  vertices.push_back(Point3d(-7.8714, -3.7236, 0));
  vertices.push_back(Point3d(-15.4434, -8.2956, 0));
  boost::optional<Space> topLeft = Space::fromFloorPrint(vertices, 3, m);
  ASSERT_TRUE(topLeft);
  topLeft->setZOrigin(3);

  // create thermal zones
  std::vector<Space> spaces = m.getConcreteModelObjects<Space>();
  for (auto& space : spaces) {
    ThermalZone z(m);
    space.setThermalZone(z);
  }

  intersectSurfaces(spaces);
  matchSurfaces(spaces);

  double exteriorFloorArea = 0;
  double interiorFloorArea = 0;
  double exteriorRoofArea = 0;
  double interiorRoofArea = 0;
  double exteriorWallArea = 0;
  double interiorWallArea = 0;
  int numRoofSurfaces = 0;

  std::vector<Surface> surfaces = m.getConcreteModelObjects<Surface>();
  for (auto& surface : surfaces) {
    if (istringEqual(surface.surfaceType(), "RoofCeiling")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Outdoors")) {
        exteriorRoofArea += surface.grossArea();
        ++numRoofSurfaces;
      } else {
        interiorRoofArea += surface.grossArea();
      }
    } else if (istringEqual(surface.surfaceType(), "Floor")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Ground")) {
        exteriorFloorArea += surface.grossArea();
      } else {
        interiorFloorArea += surface.grossArea();
      }
    } else if (istringEqual(surface.surfaceType(), "Wall")) {
      if (istringEqual(surface.outsideBoundaryCondition(), "Outdoors")) {
        exteriorWallArea += surface.grossArea();
      } else {
        interiorWallArea += surface.grossArea();
      }
    }
  }

  EXPECT_NEAR(exteriorFloorArea, 825.8048, 0.01);
  //EXPECT_NEAR(interiorFloorArea, 412.9019, 0.01);
  EXPECT_NEAR(exteriorRoofArea, 825.8048, 0.01);
  //EXPECT_NEAR(interiorRoofArea, 412.9019, 0.01);
  EXPECT_EQ(numRoofSurfaces, 9);

  //m.save("intersect3.osm", true);
  //openstudio::path outpath = resourcesPath() / toPath("model/Space_intersectSurfaces_degenerate3_after_intersect.osm");
  //m.save(outpath, true);
}

// Digital Alchemy
#ifdef WIN32
/// <summary>
/// Illustrates a fix for surface intersection getting stuck in a loop
/// First of all we need to remove surfaces that overlap within the same space
/// Second of all we use a different removeSpikes method that shrinks and expands the polygon
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
TEST_F(ModelFixture, RemoveSpikesAndOverlaps_TZ46_TZ47) {
  Model model;
  openstudio::path path = resourcesPath() / toPath("/model/RemoveSpikesAndOverlaps_TZ46_TZ47");

#  pragma region SPACE 1(TZ46 - 81)
  Space space(model);
  ASSERT_TRUE(space.name());
  EXPECT_EQ("Space 1", space.name().get());  //TZ46-81

  Point3dVector points;
  points.push_back(Point3d(0.787401574803132, 3.14960629921254, 3.84078906495842e-15));
  points.push_back(Point3d(2.75590551181098, 1.18110236220469, 7.70979364755774e-15));
  points.push_back(Point3d(2.00599925009369, 0.356205474315723, 3.76899861514623e-15));
  points.push_back(Point3d(0, 2.36220472440939, 0));

  Surface surface(points, model);
  surface.setParent(space);

  points.clear();
  points.push_back(Point3d(1.37795275590549, 0.984251968503903, 3.51739188744401));
  points.push_back(Point3d(5.31496062992121, 4.92125984251968, 3.51739188744401));
  points.push_back(Point3d(3.93700787401569, 6.2992125984252, 3.51739188744401));
  points.push_back(Point3d(0, 2.36220472440939, 3.51739188744401));

  Surface surface2(points, model);
  surface2.setParent(space);

  points.clear();
  points.push_back(Point3d(5.11811023622049, 2.75590551181099, 3.51739188744401));
  points.push_back(Point3d(5.11811023622049, 2.75590551181099, 0));
  points.push_back(Point3d(6.2992125984252, 3.93700787401571, 0));
  points.push_back(Point3d(6.2992125984252, 3.93700787401571, 3.51739188744401));

  Surface surface3(points, model);
  surface3.setParent(space);

  points.clear();
  points.push_back(Point3d(2.36220472440939, 0, 3.51739188744401));
  points.push_back(Point3d(2.36220472440939, 0, 0));
  points.push_back(Point3d(5.11811023622049, 2.75590551181099, 0));
  points.push_back(Point3d(5.11811023622049, 2.75590551181099, 3.51739188744401));

  Surface surface4(points, model);
  surface4.setParent(space);

  points.clear();
  points.push_back(Point3d(0, 2.36220472440939, 3.51739188744401));
  points.push_back(Point3d(0, 2.36220472440939, 0));
  points.push_back(Point3d(2.36220472440939, 0, 0));
  points.push_back(Point3d(2.36220472440939, 0, 3.51739188744401));

  Surface surface5(points, model);
  surface5.setParent(space);

  points.clear();
  points.push_back(Point3d(3.93700787401569, 6.2992125984252, 3.51739188744401));
  points.push_back(Point3d(3.93700787401569, 6.2992125984252, 0));
  points.push_back(Point3d(0, 2.36220472440939, 0));
  points.push_back(Point3d(0, 2.36220472440939, 3.51739188744401));

  Surface surface6(points, model);
  surface6.setParent(space);

  points.clear();
  points.push_back(Point3d(6.2992125984252, 3.93700787401571, 3.51739188744401));
  points.push_back(Point3d(6.2992125984252, 3.93700787401571, 0));
  points.push_back(Point3d(3.93700787401569, 6.2992125984252, 0));
  points.push_back(Point3d(3.93700787401569, 6.2992125984252, 3.51739188744401));

  Surface surface7(points, model);
  surface7.setParent(space);

  points.clear();
  points.push_back(Point3d(2.3622047244094, -1.44382283906452e-15, 3.51739188744401));
  points.push_back(Point3d(6.2992125984252, 3.93700787401571, 3.51739188744401));
  points.push_back(Point3d(5.31496062992121, 4.92125984251968, 3.51739188744401));
  points.push_back(Point3d(1.37795275590549, 0.984251968503901, 3.51739188744401));

  Surface surface8(points, model);
  surface8.setParent(space);

  points.clear();
  points.push_back(Point3d(3.93700787401569, 6.2992125984252, 0));
  points.push_back(Point3d(6.2992125984252, 3.93700787401571, 0));
  points.push_back(Point3d(2.3622047244094, -2.88764567812905e-15, 0));
  points.push_back(Point3d(2.00599925009369, 0.356205474315719, 4.33146851719357e-15));
  points.push_back(Point3d(2.75590551181099, 1.18110236220469, 7.21911419532262e-15));
  points.push_back(Point3d(0.787401574803138, 3.14960629921254, 4.33146851719357e-15));

  Surface surface9(points, model);
  surface9.setParent(space);

#  pragma endregion

#  pragma region SPACE 2(TZ47 - 91)

  Space space2(model);
  ASSERT_TRUE(space2.name());
  EXPECT_EQ("Space 2", space2.name().get());  //TZ47-91

  points.clear();
  points.push_back(Point3d(7.0866141732284, 9.44881889763781, 0));
  points.push_back(Point3d(8.16034359341449, 8.55404438081602, -1.44382283906452e-15));
  points.push_back(Point3d(5.90551181102359, 6.2992125984252, -1.44382283906452e-15));
  points.push_back(Point3d(4.92125984251972, 7.28346456692913, -7.21911419532262e-16));

  Surface surface10(points, model);
  surface10.setParent(space2);

  points.clear();
  points.push_back(Point3d(1.96850393700789, -2.88764567812905e-15, 6.095999804928));
  points.push_back(Point3d(9.44881889763779, 7.4803149606299, 6.095999804928));
  points.push_back(Point3d(7.0866141732284, 9.44881889763781, 6.095999804928));
  points.push_back(Point3d(-5.77529135625809e-15, 2.3622047244094, 6.095999804928));

  Surface surface11(points, model);
  surface11.setParent(space2);

  points.clear();
  points.push_back(Point3d(7.0866141732284, 9.44881889763781, 6.095999804928));
  points.push_back(Point3d(7.0866141732284, 9.44881889763781, 0));
  points.push_back(Point3d(-5.77529135625809e-15, 2.3622047244094, 0));
  points.push_back(Point3d(-5.77529135625809e-15, 2.3622047244094, 6.095999804928));

  Surface surface12(points, model);
  surface12.setParent(space2);

  points.clear();
  points.push_back(Point3d(9.44881889763779, 7.4803149606299, 6.095999804928));
  points.push_back(Point3d(9.44881889763779, 7.4803149606299, 0));
  points.push_back(Point3d(7.0866141732284, 9.44881889763781, 0));
  points.push_back(Point3d(7.0866141732284, 9.44881889763781, 6.095999804928));

  Surface surface13(points, model);
  surface13.setParent(space2);

  points.clear();
  points.push_back(Point3d(1.96850393700789, -2.88764567812905e-15, 6.095999804928));
  points.push_back(Point3d(1.96850393700789, -2.88764567812905e-15, 0));
  points.push_back(Point3d(9.44881889763779, 7.4803149606299, 0));
  points.push_back(Point3d(9.44881889763779, 7.4803149606299, 6.095999804928));

  Surface surface14(points, model);
  surface14.setParent(space2);

  points.clear();
  points.push_back(Point3d(-5.77529135625809e-15, 2.3622047244094, 6.095999804928));
  points.push_back(Point3d(-5.77529135625809e-15, 2.3622047244094, 0));
  points.push_back(Point3d(1.96850393700789, -2.88764567812905e-15, 0));
  points.push_back(Point3d(1.96850393700789, -2.88764567812905e-15, 6.095999804928));

  Surface surface15(points, model);
  surface15.setParent(space2);

  points.clear();
  points.push_back(Point3d(8.16034359341449, 8.55404438081602, -1.44382283906452e-15));
  points.push_back(Point3d(9.44881889763779, 7.4803149606299, 0));
  points.push_back(Point3d(1.96850393700789, -2.88764567812905e-15, 0));
  points.push_back(Point3d(-5.77529135625809e-15, 2.3622047244094, 0));
  points.push_back(Point3d(4.92125984251972, 7.28346456692913, -7.21911419532262e-16));
  points.push_back(Point3d(5.90551181102359, 6.2992125984252, -1.44382283906452e-15));

  Surface surface16(points, model);
  surface16.setParent(space2);

#  pragma endregion

  EXPECT_EQ(static_cast<unsigned>(2), model.getModelObjects<Space>().size());
  EXPECT_EQ(static_cast<unsigned>(16), model.getModelObjects<Surface>().size());

  EXPECT_TRUE(space.setTransformation(Transformation::translation(Vector3d(-34.6329065993854, 2.80342559613037, -9.61339169237201))
                                      * Transformation::rotation(Vector3d(0, 0, 1), -openstudio::degToRad(24.5))));
  EXPECT_DOUBLE_EQ(-34.6329065993854, space.xOrigin());
  EXPECT_DOUBLE_EQ(2.80342559613037, space.yOrigin());
  EXPECT_DOUBLE_EQ(-9.61339169237201, space.zOrigin());
  EXPECT_DOUBLE_EQ(24.5, space.directionofRelativeNorth());

  EXPECT_TRUE(space2.setTransformation(Transformation::translation(Vector3d(-40.4694197176487, -1.45922839997927, -6.095999804928))
                                       * Transformation::rotation(Vector3d(0, 0, 1), -openstudio::degToRad(24.5))));
  EXPECT_DOUBLE_EQ(-40.4694197176487, space2.xOrigin());
  EXPECT_DOUBLE_EQ(-1.45922839997927, space2.yOrigin());
  EXPECT_DOUBLE_EQ(-6.095999804928, space2.zOrigin());
  EXPECT_DOUBLE_EQ(24.5, space2.directionofRelativeNorth());

  SpaceVector spaces = model.getModelObjects<Space>();
  SpaceVector blacklist;

  int nSurfaces = 0;
  for (const auto& space : spaces) {
    nSurfaces += space.surfaces().size();
  }

  ASSERT_EQ(nSurfaces, 16);

  intersectSurfaces(spaces);

  nSurfaces = 0;
  for (const auto& space : spaces) {
    nSurfaces += space.surfaces().size();
  }

  ASSERT_EQ(nSurfaces, 17);

  matchSurfaces(spaces);

  //openstudio::path outPath = path;
  //outPath.replace_extension(openstudio::toPath("_finished.osm"));
  //model.save(outPath);
}

// Tests how concave surfaces are handled
TEST_F(ModelFixture, Surface_Intersect_ConcaveSurfaces) {

  Model model;
  Space sp1(model);

  double z = 1;

  Point3dVector top;
  top.push_back(Point3d(3, 0, z));
  top.push_back(Point3d(3, 5, z));
  top.push_back(Point3d(7, 5, z));
  top.push_back(Point3d(7, 0, z));
  auto normTop = getOutwardNormal(top);
  ASSERT_NEAR(normTop->z(), -1, 0.01);
  Surface s1(top, model);
  s1.setParent(sp1);

  Space sp2(model);
  Point3dVector bottom;
  bottom.push_back(Point3d(0, 0, z));
  bottom.push_back(Point3d(10, 0, z));
  bottom.push_back(Point3d(10, 10, z));
  bottom.push_back(Point3d(0, 10, z));
  auto normBottom = getOutwardNormal(bottom);
  ASSERT_NEAR(normBottom->z(), 1, 0.01);
  Surface s2(bottom, model);
  s2.setParent(sp2);

  SpaceVector spaces;
  spaces.push_back(sp1);
  spaces.push_back(sp2);
  intersectSurfaces(spaces);
  matchSurfaces(spaces);

  ASSERT_EQ(1, sp1.surfaces().size());
  ASSERT_EQ(4, sp1.surfaces().front().vertices().size());

  ASSERT_EQ(2, sp2.surfaces().size());
  ASSERT_EQ(4, sp2.surfaces().front().vertices().size());
  ASSERT_EQ(8, sp2.surfaces().back().vertices().size());
}

#  ifdef SURFACESHATTERING
// Skipping this one because this is outside of the current scope.
// To coreect this intersection and matching needs to allow holes and then
// decompose polygons with holes as th elast step of the provess. Often as
// is shown here the holes are removed by the intersection process anyway
/// <summary>
/// Polygon decomposition is order dependent. IN this case if the first intersect makes a hole
/// we triangulate, if it doesn't then triangulation is not needed
/// Two space lists 1,3,2 and 4,5,6 where 1 and 4, 2 and 5, and 3 and 6 are the same (but shifted)
/// so intersecting 1,3,3 should give the same result as 4,5,6 if order doesn't matter.
/// In order to prove this we have to not order by area so an additional argument has been added to
/// intersect(spaces) to this affect
///
/// Issue 3424
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
TEST_F(ModelFixture, ShatteredModel_Existing_3424) {

  Model model;
  BuildingStory bottom(model);
  BuildingStory top(model);

  Point3dVector points1;
  points1.push_back(Point3d(25.908, 24.384, 0));
  points1.push_back(Point3d(25.908, -22.86, 0));
  points1.push_back(Point3d(-38.1, -22.86, 0));
  points1.push_back(Point3d(-38.1, 24.384, 0));
  boost::optional<Space> space1 = Space::fromFloorPrint(points1, 2.4384, model);
  ASSERT_TRUE(space1);
  space1->setBuildingStory(bottom);
  double a1 = getArea(points1).value();

  Point3dVector points2;
  points2.push_back(Point3d(9.144, 10.668, 2.4384));
  points2.push_back(Point3d(9.144, -9.144, 2.4384));
  points2.push_back(Point3d(-19.812, -9.144, 2.4384));
  points2.push_back(Point3d(-19.812, 10.668, 2.4384));
  boost::optional<Space> space2 = Space::fromFloorPrint(points2, 2.4384, model);
  ASSERT_TRUE(space2);
  space2->setBuildingStory(top);
  double a2 = getArea(points2).value();

  Point3dVector points3;
  points3.push_back(Point3d(9.144, 24.384, 2.4384));
  points3.push_back(Point3d(9.144, 10.668, 2.4384));
  points3.push_back(Point3d(-38.1, 10.668, 2.4384));
  points3.push_back(Point3d(-38.1, 24.384, 2.4384));
  boost::optional<Space> space3 = Space::fromFloorPrint(points3, 2.4384, model);
  ASSERT_TRUE(space3);
  space3->setBuildingStory(top);
  double a3 = getArea(points3).value();

  Point3dVector points4;
  points4.push_back(Point3d(25.908, 24.384, 0));
  points4.push_back(Point3d(25.908, -22.86, 0));
  points4.push_back(Point3d(-38.1, -22.86, 0));
  points4.push_back(Point3d(-38.1, 24.384, 0));
  boost::optional<Space> space4 = Space::fromFloorPrint(points4, 2.4384, model);
  ASSERT_TRUE(space4);
  space4->setBuildingStory(bottom);
  space4->setXOrigin(75);
  double a4 = getArea(points4).value();

  Point3dVector points5;
  points5.push_back(Point3d(9.144, 10.668, 2.4384));
  points5.push_back(Point3d(9.144, -9.144, 2.4384));
  points5.push_back(Point3d(-19.812, -9.144, 2.4384));
  points5.push_back(Point3d(-19.812, 10.668, 2.4384));
  boost::optional<Space> space5 = Space::fromFloorPrint(points5, 2.4384, model);
  ASSERT_TRUE(space5);
  space5->setBuildingStory(top);
  space5->setXOrigin(75);
  double a5 = getArea(points5).value();

  Point3dVector points6;
  points6.push_back(Point3d(9.144, 24.384, 2.4384));
  points6.push_back(Point3d(9.144, 10.668, 2.4384));
  points6.push_back(Point3d(-38.1, 10.668, 2.4384));
  points6.push_back(Point3d(-38.1, 24.384, 2.4384));
  boost::optional<Space> space6 = Space::fromFloorPrint(points6, 2.4384, model);
  ASSERT_TRUE(space6);
  space6->setBuildingStory(top);
  space6->setXOrigin(75);
  double a6 = getArea(points6).value();

  // Make two lists of spaces (so we can call intersect with the space sin different orders)
  std::vector<Space> spaces1;
  spaces1.push_back(*space1);
  spaces1.push_back(*space3);
  spaces1.push_back(*space2);

  std::vector<Space> spaces2;
  spaces2.push_back(*space4);
  spaces2.push_back(*space5);
  spaces2.push_back(*space6);

  //std::vector<PolygonGroup> polygonGroups1;
  //polygonGroups1.push_back(space1->ToPolygonGroup());
  //polygonGroups1.push_back(space2->ToPolygonGroup());
  //polygonGroups1.push_back(space3->ToPolygonGroup());
  //std::vector<PolygonGroup> polygonGroups2;
  //PolygonGroup& g4 = space4->ToPolygonGroup();
  //polygonGroups2.push_back(space4->ToPolygonGroup());
  //polygonGroups2.push_back(space5->ToPolygonGroup());
  //polygonGroups2.push_back(space6->ToPolygonGroup());

  // Model before intersection
  model.save(toPath("./ShatterTest00.osm"), true);
  LOG(Info, "Saved model before intersection");

  try {

    // RUn the existing code
    intersectSurfaces(spaces1);
    LOG(Info, "Completed first surface intersections");

    intersectSurfaces(spaces2);
    LOG(Info, "Completed second surface intersections");

    // Model after intersection
    model.save(toPath("./ShatterTest01.osm"), true);
    LOG(Info, "Saved model after intersection");

    ASSERT_EQ(8, space1->surfaces().size());
    ASSERT_EQ(12, space4->surfaces().size());

    std::vector<Surface> ceilingSpace1;
    std::vector<Surface> ceilingSpace4;

    for (auto surface : space1->surfaces()) {
      if (getOutwardNormal(surface.vertices())->z() == 1) ceilingSpace1.push_back(surface);
    }
    for (auto surface : space4->surfaces()) {
      if (getOutwardNormal(surface.vertices())->z() == 1) ceilingSpace4.push_back(surface);
    }

    ASSERT_EQ(3, ceilingSpace1.size());
    ASSERT_EQ(7, ceilingSpace4.size());
    LOG(Info, "Done!!!");
  } catch (Exception w) {
    LOG(Info, "Error");
  }
}

#  endif

#  ifdef EXCLUDE
TEST_F(ModelFixture, Issue_1322) {

  osversion::VersionTranslator translator;
  path path1 = toPath("./7-7 Windows Complete.osm");
  path path2 = toPath("./two_stories_pre_intersect");
  model::OptionalModel model = translator.loadModel(toPath("7-7 Windows Complete.osm"));
  EXPECT_TRUE(model);

  SpaceVector spaces = model->getModelObjects<Space>();

  intersectSurfaces(spaces);
  matchSurfaces(spaces);
}

TEST_F(ModelFixture, Issue_1683) {

  osversion::VersionTranslator translator;
  openstudio::path modelPath = resourcesPath() / toPath("model/15023_Model12.osm");
  model::OptionalModel model = translator.loadModel(modelPath);
  EXPECT_TRUE(model);

  SpaceVector spaces = model->getConcreteModelObjects<Space>();
  intersectSurfaces(spaces);
  matchSurfaces(spaces);

  model->save(toPath("./15023_Model12_after.osm"), true);
}
#  endif
TEST_F(ModelFixture, Perimeter) {

  // NOTE: Need a simple model or make one before this is checked in
  osversion::VersionTranslator translator;
  openstudio::path modelPath = resourcesPath() / toPath("model/floorplan_school.osm");
  model::OptionalModel model = translator.loadModel(modelPath);
  EXPECT_TRUE(model);

  std::vector<Point3dVector> polygons;
  std::vector<Space> spaces;
  std::vector<std::string> spaceNames;
  std::vector<Surface> surfaces;

  for (auto space : model->getModelObjects<Space>()) {
    std::string spacename = space.name().value();
    Transformation spaceTransformation = space.transformation();
    for (auto surface : space.surfaces()) {
      std::string surfname = surface.name().value();
      Point3dVector points = spaceTransformation * surface.vertices();
      auto normal = openstudio::getOutwardNormal(points);
      if (normal) {
        if (normal->z() == -1) {
          if (points[0].z() == 0) {
            polygons.push_back(points);
            spaces.push_back(space);
            spaceNames.push_back(spacename);
          }
        }
      }
    }
  }

  // The traditional method
  auto result1 = openstudio::joinAll(polygons, 0.01);
  ASSERT_EQ(1, result1.size());
  ASSERT_EQ(12, result1[0].size());

  // Using polygons
  auto result2 = openstudio::joinAllPolygons(polygons, 0.01);
  ASSERT_EQ(1, result2.size());
  ASSERT_EQ(12, result2[0].getOuterPath().size());

  // Calculate perimeter
  Polygon3d footprint = result2[0];
  double perimeter = footprint.perimeter();
  ASSERT_NEAR(perimeter, 1428.0, 0.01);

  // Compare points list to polygon
  ASSERT_TRUE(circularEqual(result1.front(), footprint.getOuterPath()));
}

// Checks the exposed perimeter calculation for each down facing surface at ground level
TEST_F(ModelFixture, ExposedPerimeter) {

  // Open a test model
  osversion::VersionTranslator translator;
  openstudio::path modelPath = resourcesPath() / toPath("model/floorplan_school.osm");
  model::OptionalModel model = translator.loadModel(modelPath);
  EXPECT_TRUE(model);

  std::vector<Point3dVector> polygons;
  std::vector<Space> spaces;
  std::vector<Surface> surfaces;

  // Iterate over spaces, get their surfaces and transform to model coordinates,
  // find the surfaces with downward facing normal at elevation of 0
  for (auto space : model->getModelObjects<Space>()) {
    Transformation spaceTransformation = space.transformation();
    std::string spacename = space.name().value();
    for (auto surface : space.surfaces()) {
      bool added = false;
      Point3dVector points = spaceTransformation * surface.vertices();
      auto normal = openstudio::getOutwardNormal(points);
      if (normal) {
        if (normal->z() < 0) {
          if (points[0].z() == 0) {
            polygons.push_back(points);
            spaces.push_back(space);
            if (!added) {
              added = true;
              surfaces.push_back(surface);
            }
          }
        }
      }
    }
  }

  ASSERT_EQ(spaces.size(), 20);

  // Join all those polygons into one
  auto result2 = openstudio::joinAllPolygons(polygons, 0.01);
  ASSERT_EQ(1, result2.size());
  ASSERT_EQ(12, result2[0].getOuterPath().size());
  Polygon3d footprint = result2.front();

  for (auto surface : surfaces) {
    std::string surfname = surface.name().value();
    double exposedPerimeter = 0;
    int nOverlaps = 0;
    boost::optional<Space> space = surface.space();
    if (!space) continue;
    std::string spacename = space->name().value();
    Transformation spaceTransformation = space->transformation();
    Point3dVector points = spaceTransformation * surface.vertices();

    // Find points/edges on the perimeter that points on the surface are coincident with
    for (size_t i = 0; i < points.size(); i++) {
      Point3d p1 = points[i];
      Point3d p2 = points[(i + 1) % points.size()];
      Point3dVector line;
      line.push_back(p1);
      line.push_back(p2);
      Point3dVectorVector overlaps = footprint.overlap(line);
      for (auto overlap : overlaps) {
        exposedPerimeter += openstudio::getDistance(overlap[0], overlap[1]);
        nOverlaps++;
      }
    }

    // Get the space perimeter from the space and compare
    auto spacePerimeter = space->exposedPerimeter(footprint);
    ASSERT_EQ(exposedPerimeter, spacePerimeter);

    if (spacename == "Artroom 103") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 60, 0.01);
    } else if (spacename == "Bathroom 115") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 32, 0.01);
    } else if (spacename == "Classroom 101" || spacename == "Classroom 105" || spacename == "Classroom 109" || spacename == "Classroom 111") {
      ASSERT_EQ(nOverlaps, 2);
      ASSERT_NEAR(exposedPerimeter, 66, 0.01);
    } else if (spacename == "Classroom 102" || spacename == "Lobby 113") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 62, 0.01);
    } else if (spacename == "Classroom 106" || spacename == "Classroom 108" || spacename == "Classroom 112") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 122, 0.01);
    } else if (spacename == "Gym 118") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 54, 0.01);
    } else if (spacename == "Kitchen 119") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 26, 0.01);
    } else if (spacename == "Mechanical 114") {
      ASSERT_EQ(nOverlaps, 0);
      ASSERT_NEAR(exposedPerimeter, 0, 0.01);
    } else if (spacename == "Media Center 116") {
      ASSERT_EQ(nOverlaps, 3);
      ASSERT_NEAR(exposedPerimeter, 198, 0.01);
    } else if (spacename == "Offices 117") {
      ASSERT_EQ(nOverlaps, 2);
      ASSERT_NEAR(exposedPerimeter, 138, 0.01);
    } else if (spacename == "Corridor 104" || spacename == "Corridor 110") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 10, 0.01);
    } else if (spacename == "Corridor 107") {
      ASSERT_EQ(nOverlaps, 1);
      ASSERT_NEAR(exposedPerimeter, 28, 0.01);
    } else if (spacename == "Cafeteria 120") {
      ASSERT_EQ(nOverlaps, 2);
      ASSERT_NEAR(exposedPerimeter, 118, 0.01);
    } else {
      // Makes sure we've checked all 20 spaces and got the names right
      ASSERT_FALSE(true);
    }
  }
}

TEST_F(ModelFixture, Issue_2560) {

  Model model;
  BuildingStory bottom(model);
  BuildingStory top(model);

  Point3dVector bottomFace;
  bottomFace.push_back(Point3d(4, 2, 4));
  bottomFace.push_back(Point3d(4, 6, 4));
  bottomFace.push_back(Point3d(18, 6, 4));
  bottomFace.push_back(Point3d(18, 2, 4));
  OptionalSpace sp1 = Space::fromFloorPrint(bottomFace, 4, model);
  ASSERT_TRUE(sp1);
  sp1->setBuildingStory(top);
  ThermalZone tz1(model);
  sp1->setThermalZone(tz1);

  bottomFace.clear();
  bottomFace.push_back(Point3d(0, 0, 0));
  bottomFace.push_back(Point3d(0, 3, 0));
  bottomFace.push_back(Point3d(20, 3, 0));
  bottomFace.push_back(Point3d(20, 0, 0));
  OptionalSpace sp2 = Space::fromFloorPrint(bottomFace, 4, model);
  ASSERT_TRUE(sp2);
  sp2->setBuildingStory(bottom);
  ThermalZone tz2(model);
  sp2->setThermalZone(tz2);

  bottomFace.clear();
  bottomFace.push_back(Point3d(0, 3, 0));
  bottomFace.push_back(Point3d(0, 5, 0));
  bottomFace.push_back(Point3d(20, 5, 0));
  bottomFace.push_back(Point3d(20, 3, 0));
  OptionalSpace sp3 = Space::fromFloorPrint(bottomFace, 4, model);
  ASSERT_TRUE(sp3);
  sp3->setBuildingStory(bottom);
  ThermalZone tz3(model);
  sp3->setThermalZone(tz3);

  bottomFace.clear();
  bottomFace.push_back(Point3d(0, 5, 0));
  bottomFace.push_back(Point3d(0, 8, 0));
  bottomFace.push_back(Point3d(20, 8, 0));
  bottomFace.push_back(Point3d(20, 5, 0));
  OptionalSpace sp4 = Space::fromFloorPrint(bottomFace, 4, model);
  ASSERT_TRUE(sp4);
  sp4->setBuildingStory(bottom);
  ThermalZone tz4(model);
  sp4->setThermalZone(tz4);

  model.save(toPath("./2560_before.osm"), true);

  SpaceVector spaces = model.getConcreteModelObjects<Space>();
  intersectSurfaces(spaces);
  matchSurfaces(spaces);
  model.save(toPath("./2560_after.osm"), true);

  for (auto space : spaces) {
    std::string spaceName = space.name().value();
    auto surfaces = space.surfaces();
    for (auto surface : surfaces) {
      std::string surfaceName = surface.name().value();
      auto vertices = surface.vertices();
    }
  }
}


#endif
