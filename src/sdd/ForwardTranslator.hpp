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

#ifndef SDD_FORWARDTRANSLATOR_HPP
#define SDD_FORWARDTRANSLATOR_HPP

#include "SDDAPI.hpp"

#include "../utilities/core/Path.hpp"
#include "../utilities/core/Optional.hpp"
#include "../utilities/core/Logger.hpp"
#include "../utilities/core/StringStreamLogSink.hpp"

#include "../model/ModelObject.hpp"

#include <map>

namespace pugi {
  class xml_node;
  class xml_document;
}

namespace openstudio {

  class ProgressBar;
  class Transformation;

namespace model {
  class Model;
  class ModelObject;
  class Material;
  class ConstructionBase;
  class Building;
  class BuildingStory;
  class ThermalZone;
  class Space;
  class Surface;
  class SubSurface;
  class ShadingSurface;
  class AirLoopHVAC;
  class FanConstantVolume;
  class CoilCoolingDXSingleSpeed;
  class CoilHeatingGas;
  class AirLoopHVACOutdoorAirSystem;
}

namespace sdd {

  class SDD_API ForwardTranslator {
  public:

    ForwardTranslator();

    virtual ~ForwardTranslator();

    bool modelToSDD(const openstudio::model::Model& model, const openstudio::path& path, ProgressBar* progressBar=nullptr);

    /** Get warning messages generated by the last translation.
      * Warnings are issues encountered during translation that are important, but not fatal to producing a valid SDD for compliance analysis*/
    std::vector<LogMessage> warnings() const;

    /** Get error messages generated by the last translation.
      * Errors are issues encountered during translation that are fatal to producing a valid SDD for compliance analysis*/
    std::vector<LogMessage> errors() const;

  private:

    std::string escapeName(std::string name);

    // listed in translation order
    // Any of these may throw if they encounter something so terrible they cannot continue,
    // and the caller (modelToSDD) should catch the exception and report the log up to that point.

    // Use LOG(Error ... or LOG(Warn ... to produce log messages.
    // If there is an error that prevents the translation from being able to continue then use LOG_AND_THROW.
    // Prefer LOG(Error over LOG_AND_THROW if possible.
    // Use OS_ASSERT to catch logic errors in the translator implementation.  Do not use OS_ASSERT on bad input, use LOG( instead.

    bool translateModel(const openstudio::model::Model& model, pugi::xml_document& document);
    boost::optional<pugi::xml_node> translateMaterial(const openstudio::model::Material& material, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateConstructionBase(const openstudio::model::ConstructionBase& constructionBase, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateDoorConstruction(const openstudio::model::ConstructionBase& constructionBase, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateFenestrationConstruction(const openstudio::model::ConstructionBase& constructionBase, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateBuilding(const openstudio::model::Building& building, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateBuildingStory(const openstudio::model::BuildingStory& buildingStory, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateSpace(const openstudio::model::Space& space, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateSurface(const openstudio::model::Surface& surface, const openstudio::Transformation& transformation, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateSubSurface(const openstudio::model::SubSurface& subSurface, const openstudio::Transformation& transformation, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateShadingSurface(const openstudio::model::ShadingSurface& shadingSurface, const openstudio::Transformation& transformation, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateThermalZone(const openstudio::model::ThermalZone& thermalZone, pugi::xml_node& root);
    boost::optional<pugi::xml_node> translateAirLoopHVAC(const openstudio::model::AirLoopHVAC& airLoop, pugi::xml_node& root);

    boost::optional<pugi::xml_node> translateFanConstantVolume(const openstudio::model::FanConstantVolume& fan, pugi::xml_node& airSegElement);
    boost::optional<pugi::xml_node> translateCoilCoolingDXSingleSpeed(const openstudio::model::CoilCoolingDXSingleSpeed& coil, pugi::xml_node& airSegElement);
    boost::optional<pugi::xml_node> translateCoilHeatingGas(const openstudio::model::CoilHeatingGas& coil, pugi::xml_node& airSegElement);
    boost::optional<pugi::xml_node> translateAirLoopHVACOutdoorAirSystem(const openstudio::model::AirLoopHVACOutdoorAirSystem& oasys, pugi::xml_node& airSegElement);

    std::map<openstudio::Handle, pugi::xml_node> m_translatedObjects;

    // Log untranslated objects as an error,
    // unless the type is in the m_ignoreTypes or m_ignoreObjects member.
    void logUntranslatedObjects(const model::Model& model);

    // Use this member to ignore an entire class of ModelObject.
    std::vector<IddObjectType> m_ignoreTypes;

    // Use this member to ignore a specific ModelObject, but not all of that type.
    // For example you might want to ignore FanConstantSpeed inside of terminals, but
    // not ignore all FanConstantSpeed objects.
    // There will be no warning that the type is ignored.
    std::vector<Handle> m_ignoreObjects;

    StringStreamLogSink m_logSink;

    ProgressBar* m_progressBar;

    bool m_autoHardSize;
    bool m_autoEfficiency;

    REGISTER_LOGGER("openstudio.sdd.ForwardTranslator");
  };

} // sdd
} // openstudio

#endif // SDD_FORWARDTRANSLATOR_HPP
