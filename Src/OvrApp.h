#ifndef OVRAPP_H
#define OVRAPP_H

#include "App.h"
#include "GuiSys.h"
#include "SceneView.h"
#include "SoundEffectContext.h"

namespace OVR {
class ovrLocale;
}

class MenuItem {
 public:
  bool Selectable = false;
  bool Selected = false;
  int PosX = 100;
  int PosY = 100;

 public:
  virtual ~MenuItem() {}
  virtual void DrawText(float transparency) {}
  virtual void DrawTexture(float transparency) {}
};

class MenuButton : public MenuItem {
 public:
  MenuButton(GLuint iconId, std::string text, int posX, int posY,
             void (*pressFunction)(MenuButton *item), void (*leftFunction)(MenuButton *item),
             void (*rightFunction)(MenuButton *item)) {
    PosX = posX;
    PosY = posY;
    IconId = iconId;
    Text = text;
    PressFunction = pressFunction;
    LeftFunction = leftFunction;
    RightFunction = rightFunction;
  }

  virtual ~MenuButton() {}

  GLuint IconId;

  std::string Text;

  void (*PressFunction)(MenuButton *item);

  void (*LeftFunction)(MenuButton *item);

  void (*RightFunction)(MenuButton *item);

  virtual void DrawText(float transparency) override;

  virtual void DrawTexture(float transparency) override;
};

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
