#ifndef OVRAPP_H
#define OVRAPP_H

#include "KingInclude.h"

namespace OVR {
class ovrLocale;
}

void SaveSettings();
void LoadSettings();
void ScanDirectory();
void CreateScreen();

class OvrApp : public OVR::VrAppInterface {
 public:
  OvrApp();

  virtual ~OvrApp();

  virtual void SetUpMenu();

  virtual void Configure(OVR::ovrSettings &settings);

  virtual void EnteredVrMode(const OVR::ovrIntentType intentType, const char *intentFromPackage,
                             const char *intentJSON, const char *intentURI);

  virtual void LeavingVrMode();

  virtual OVR::ovrFrameResult Frame(const OVR::ovrFrameInput &vrFrame);

  class OVR::ovrLocale &GetLocale() {
    return *Locale;
  }

 private:
  OVR::ovrSoundEffectContext *SoundEffectContext;
  OVR::OvrGuiSys::SoundEffectPlayer *SoundEffectPlayer;
  OVR::OvrGuiSys *GuiSys;
  OVR::ovrLocale *Locale;

  OVR::ModelFile *SceneModel;
  OVR::OvrSceneView Scene;
};

#endif  // OVRAPP_H
