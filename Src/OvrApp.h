#ifndef OVRAPP_H
#define OVRAPP_H

#include "App.h"
#include "FontMaster.h"
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
  bool Visible = true;
  int PosX = 100, PosY = 100;

 public:
  virtual ~MenuItem() {}
  virtual void DrawText(float offsetX, float transparency) {}
  virtual void DrawTexture(float offsetX, float transparency) {}
};

class MenuLabel : public MenuItem {
 public:
  MenuLabel(FontManager::RenderFont font, std::string text, int posX, int posY, int width,
            int height, ovrVector4f color) {
    Font = font;
    Text = text;
    // center text
    int textWidth = FontManager::GetWidth(font, text);
    PosX = posX + width / 2 - textWidth / 2;
    PosY = posY + height / 2 - font.FontSize;
    Color = color;
  }

  virtual ~MenuLabel() {}

  ovrVector4f Color;

  FontManager::RenderFont Font;

  std::string Text;

  virtual void DrawText(float offsetX, float transparency) override;
};

class MenuImage : public MenuItem {
 public:
  MenuImage(GLuint imageId, int posX, int posY, int width, int height, ovrVector4f color) {
    ImageId = imageId;
    PosX = posX;
    PosY = posY;
    Width = width;
    Height = height;
    Color = color;
  }

  ovrVector4f Color;

  GLuint ImageId;

  int Width, Height;

  virtual ~MenuImage() {}

  virtual void DrawTexture(float offsetX, float transparency) override;
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

  virtual void DrawText(float offsetX, float transparency) override;

  virtual void DrawTexture(float offsetX, float transparency) override;
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
