#include <OVR_FileSys.h>
#include <OVR_Input.h>
#include <VrApi_Input.h>
#include <VrApi_Types.h>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <VRMenu.h>
#include <dirent.h>
#include <algorithm>
#include "GuiSys.h"
#include "OVR_Locale.h"
#include "OvrApp.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gli/convert.hpp>    // convert a texture from a format to another
#include <gli/duplicate.hpp>  // duplicate the data of a texture, allocating a new texture storage
#include <gli/dx.hpp>         // facilitate the use of GLI with Direct3D API
#include <gli/format.hpp>     // list of the supported formats
#include <gli/generate_mipmaps.hpp>  // generating the mipmaps of a texture
#include <gli/gl.hpp>                // translate GLI enums to OpenGL enums
#include <gli/gli.hpp>
#include <gli/image.hpp>  // use images, a representation of a single texture level.
#include <gli/levels.hpp>  // compute the number of mipmaps levels necessary to create a mipmap complete texture.
#include <gli/load.hpp>                // load DDS, KTX or KMG textures from files or memory.
#include <gli/load_dds.hpp>            // load DDS textures from files or memory.
#include <gli/reduce.hpp>              // include to perform reduction operations.
#include <gli/sampler.hpp>             // enumations for texture sampling
#include <gli/sampler1d.hpp>           // class to sample 1d texture
#include <gli/sampler1d_array.hpp>     // class to sample 1d array texture
#include <gli/sampler2d.hpp>           // class to sample 2d texture
#include <gli/sampler2d_array.hpp>     // class to sample 2d array texture
#include <gli/sampler_cube.hpp>        // class to sample cube texture
#include <gli/sampler_cube_array.hpp>  // class to sample cube array texture
#include <gli/target.hpp>              // helper function to query property of a generic texture
#include <gli/texture.hpp>          // generic texture class that can represent any kind of texture
#include <gli/texture2d.hpp>        // representation of a 2d texture
#include <gli/texture2d_array.hpp>  // representation of a 2d array texture

#include <gearboy.h>

#include "DrawHelper.h"
#include "Emulator.h"
#include "FontMaster.h"
#include "LayerBuilder.h"

using namespace OVR;

#define menuWidth 640
#define menuHeight 576

#define HEADER_HEIGHT 75

#define SCROLL_DELAY 15
#define SCROLL_TIME 5

#define SCROLL_DELAY_MOVE 10
#define SCROLL_TIME_MOVE 1

#define MAX_SAVESLOTS 10
#define STR_VERSION "ver.1.0"

#define MoveSpeed 0.00390625f
#define ZoomSpeed 0.03125f
#define MIN_RADIUS 0.5f
#define MAX_RADIUS 5.5f

#define GL(func) func;

const int SAVE_FILE_VERSION = 3;

std::vector<Emulator::Rom> romFiles;

GlProgram glprog;

int listPosX = 0;
int listPosY = 0;
int scrollbarWidth = 10;
int scrollbarHeight = 400;
int listItemSize;
int menuItemSize;
int listStartY;

bool menuOpen = true;
bool romSelection = true;
bool loadedRom;

// saved variables
bool followHead;
int saveSlot;
bool allowUpDownSlashLeftRight;  // probably not necessary
bool showExitDialog = false;

int currentRomListSelection;
int menuListState;
int maxListItems = 18;

float transitionPercentage = 1.0f;
int transitionDirection;

std::string strForceDMG[] = {"Force DMG: Yes", "Force DMG: No"};
std::string strMove[] = {"Follow Head: Yes", "Follow Head: No"};
std::string strHeader = "GearboyVR";
std::string strNoSave = "--Empty Slot--";
std::string strMoveMenu = "<- Move Screen ->";
int strMoveMenuTextWidth;
int strNoSaveWidth;

int button_mapping_menu;
int button_mapping_menu_index = 3;
std::string MapButtonStr[] = {"A", "B", "X", "Y"};
int MapButtons[] = {BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y};

unsigned int lastButtonState;

const char *appDir;
const char *storageDir;

std::string saveFilePath;
std::string stateFolderPath;
std::string romFolderPath;

ovrVector4f textColor = {1.0f, 1.0f, 1.0f, 1.0f};
ovrVector4f textSelectionColor = {0.15f, 0.8f, 0.6f, 1.0f};
ovrVector4f textColorVersion = {0.8f, 0.8f, 0.8f, 0.8f};
ovrVector4f sliderColor = {0.8f, 0.8f, 0.8f, 0.8f};
ovrVector4f headerColor = textColor;

int buttonDownCount;

FontManager::RenderFont fontHeader, fontMenu, fontList, fontSlot, fontSmall;

int strVersionWidth;
const ovrJava *java;

jclass clsData;
jmethodID getVal;

int batteryLevel, batter_string_width;
std::string time_string, battery_string;

Vector4f MenuBackgroundColor{0.03f, 0.036f, 0.06f, 0.99f};
Vector4f MenuBackgroundOverlayColor{0.0f, 0.0f, 0.0f, 0.25f};

Vector4f BatteryColor{1.0f, 1.0f, 1.0f, 1.0f};
Vector4f BatteryBackgroundColor{0.15f, 0.15f, 0.15f, 1.0f};

ovrTextureSwapChain *MenuSwapChain;
GLuint MenuFrameBuffer = 0;

GLuint textureIdMenu, textureHeaderIconId, textureGbIconId, textureGbcIconId, textureSaveIconId,
    textureLoadIconId, textureWhiteId, texturePlayId, textureResumeId, textureSettingsId,
    texuterLeftRightIconId, textureUpDownIconId, textureResetIconId, textureSaveSlotIconId,
    textureLoadRomIconId, textureBackIconId, textureMoveIconId, textureDistanceIconId,
    textureResetViewIconId, textureScaleIconId, textureMappingIconId, texturePaletteIconId,
    textureButtonAIconId, textureButtonBIconId, textureFollowHeadIconId, textureDMGIconId,
    textureExitIconId;

template <typename T>
std::string to_string(T value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

class Menu {
 public:
  int CurrentSelection;

  std::vector<MenuItem *> MenuItems;

 public:
  void (*BackPress)();
};

bool isTransitioning;
int transitionDir, transitionMoveDir = 1;
float transitionState = 1;
Menu *currentMenu, *nextMenu;
Menu mainMenu, settingsMenu, moveMenu, buttonMapMenu;

MenuLabel *emptySlotLabel;

MenuButton *yawButton;
MenuButton *pitchButton;
MenuButton *rollButton;
MenuButton *scaleButton;
MenuButton *distanceButton;

void MenuLabel::DrawText(float offsetX, float transparency) {
  if (Visible) FontManager::RenderText(Font, Text, PosX, PosY, 1.0f, Color, transparency);
}

void MenuImage::DrawTexture(float offsetX, float transparency) {
  if (Visible) DrawHelper::DrawTexture(ImageId, PosX, PosY, Width, Height, Color, transparency);
}

void MenuButton::DrawText(float offsetX, float transparency) {
  if (Visible)
    FontManager::RenderText(fontMenu, Text, PosX + 30 + (Selected ? 5 : 0) + offsetX, PosY, 1.0f,
                            Selected ? textSelectionColor : textColor, transparency);
}

void MenuButton::DrawTexture(float offsetX, float transparency) {
  if (IconId > 0 && Visible)
    DrawHelper::DrawTexture(IconId, PosX + (Selected ? 5 : 0) + offsetX, PosY + 3, 26, 26,
                            Selected ? textSelectionColor : textColor, transparency);
}

#if defined(OVR_OS_ANDROID)

extern "C" {

jlong Java_com_a_gear_boy_go_MainActivity_nativeSetAppInterface(JNIEnv *jni, jclass clazz,
                                                                jobject activity,
                                                                jstring fromPackageName,
                                                                jstring commandString,
                                                                jstring uriString) {
  jmethodID messageMe = jni->GetMethodID(clazz, "getInternalStorageDir", "()Ljava/lang/String;");
  jobject result = (jstring)jni->CallObjectMethod(activity, messageMe);
  storageDir = jni->GetStringUTFChars((jstring)result, NULL);

  messageMe = jni->GetMethodID(clazz, "getExternalFilesDir", "()Ljava/lang/String;");
  result = (jstring)jni->CallObjectMethod(activity, messageMe);
  appDir = jni->GetStringUTFChars((jstring)result, NULL);

  saveFilePath = appDir;
  saveFilePath.append("/settings.config");

  stateFolderPath = storageDir;
  stateFolderPath += "/Roms/GB/States/";

  romFolderPath = storageDir;
  romFolderPath += "/Roms/GB/";

  LOG("got string from java: appdir %s", appDir);
  LOG("got string from java: storageDir %s", storageDir);

  LOG("nativeSetAppInterface");
  return (new OvrApp())
      ->SetActivity(jni, clazz, activity, fromPackageName, commandString, uriString);
}

}  // extern "C"

#endif

OvrApp::OvrApp()
    : SoundEffectContext(NULL),
      SoundEffectPlayer(NULL),
      GuiSys(OvrGuiSys::Create()),
      Locale(NULL),
      SceneModel(NULL) {}

OvrApp::~OvrApp() {
  delete SoundEffectPlayer;
  SoundEffectPlayer = NULL;

  delete SoundEffectContext;
  SoundEffectContext = NULL;

  OvrGuiSys::Destroy(GuiSys);
  if (SceneModel != NULL) {
    delete SceneModel;
  }
}

/// Filename can be KTX or DDS files
GLuint Load_Texture(const void *Data, std::size_t Size) {
  LOG("Loading Texture");

  gli::texture Texture = gli::load((const char *)Data, Size);
  if (Texture.empty()) {
    LOG("Faild loading");
    return 0;
  }
  LOG("Loaded TEXTURE");

  // gli::gl GLF(gli::gl::PROFILE_ES30);
  gli::gl GL1(gli::gl::PROFILE_ES30);
  gli::gl::format const Format = GL1.translate(Texture.format(), Texture.swizzles());
  GLenum Target = GL1.translate(Texture.target());

  GLuint TextureName = 0;
  glGenTextures(1, &TextureName);
  glBindTexture(Target, TextureName);
  glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

  glm::tvec3<GLsizei> const Extent(Texture.extent());
  GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

  glTexStorage2D(Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x,
                 Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);

  for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
    for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
      for (std::size_t Level = 0; Level < Texture.levels(); ++Level) {
        GLsizei const LayerGL = static_cast<GLsizei>(Layer);
        glm::tvec3<GLsizei> Extent(Texture.extent(Level));
        Target = gli::is_target_cube(Texture.target())
                     ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
                     : Target;

        if (gli::is_compressed(Texture.format())) {
          if (Texture.target() == gli::TARGET_1D_ARRAY) LOG("TARGET_1D");
          if (Texture.target() == gli::TARGET_2D) LOG("TARGET_2D");

          glCompressedTexSubImage2D(Target, static_cast<GLint>(Level), 0, 0, Extent.x,
                                    Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
                                    Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                                    Texture.data(Layer, Face, Level));
        } else {
          glTexSubImage2D(Target, static_cast<GLint>(Level), 0, 0, Extent.x,
                          Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
                          Format.External, Format.Type, Texture.data(Layer, Face, Level));
        }
      }

  return TextureName;
}
ovrSettings *OVR_Settings;
void OvrApp::Configure(ovrSettings &settings) {
  settings.CpuLevel = 0;
  settings.GpuLevel = 0;

  settings.RenderMode = RENDERMODE_MULTIVIEW;
  settings.UseSrgbFramebuffer = true;

  OVR_Settings = &settings;
}

int UpdateBatteryLevel() {
  jint bLevel = java->Env->CallIntMethod(java->ActivityObject, getVal);
  int returnValue = (int)bLevel;
  return returnValue;
}

void SetUpScrollList() {
  listItemSize = (fontList.FontSize + 11);
  menuItemSize = (fontMenu.FontSize + 7);
  listPosX = 5;
  listPosY = HEADER_HEIGHT + 10;
  maxListItems = (menuHeight - HEADER_HEIGHT - 20) / listItemSize;
  scrollbarHeight = (menuHeight - HEADER_HEIGHT - 20);
  listStartY = listPosY + (scrollbarHeight - (maxListItems * listItemSize)) / 2;
}

void CreateScreen() {
  // menu layer
  MenuSwapChain = vrapi_CreateTextureSwapChain(VRAPI_TEXTURE_TYPE_2D, VRAPI_TEXTURE_FORMAT_8888,
                                               menuWidth, menuHeight, 1, false);

  textureIdMenu = vrapi_GetTextureSwapChainHandle(MenuSwapChain, 0);
  glBindTexture(GL_TEXTURE_2D, textureIdMenu);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, menuWidth, menuHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  if (false) {  // !glExtensions.EXT_texture_border_clamp
    LOG("Error texture border clamp not supported");
    // Just clamp to edge. However, this requires manually clearing the border
    // around the layer to clear the edge texels.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  // create hte framebuffer for the menu texture
  glGenFramebuffers(1, &MenuFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, MenuFrameBuffer);

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)textureIdMenu,
                         0);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers);  // "1" is the size of DrawBuffers

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  LOG("finished creating screens");
}

void CreateWhiteImage() {
  uint32_t white = 0xFFFFFFFF;
  glGenTextures(1, &textureWhiteId);
  glBindTexture(GL_TEXTURE_2D, textureWhiteId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void SaveSettings() {
  std::ofstream saveFile(saveFilePath, std::ios::trunc | std::ios::binary);
  saveFile.write(reinterpret_cast<const char *>(&SAVE_FILE_VERSION), sizeof(int));

  Emulator::SaveSettings(&saveFile);
  saveFile.write(reinterpret_cast<const char *>(&currentRomListSelection), sizeof(int));
  saveFile.write(reinterpret_cast<const char *>(&saveSlot), sizeof(int));
  saveFile.write(reinterpret_cast<const char *>(&LayerBuilder::screenPitch), sizeof(float));
  saveFile.write(reinterpret_cast<const char *>(&LayerBuilder::screenYaw), sizeof(float));
  saveFile.write(reinterpret_cast<const char *>(&LayerBuilder::screenRoll), sizeof(float));
  saveFile.write(reinterpret_cast<const char *>(&LayerBuilder::radiusMenuScreen), sizeof(float));
  saveFile.write(reinterpret_cast<const char *>(&LayerBuilder::screenSize), sizeof(float));
  saveFile.write(reinterpret_cast<const char *>(&followHead), sizeof(bool));
  saveFile.write(reinterpret_cast<const char *>(&button_mapping_menu_index), sizeof(int));

  saveFile.close();
  LOG("Saved Settings");
}

void LoadSettings() {
  std::ifstream loadFile(saveFilePath, std::ios::in | std::ios::binary | std::ios::ate);
  if (loadFile.is_open()) {
    loadFile.seekg(0, std::ios::beg);

    int saveFileVersion = 0;
    loadFile.read((char *)&saveFileVersion, sizeof(int));

    // only load if the save versions are compatible
    if (saveFileVersion == SAVE_FILE_VERSION) {
      Emulator::LoadSettings(&loadFile);
      loadFile.read((char *)&currentRomListSelection, sizeof(int));
      loadFile.read((char *)&saveSlot, sizeof(int));
      loadFile.read((char *)&LayerBuilder::screenPitch, sizeof(float));
      loadFile.read((char *)&LayerBuilder::screenYaw, sizeof(float));
      loadFile.read((char *)&LayerBuilder::screenRoll, sizeof(float));
      loadFile.read((char *)&LayerBuilder::radiusMenuScreen, sizeof(float));
      loadFile.read((char *)&LayerBuilder::screenSize, sizeof(float));
      loadFile.read((char *)&followHead, sizeof(bool));
      loadFile.read((char *)&button_mapping_menu_index, sizeof(int));
    }

    // TODO: reset all loaded settings
    if (loadFile.fail())
      LOG("Failed Loading Settings");
    else
      LOG("Settings Loaded");

    loadFile.close();
  }

  LOG("menu index %i", button_mapping_menu_index);
  button_mapping_menu = MapButtons[button_mapping_menu_index];
}

void ScanDirectory() {
  DIR *dir;
  struct dirent *ent;
  std::string fullPath;

  if ((dir = opendir(romFolderPath.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      fullPath = "";
      fullPath.append(romFolderPath);
      fullPath.append("/");
      fullPath.append(ent->d_name);

      LOG("storageDir: %s", romFolderPath.c_str());

      // check if it is a file
      // gb|dmg|gbc|cgb|sgb
      if (ent->d_type == 8) {
        std::string strFilename = ent->d_name;

        if (strFilename.find(".gb") != std::string::npos ||
            strFilename.find(".dmg") != std::string::npos ||
            strFilename.find(".gbc") != std::string::npos ||
            strFilename.find(".cgb") != std::string::npos ||
            strFilename.find(".sgb") != std::string::npos) {
          size_t lastIndex = strFilename.find_last_of(".");
          std::string listName = strFilename.substr(0, lastIndex);
          size_t lastIndexSave = (fullPath).find_last_of(".");
          std::string listNameSave = fullPath.substr(0, lastIndexSave);

          Emulator::Rom newRom;
          newRom.RomName = listName;
          newRom.FullPath = fullPath;
          newRom.FullPathNorm = listNameSave;
          newRom.SavePath = listNameSave + ".srm";

          // check if it is a gbc rom
          newRom.isGbc = (strFilename.find(".gbc") != std::string::npos ||
                          strFilename.find(".cgb") != std::string::npos);

          romFiles.push_back(newRom);

          LOG("found rom: %s %s %s", newRom.RomName.c_str(), newRom.FullPath.c_str(),
              newRom.SavePath.c_str());
        }
      }
    }
    closedir(dir);
    LOG("sort list");
    std::sort(romFiles.begin(), romFiles.end(), Emulator::SortByRomName);
    LOG("finished sorting list");
  } else {
    LOG("could not open folder");
  }
}

void OvrApp::EnteredVrMode(const ovrIntentType intentType, const char *intentFromPackage,
                           const char *intentJSON, const char *intentURI) {
  OVR_UNUSED(intentFromPackage);
  OVR_UNUSED(intentJSON);
  OVR_UNUSED(intentURI);

  if (intentType == INTENT_LAUNCH) {
    FontManager::Init(menuWidth, menuHeight);
    FontManager::LoadFont(&fontHeader, const_cast<char *>("/system/fonts/Roboto-Regular.ttf"), 55);
    FontManager::LoadFont(&fontMenu, const_cast<char *>("/system/fonts/Roboto-Light.ttf"), 24);
    FontManager::LoadFont(&fontList, const_cast<char *>("/system/fonts/Roboto-Light.ttf"), 20);
    FontManager::LoadFont(&fontSmall, const_cast<char *>("/system/fonts/Roboto-Light.ttf"), 16);
    FontManager::LoadFont(&fontSlot, const_cast<char *>("/system/fonts/Roboto-Light.ttf"), 26);
    FontManager::CloseFontLoader();

    // load icons
    MemBufferT<uint8_t> buffer;
    if (app->GetFileSys().ReadFile("apk:///assets/header_icon.dds", buffer))
      textureHeaderIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/gb_cartridge.dds", buffer))
      textureGbIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/gbc_cartridge.dds", buffer))
      textureGbcIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/save_icon.dds", buffer))
      textureSaveIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/load_icon.dds", buffer))
      textureLoadIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/play_icon.dds", buffer))
      texturePlayId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/resume_icon.dds", buffer))
      textureResumeId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/settings_icon.dds", buffer))
      textureSettingsId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));

    if (app->GetFileSys().ReadFile("apk:///assets/leftright_icon.dds", buffer))
      texuterLeftRightIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/updown_icon.dds", buffer))
      textureUpDownIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));

    if (app->GetFileSys().ReadFile("apk:///assets/reset_icon.dds", buffer))
      textureResetIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/save_slot_icon.dds", buffer))
      textureSaveSlotIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/rom_list_icon.dds", buffer))
      textureLoadRomIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));

    if (app->GetFileSys().ReadFile("apk:///assets/move_icon.dds", buffer))
      textureMoveIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/back_icon.dds", buffer))
      textureBackIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/distance_icon.dds", buffer))
      textureDistanceIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/reset_view_icon.dds", buffer))
      textureResetViewIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/scale_icon.dds", buffer))
      textureScaleIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/mapping_icon.dds", buffer))
      textureMappingIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));

    if (app->GetFileSys().ReadFile("apk:///assets/palette_icon.dds", buffer))
      texturePaletteIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/button_a_icon.dds", buffer))
      textureButtonAIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/button_b_icon.dds", buffer))
      textureButtonBIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/follow_head_icon.dds", buffer))
      textureFollowHeadIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/force_dmg_icon.dds", buffer))
      textureDMGIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));
    if (app->GetFileSys().ReadFile("apk:///assets/exit_icon.dds", buffer))
      textureExitIconId = Load_Texture(buffer, static_cast<int>(buffer.GetSize()));

    CreateWhiteImage();

    // TODO move me to the launch of the app
    DrawHelper::Init(menuWidth, menuHeight);

    LoadSettings();

    Emulator::Init(stateFolderPath);

    ScanDirectory();

    SetUpScrollList();

    SetUpMenu();

    CreateScreen();

    java = app->GetJava();
    SoundEffectContext = new ovrSoundEffectContext(*java->Env, java->ActivityObject);
    SoundEffectContext->Initialize(&app->GetFileSys());
    SoundEffectPlayer = new OvrGuiSys::ovrDummySoundEffectPlayer();

    Locale = ovrLocale::Create(*java->Env, java->ActivityObject, "default");

    clsData = java->Env->GetObjectClass(java->ActivityObject);
    getVal = java->Env->GetMethodID(clsData, "getInt", "()I");

    String fontName;
    GetLocale().GetString("@string/font_name", "efigs.fnt", fontName);
    GuiSys->Init(this->app, *SoundEffectPlayer, fontName.ToCStr(), &app->GetDebugLines());

    LOG("Create the Menu");
    // CreateMenu();

    MaterialParms materialParms;
    materialParms.UseSrgbTextureFormats = false;

    const char *sceneUri = "apk:///assets/box.ovrscene";

    SceneModel =
        LoadModelFile(app->GetFileSys(), sceneUri, Scene.GetDefaultGLPrograms(), materialParms);

    if (SceneModel != NULL) {
    } else {
      LOG("OvrApp::EnteredVrMode Failed to load %s", sceneUri);
    }
  } else if (intentType == INTENT_NEW) {
  }
}

void OvrApp::LeavingVrMode() {}

void GetTimeString(std::string &timeString) {
  struct timespec res;
  clock_gettime(CLOCK_REALTIME, &res);
  time_t t = res.tv_sec;  // just in case types aren't the same
  tm tmv;
  localtime_r(&t, &tmv);  // populate tmv with local time info

  timeString.clear();
  if (tmv.tm_hour < 10) timeString.append("0");
  timeString.append(to_string(tmv.tm_hour));
  timeString.append(":");
  if (tmv.tm_min < 10) timeString.append("0");
  timeString.append(to_string(tmv.tm_min));
}

void GetBattryString(std::string &batteryString) {
  batteryLevel = UpdateBatteryLevel();
  batteryString.clear();
  batteryString.append(to_string(batteryLevel));
  batteryString.append("%");

  batter_string_width = FontManager::GetWidth(fontSmall, batteryString);
}

bool ButtonPressed(const ovrFrameInput &vrFrame, int button) {
  return vrFrame.Input.buttonState & button &&
         (!(lastButtonState & button) || buttonDownCount > SCROLL_DELAY);
}

void UpdateRomSelection(const ovrFrameInput &vrFrame) {
  if (vrFrame.Input.buttonState & BUTTON_LSTICK_UP || vrFrame.Input.buttonState & BUTTON_DPAD_UP) {
    if ((!(lastButtonState & BUTTON_LSTICK_UP) && !(lastButtonState & BUTTON_DPAD_UP)) ||
        buttonDownCount > SCROLL_DELAY) {
      currentRomListSelection--;
      buttonDownCount -= SCROLL_TIME;
    }
    // @TODO use time instead
    buttonDownCount++;
  } else if (vrFrame.Input.buttonState & BUTTON_LSTICK_DOWN ||
             vrFrame.Input.buttonState & BUTTON_DPAD_DOWN) {
    if ((!(lastButtonState & BUTTON_LSTICK_DOWN) && !(lastButtonState & BUTTON_DPAD_DOWN)) ||
        buttonDownCount > SCROLL_DELAY) {
      currentRomListSelection++;
      buttonDownCount -= SCROLL_TIME;
    }
    buttonDownCount++;
  } else {
    buttonDownCount = 0;
  }

  if (currentRomListSelection < 0)
    currentRomListSelection = (int)(romFiles.size() - 1);
  else if (currentRomListSelection >= romFiles.size())
    currentRomListSelection = 0;

  // scroll the menu
  if (currentRomListSelection - 2 < menuListState && menuListState > 0) {
    menuListState--;
  }
  if (currentRomListSelection + 2 >= menuListState + maxListItems &&
      menuListState + maxListItems < romFiles.size()) {
    menuListState++;
  }

  // load the selected rom
  if (romFiles.size() > 0 && vrFrame.Input.buttonState & BUTTON_A &&
      !(lastButtonState & BUTTON_A)) {
    SaveSettings();
    saveSlot = 0;
    Emulator::LoadGame(&romFiles[currentRomListSelection]);
    menuOpen = false;
    romSelection = false;
    loadedRom = true;
  }
  // go back
  if (vrFrame.Input.buttonState & BUTTON_B && !(lastButtonState & BUTTON_B) && loadedRom) {
    romSelection = false;
  }
}

void UpdateMenu(const ovrFrameInput &vrFrame) {
  if (isTransitioning) {
    transitionState -= 0.15f;
    if (transitionState < 0) {
      transitionState = 1;
      isTransitioning = false;
      currentMenu = nextMenu;
    }
  }

  // @hack: this should be done nicer
  emptySlotLabel->Visible = !Emulator::currentGame->saveStates[saveSlot].filled;

  // could be done with a single &
  if (vrFrame.Input.buttonState & BUTTON_LSTICK_UP || vrFrame.Input.buttonState & BUTTON_DPAD_UP ||
      vrFrame.Input.buttonState & BUTTON_LSTICK_DOWN ||
      vrFrame.Input.buttonState & BUTTON_DPAD_DOWN ||
      vrFrame.Input.buttonState & BUTTON_LSTICK_LEFT ||
      vrFrame.Input.buttonState & BUTTON_DPAD_LEFT ||
      vrFrame.Input.buttonState & BUTTON_LSTICK_RIGHT ||
      vrFrame.Input.buttonState & BUTTON_DPAD_RIGHT) {
    buttonDownCount++;
  } else {
    buttonDownCount = 0;
  }

  if (ButtonPressed(vrFrame, BUTTON_LSTICK_UP) || ButtonPressed(vrFrame, BUTTON_DPAD_UP)) {
    currentMenu->MenuItems[currentMenu->CurrentSelection]->Selected = false;
    currentMenu->CurrentSelection--;
    buttonDownCount -= SCROLL_TIME;
  }

  if (ButtonPressed(vrFrame, BUTTON_LSTICK_DOWN) || ButtonPressed(vrFrame, BUTTON_DPAD_DOWN)) {
    currentMenu->MenuItems[currentMenu->CurrentSelection]->Selected = false;
    currentMenu->CurrentSelection++;
    buttonDownCount -= SCROLL_TIME;
  }

  if (currentMenu->CurrentSelection < 0)
    currentMenu->CurrentSelection = (int)(currentMenu->MenuItems.size() - 1);
  else if (currentMenu->CurrentSelection >= currentMenu->MenuItems.size())
    currentMenu->CurrentSelection = 0;

  currentMenu->MenuItems[currentMenu->CurrentSelection]->Selected = true;

  MenuButton *button =
      dynamic_cast<MenuButton *>(currentMenu->MenuItems[currentMenu->CurrentSelection]);

  if (button != NULL) {
    if (ButtonPressed(vrFrame, BUTTON_LSTICK_LEFT) || ButtonPressed(vrFrame, BUTTON_DPAD_LEFT)) {
      if (button->LeftFunction != nullptr) button->LeftFunction(button);
      buttonDownCount -= SCROLL_TIME_MOVE;
    }

    if (ButtonPressed(vrFrame, BUTTON_LSTICK_RIGHT) || ButtonPressed(vrFrame, BUTTON_DPAD_RIGHT)) {
      if (button->RightFunction != nullptr) button->RightFunction(button);
      buttonDownCount -= SCROLL_TIME_MOVE;
    }

    if (vrFrame.Input.buttonState & BUTTON_A && !(lastButtonState & BUTTON_A) &&
        button->PressFunction != nullptr) {
      button->PressFunction(button);
    }
  }

  if (vrFrame.Input.buttonState & BUTTON_B && !(lastButtonState & BUTTON_B) &&
      currentMenu->BackPress != nullptr) {
    currentMenu->BackPress();
  }
}

void UpdateGUI(const ovrFrameInput &vrFrame) {
  if (romSelection)
    UpdateRomSelection(vrFrame);
  else
    UpdateMenu(vrFrame);
}

void DrawMenu() {
  LOG("Draw Menu");

  float progress = sinf((1 - transitionState) * MATH_FLOAT_PIOVER2);

  // draw menu strings
  FontManager::Begin();

  for (uint i = 0; i < currentMenu->MenuItems.size(); i++)
    currentMenu->MenuItems[i]->DrawText(-transitionMoveDir * progress * 150, (1 - progress));

  if (isTransitioning)
    for (uint i = 0; i < nextMenu->MenuItems.size(); i++)
      nextMenu->MenuItems[i]->DrawText(transitionMoveDir * (1 - progress) * 100, progress);

  FontManager::Close();

  // draw the menu textures
  for (uint i = 0; i < currentMenu->MenuItems.size(); i++)
    currentMenu->MenuItems[i]->DrawTexture(-transitionMoveDir * progress * 150, (1 - progress));

  if (isTransitioning)
    for (uint i = 0; i < nextMenu->MenuItems.size(); i++)
      nextMenu->MenuItems[i]->DrawTexture(transitionMoveDir * (1 - progress) * 100, progress);
}

void DrawRomList() {
  // calculate the slider position
  float scale = maxListItems / (float)romFiles.size();
  if (scale > 1) scale = 1;
  GLfloat recHeight = scrollbarHeight * scale;

  GLfloat sliderPercentage = 0;
  if (romFiles.size() > maxListItems)
    sliderPercentage = (menuListState / (float)(romFiles.size() - maxListItems));
  else
    sliderPercentage = 0;

  GLfloat recPosY = (scrollbarHeight - recHeight) * sliderPercentage;

  // slider background
  DrawHelper::DrawTexture(textureWhiteId, listPosX, listPosY, scrollbarWidth, scrollbarHeight,
                          MenuBackgroundOverlayColor, 1);
  // slider
  DrawHelper::DrawTexture(textureWhiteId, listPosX, listPosY + recPosY, scrollbarWidth, recHeight,
                          sliderColor, 1);

  // draw the cartridge icons
  for (uint i = (uint)menuListState; i < menuListState + maxListItems; i++) {
    if (i < romFiles.size()) {
      DrawHelper::DrawTexture(
          romFiles[i].isGbc ? textureGbcIconId : textureGbIconId,
          listPosX + scrollbarWidth + 15 + (((uint)currentRomListSelection == i) ? 5 : 0),
          listStartY + listItemSize * (i - menuListState), 21, 24, {1.0f, 1.0f, 1.0f, 1.0f}, 1);
    }
  }

  FontManager::Begin();
  // draw rom list
  for (uint i = (uint)menuListState; i < menuListState + maxListItems; i++) {
    if (i < romFiles.size()) {
      FontManager::RenderText(
          fontList, romFiles[i].RomName,
          listPosX + scrollbarWidth + 44 + (((uint)currentRomListSelection == i) ? 5 : 0),
          listStartY + listItemSize * (i - menuListState), 1.0f,
          ((uint)currentRomListSelection == i) ? textSelectionColor : textColor, 1);
    } else
      break;
  }
  FontManager::Close();
}

void DrawGUI() {
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  glBindFramebuffer(GL_FRAMEBUFFER, MenuFrameBuffer);
  // Render on the whole framebuffer, complete from the lower left corner to the
  // upper right
  glViewport(0, 0, menuWidth, menuHeight);

  glClearColor(MenuBackgroundColor.x, MenuBackgroundColor.y, MenuBackgroundColor.z,
               MenuBackgroundColor.w);
  glClear(GL_COLOR_BUFFER_BIT);

  // header
  DrawHelper::DrawTexture(textureWhiteId, 0, 0, menuWidth, HEADER_HEIGHT,
                          MenuBackgroundOverlayColor, 1);
  // icon
  DrawHelper::DrawTexture(textureHeaderIconId, 0, 0, 75, 75, headerColor, 1);

  FontManager::Begin();
  FontManager::RenderText(fontHeader, strHeader, 75, 0.0f, 1.0f, headerColor, 1);

  // update the battery string
  GetBattryString(battery_string);
  FontManager::RenderText(fontSmall, battery_string,
                          menuWidth - strVersionWidth - 110.0f - batter_string_width,
                          HEADER_HEIGHT - 21, 1.0f, textColorVersion, 1);

  // update the time string
  GetTimeString(time_string);
  FontManager::RenderText(fontSmall, time_string, menuWidth - strVersionWidth - 70.0f,
                          HEADER_HEIGHT - 21, 1.0f, textColorVersion, 1);

  FontManager::RenderText(fontSmall, STR_VERSION, menuWidth - strVersionWidth - 7.0f,
                          HEADER_HEIGHT - 21, 1.0f, textColorVersion, 1);
  FontManager::Close();

  // draw battery
  int maxHeight = 15;
  DrawHelper::DrawTexture(textureWhiteId, menuWidth - strVersionWidth - 106.0f,
                          HEADER_HEIGHT - maxHeight - 4, 8, maxHeight, BatteryBackgroundColor, 1);
  int height = (int)(batteryLevel / 100.0 * maxHeight);
  DrawHelper::DrawTexture(textureWhiteId, menuWidth - strVersionWidth - 106.0f,
                          HEADER_HEIGHT - height - 4, 8, height, BatteryColor, 1);

  if (romSelection)
    DrawRomList();
  else
    DrawMenu();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ovrFrameResult OvrApp::Frame(const ovrFrameInput &vrFrame) {
  if (showExitDialog) {
    OvrApp::app->ShowSystemUI(VRAPI_SYS_UI_CONFIRM_QUIT_MENU);
    showExitDialog = false;
  }

  if (vrFrame.Input.buttonState & button_mapping_menu && !(lastButtonState & button_mapping_menu) &&
      loadedRom) {
    menuOpen = !menuOpen;
  }

  if (!menuOpen) {
    if (transitionPercentage > 0) transitionPercentage -= 0.25f;
    if (transitionPercentage < 0) transitionPercentage = 0;

    Emulator::Update(vrFrame, lastButtonState);
  } else {
    if (transitionPercentage < 1) transitionPercentage += 0.25f;
    if (transitionPercentage > 1) transitionPercentage = 1;

    UpdateGUI(vrFrame);
  }

  /*
  LOG("orientation: %f, %f, %f, %f",
  vrFrame.Tracking.HeadPose.Pose.Orientation.x,
      vrFrame.Tracking.HeadPose.Pose.Orientation.y,
  vrFrame.Tracking.HeadPose.Pose.Orientation.z,
      vrFrame.Tracking.HeadPose.Pose.Orientation.w);

  LOG("position: %f, %f, %f", vrFrame.Tracking.HeadPose.Pose.Position.x,
      vrFrame.Tracking.HeadPose.Pose.Position.y,
  vrFrame.Tracking.HeadPose.Pose.Position.z);
  */

  // Matrix4f mat = Matrix4f( vrFrame.Tracking.HeadPose.Pose );
  // Vector3f pointerStart = mat.Transform( Vector3f( 0.0f ) );
  // Vector3f pointerEnd = mat.Transform( Vector3f( 0.0f, 0.0f, -10.0f ) );

  // controllerSurface.modelMatrix = mat * Matrix4f::RotationY( controllerYaw )
  // * Matrix4f::RotationX( controllerPitch );

  // Player movement.
  // Scene.Frame(vrFrame);

  ovrFrameResult res;

  // Clear the eye buffers to 0 alpha so the overlay plane shows through.
  res.ClearColor = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
  res.ClearColorBuffer = true;

  res.FrameIndex = vrFrame.FrameNumber;
  res.DisplayTime = vrFrame.PredictedDisplayTimeInSeconds;
  res.SwapInterval = app->GetSwapInterval();

  res.FrameFlags = 0;
  res.LayerCount = 0;

  // res.FrameMatrices.CenterView = CenterEyeViewMatrix;

  /*
  Scene.GetFrameMatrices(vrFrame.FovX, vrFrame.FovY, res.FrameMatrices);
  Scene.GenerateFrameSurfaceList(res.FrameMatrices, res.Surfaces);


  ovrLayerProjection2 &worldLayer = res.Layers[res.LayerCount++].Projection;
  worldLayer = vrapi_DefaultLayerProjection2();

  worldLayer.HeadPose = vrFrame.Tracking.HeadPose;
  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
      worldLayer.Textures[eye].ColorSwapChain =
  vrFrame.ColorTextureSwapChain[eye]; worldLayer.Textures[eye].SwapChainIndex =
  vrFrame.TextureSwapChainIndex; worldLayer.Textures[eye].TexCoordsFromTanAngles
  = vrFrame.TexCoordsFromTanAngles;
  }

  worldLayer.Header.Flags |=
  VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;
  */

  // emulation screen layer
  // Add a simple cylindrical layer

  LayerBuilder::UpdateDirection(vrFrame);

  // virtual screen layer
  res.Layers[res.LayerCount].Cylinder =
      LayerBuilder::BuildCylinderLayer(Emulator::CylinderSwapChain, Emulator::CylinderWidth,
                                       Emulator::CylinderHeight, &vrFrame.Tracking, followHead);

  res.Layers[res.LayerCount].Cylinder.Header.Flags |=
      VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;
  res.Layers[res.LayerCount].Cylinder.Header.SrcBlend = VRAPI_FRAME_LAYER_BLEND_ONE;
  res.Layers[res.LayerCount].Cylinder.Header.DstBlend = VRAPI_FRAME_LAYER_BLEND_ZERO;
  res.LayerCount++;

  if (transitionPercentage > 0) {
    // menu layer
    if (menuOpen) DrawGUI();

    res.Layers[res.LayerCount].Cylinder = LayerBuilder::BuildSettingsCylinderLayer(
        MenuSwapChain, menuWidth, menuHeight, &vrFrame.Tracking, followHead);

    res.Layers[res.LayerCount].Cylinder.Header.Flags |=
        VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;
    res.Layers[res.LayerCount].Cylinder.Header.ColorScale = {
        transitionPercentage, transitionPercentage, transitionPercentage, transitionPercentage};
    res.Layers[res.LayerCount].Cylinder.Header.SrcBlend = VRAPI_FRAME_LAYER_BLEND_SRC_ALPHA;
    res.Layers[res.LayerCount].Cylinder.Header.DstBlend =
        VRAPI_FRAME_LAYER_BLEND_ONE_MINUS_SRC_ALPHA;
    res.LayerCount++;
  }

  lastButtonState = vrFrame.Input.buttonState;

  return res;
}

void StartTransition(Menu *next, int dir) {
  if (isTransitioning) return;
  isTransitioning = true;
  transitionMoveDir = dir;
  nextMenu = next;
}

void OnClickResumGame(MenuButton *item) {
  LOG("Pressed RESUME GAME");
  menuOpen = false;
}

void OnClickResetGame(MenuButton *item) {
  LOG("RESET GAME");
  Emulator::ResetGame();
  menuOpen = false;
}

void OnClickSaveGame(MenuButton *item) {
  LOG("on click save game");
  Emulator::SaveState(saveSlot);
  menuOpen = false;
}

void OnClickLoadGame(MenuButton *item) {
  Emulator::LoadState(saveSlot);
  menuOpen = false;
}

void OnClickLoadRomGame(MenuButton *item) { romSelection = true; }

void OnClickSettingsGame(MenuButton *item) { StartTransition(&settingsMenu, 1); }

void ChangePalette(MenuButton *item, int dir) {
  Emulator::ChangePalette(dir);
  item->Text = "Palette: " + to_string(Emulator::selectedPalette);
}

void OnClickChangePaletteLeft(MenuButton *item) {
  ChangePalette(item, -1);
  SaveSettings();
}

void OnClickChangePaletteRight(MenuButton *item) {
  ChangePalette(item, 1);
  SaveSettings();
}

void SetForceDMG(MenuButton *item, bool newForceDMG) {
  Emulator::forceDMG = newForceDMG;
  item->Text = strForceDMG[Emulator::forceDMG ? 0 : 1];
}

void OnClickEmulatedModel(MenuButton *item) {
  SetForceDMG(item, !Emulator::forceDMG);
  SaveSettings();
}

void OnClickFollowMode(MenuButton *item) {
  followHead = !followHead;
  item->Text = strMove[followHead ? 0 : 1];
  SaveSettings();
}

void OnClickMoveScreen(MenuButton *item) { StartTransition(&moveMenu, 1); }

void OnClickMappingScreen(MenuButton *item) { StartTransition(&buttonMapMenu, 1); }

void OnClickAllowLeftRight(MenuButton *item) {
  allowUpDownSlashLeftRight = !allowUpDownSlashLeftRight;
  item->Text = "Allow Up+Down / Left+Right: ";
  item->Text += (allowUpDownSlashLeftRight ? "Enabled" : "Disabled");
}

void OnClickBackAndSave(MenuButton *item) {
  StartTransition(&mainMenu, -1);
  SaveSettings();
}

void OnBackPressedSettings() {
  StartTransition(&mainMenu, -1);
  SaveSettings();
}

void OnClickBackMove(MenuButton *item) {
  StartTransition(&settingsMenu, -1);
  SaveSettings();
}

void OnBackPressedMove() {
  StartTransition(&settingsMenu, -1);
  SaveSettings();
}

void OnClickSaveSlotLeft(MenuButton *item) {
  saveSlot--;
  if (saveSlot < 0) saveSlot = MAX_SAVESLOTS - 1;
  item->Text = "Save Slot: " + to_string(saveSlot);
  Emulator::UpdateStateImage(saveSlot);
  SaveSettings();
}

void OnClickSaveSlotRight(MenuButton *item) {
  saveSlot++;
  if (saveSlot >= MAX_SAVESLOTS) saveSlot = 0;
  item->Text = "Save Slot: " + to_string(saveSlot);
  Emulator::UpdateStateImage(saveSlot);
  SaveSettings();
}

float ToDegree(float radian) { return (int)(180.0 / VRAPI_PI * radian * 10) / 10.0f; }

void MoveYaw(MenuButton *item, float dir) {
  LayerBuilder::screenYaw -= dir;
  item->Text = "Yaw: " + to_string(ToDegree(LayerBuilder::screenYaw));
}

void MovePitch(MenuButton *item, float dir) {
  LayerBuilder::screenPitch -= dir;
  item->Text = "Pitch: " + to_string(ToDegree(LayerBuilder::screenPitch));
}

void MoveRoll(MenuButton *item, float dir) {
  LayerBuilder::screenRoll -= dir;
  item->Text = "Roll: " + to_string(ToDegree(LayerBuilder::screenRoll));
}

void ChangeDistance(MenuButton *item, float dir) {
  LayerBuilder::radiusMenuScreen -= dir;

  if (LayerBuilder::radiusMenuScreen < MIN_RADIUS) LayerBuilder::radiusMenuScreen = MIN_RADIUS;
  if (LayerBuilder::radiusMenuScreen > MAX_RADIUS) LayerBuilder::radiusMenuScreen = MAX_RADIUS;

  item->Text = "Distance: " + to_string(LayerBuilder::radiusMenuScreen);
}

void ChangeScale(MenuButton *item, float dir) {
  LayerBuilder::screenSize -= dir;

  if (LayerBuilder::screenSize < 0.5f) LayerBuilder::screenSize = 0.5f;
  if (LayerBuilder::screenSize > 2.0f) LayerBuilder::screenSize = 2.0f;

  item->Text = "Scale: " + to_string(LayerBuilder::screenSize);
}

void OnClickResetView(MenuButton *item) {
  LayerBuilder::ResetValues();

  // updates the visible values
  MoveYaw(yawButton, 0);
  MovePitch(pitchButton, 0);
  MoveRoll(rollButton, 0);
  ChangeDistance(distanceButton, 0);
  ChangeScale(scaleButton, 0);

  SaveSettings();
}

void MoveAButtonMapping(MenuButton *item, int dir) {
  Emulator::ChangeButtonMapping(0, dir);
  item->Text = "mapped to: " + MapButtonStr[Emulator::button_mapping_index[0]];
}

void MoveBButtonMapping(MenuButton *item, int dir) {
  Emulator::ChangeButtonMapping(1, dir);
  item->Text = "mapped to: " + MapButtonStr[Emulator::button_mapping_index[1]];
}

void MoveMenuButtonMapping(MenuButton *item, int dir) {
  button_mapping_menu_index += dir;
  if (button_mapping_menu_index > 3) button_mapping_menu_index = 2;

  button_mapping_menu = MapButtons[button_mapping_menu_index];
  item->Text = "menu mapped to: " + MapButtonStr[button_mapping_menu_index];
}

void OnClickChangeMenuButtonLeft(MenuButton *item) { MoveMenuButtonMapping(item, 1); }
void OnClickChangeMenuButtonRight(MenuButton *item) { MoveMenuButtonMapping(item, 1); }

void OnClickChangeAButtonLeft(MenuButton *item) { MoveAButtonMapping(item, -1); }
void OnClickChangeAButtonRight(MenuButton *item) { MoveAButtonMapping(item, 1); }

void OnClickChangeBButtonLeft(MenuButton *item) { MoveBButtonMapping(item, -1); }
void OnClickChangeBButtonRight(MenuButton *item) { MoveBButtonMapping(item, 1); }

void OnClickMoveScreenYawLeft(MenuButton *item) { MoveYaw(item, MoveSpeed); }

void OnClickMoveScreenYawRight(MenuButton *item) { MoveYaw(item, -MoveSpeed); }

void OnClickMoveScreenPitchLeft(MenuButton *item) { MovePitch(item, -MoveSpeed); }

void OnClickMoveScreenPitchRight(MenuButton *item) { MovePitch(item, MoveSpeed); }

void OnClickMoveScreenRollLeft(MenuButton *item) { MoveRoll(item, -MoveSpeed); }

void OnClickMoveScreenRollRight(MenuButton *item) { MoveRoll(item, MoveSpeed); }

void OnClickMoveScreenDistanceLeft(MenuButton *item) { ChangeDistance(item, ZoomSpeed); }

void OnClickMoveScreenDistanceRight(MenuButton *item) { ChangeDistance(item, -ZoomSpeed); }

void OnClickMoveScreenScaleLeft(MenuButton *item) { ChangeScale(item, MoveSpeed); }

void OnClickMoveScreenScaleRight(MenuButton *item) { ChangeScale(item, -MoveSpeed); }

void OnClickExit(MenuButton *item) {
  Emulator::SaveRam();
  showExitDialog = true;
}

void OvrApp::SetUpMenu() {
  // main menu page
  int posX = 20;
  int posY = HEADER_HEIGHT + 20;
  std::string strSaveSlot = "Save Slot: " + to_string(saveSlot);
  mainMenu.MenuItems.push_back(new MenuButton(textureResumeId, "Resume Game", posX, posY,
                                              OnClickResumGame, nullptr, nullptr));
  mainMenu.MenuItems.push_back(new MenuButton(textureResetIconId, "Reset Game", posX,
                                              posY += menuItemSize, OnClickResetGame, nullptr,
                                              nullptr));
  mainMenu.MenuItems.push_back(new MenuButton(textureSaveSlotIconId, strSaveSlot, posX,
                                              posY += menuItemSize + 10, OnClickSaveSlotRight,
                                              OnClickSaveSlotLeft, OnClickSaveSlotRight));
  mainMenu.MenuItems.push_back(new MenuButton(textureSaveIconId, "Save", posX, posY += menuItemSize,
                                              OnClickSaveGame, nullptr, nullptr));
  mainMenu.MenuItems.push_back(new MenuButton(textureLoadIconId, "Load", posX, posY += menuItemSize,
                                              OnClickLoadGame, nullptr, nullptr));
  mainMenu.MenuItems.push_back(new MenuButton(textureLoadRomIconId, "Load Rom", posX,
                                              posY += menuItemSize + 10, OnClickLoadRomGame,
                                              nullptr, nullptr));
  mainMenu.MenuItems.push_back(new MenuButton(textureSettingsId, "Settings", posX,
                                              posY += menuItemSize, OnClickSettingsGame, nullptr,
                                              nullptr));
  mainMenu.MenuItems.push_back(new MenuButton(
      textureExitIconId, "Exit", posX, posY += menuItemSize + 10, OnClickExit, nullptr, nullptr));

  // background
  mainMenu.MenuItems.push_back(new MenuImage(textureWhiteId, menuWidth - 320 - 20 - 5,
                                             HEADER_HEIGHT + 20 - 5, 320 + 10, 288 + 10,
                                             MenuBackgroundOverlayColor));
  // image slot
  mainMenu.MenuItems.push_back(new MenuImage(Emulator::stateImageId, menuWidth - 320 - 20,
                                             HEADER_HEIGHT + 20, 320, 288,
                                             {1.0f, 1.0f, 1.0f, 1.0f}));
  emptySlotLabel = new MenuLabel(fontSlot, "--Empty Slot--", menuWidth - 320 - 20,
                                 HEADER_HEIGHT + 20, 320, 288, {1.0f, 1.0f, 1.0f, 1.0f});
  mainMenu.MenuItems.push_back(emptySlotLabel);

  /*
  if (Emulator::currentGame->saveStates[saveSlot].filled) {
    DrawHelper::DrawTexture(Emulator::stateImageId, menuWidth - 320 - 20, HEADER_HEIGHT + 20,
                            160 * 2, 144 * 2, {1.0f, 1.0f, 1.0f, 1.0f}, 1);
  } else {
    // menuWidth - 320 - 20, HEADER_HEIGHT + 20, 320, 288
    FontManager::Begin();
    FontManager::RenderText(fontSlot, strNoSave, menuWidth - 160 - 20 - strNoSaveWidth / 2,
                            HEADER_HEIGHT + 20 + 144 - 26, 1.0f, {0.95f, 0.95f, 0.95f, 1.0f}, 1);
    FontManager::Close();
  }
  */

  // settings page
  posY = HEADER_HEIGHT + 20;

  settingsMenu.MenuItems.push_back(new MenuButton(textureMappingIconId, "Button Mapping", posX,
                                                  posY, OnClickMappingScreen, nullptr, nullptr));
  settingsMenu.MenuItems.push_back(new MenuButton(textureMoveIconId, "Move Screen", posX,
                                                  posY += menuItemSize, OnClickMoveScreen, nullptr,
                                                  nullptr));
  settingsMenu.MenuItems.push_back(new MenuButton(
      textureFollowHeadIconId, strMove[followHead ? 0 : 1], posX, posY += menuItemSize + 5,
      OnClickFollowMode, OnClickFollowMode, OnClickFollowMode));

  MenuButton *paletteButton = new MenuButton(texturePaletteIconId, "", posX, posY += menuItemSize,
                                             OnClickChangePaletteRight, OnClickChangePaletteLeft,
                                             OnClickChangePaletteRight);
  MenuButton *dmgButton =
      new MenuButton(textureDMGIconId, "", posX, posY += menuItemSize, OnClickEmulatedModel,
                     OnClickEmulatedModel, OnClickEmulatedModel);

  settingsMenu.MenuItems.push_back(paletteButton);
  settingsMenu.MenuItems.push_back(dmgButton);
  settingsMenu.MenuItems.push_back(new MenuButton(textureBackIconId, "Back", posX,
                                                  posY += menuItemSize + 5, OnClickBackAndSave,
                                                  nullptr, nullptr));

  // state screenshot background
  settingsMenu.MenuItems.push_back(new MenuImage(textureWhiteId, menuWidth - 320 - 20 - 5,
                                                 HEADER_HEIGHT + 20 - 5, 320 + 10, 288 + 10,
                                                 MenuBackgroundOverlayColor));
  settingsMenu.MenuItems.push_back(new MenuImage(Emulator::textureID, menuWidth - 320 - 20,
                                                 HEADER_HEIGHT + 20, 320, 288,
                                                 {1.0f, 1.0f, 1.0f, 1.0f}));

  settingsMenu.BackPress = OnBackPressedSettings;
  // set text
  ChangePalette(paletteButton, 0);
  SetForceDMG(dmgButton, Emulator::forceDMG);

  // button mapping page
  posY = HEADER_HEIGHT + 20;

  MenuButton *menuButton =
      new MenuButton(textureLoadRomIconId, "", posX, posY, OnClickChangeMenuButtonRight,
                     OnClickChangeMenuButtonLeft, OnClickChangeMenuButtonRight);
  MenuButton *aButton = new MenuButton(textureButtonAIconId, "", posX, posY += menuItemSize,
                                       OnClickChangeAButtonRight, OnClickChangeAButtonLeft,
                                       OnClickChangeAButtonRight);
  MenuButton *bButton = new MenuButton(textureButtonBIconId, "", posX, posY += menuItemSize,
                                       OnClickChangeBButtonRight, OnClickChangeBButtonLeft,
                                       OnClickChangeBButtonRight);
  MoveMenuButtonMapping(menuButton, 0);
  MoveAButtonMapping(aButton, 0);
  MoveBButtonMapping(bButton, 0);

  buttonMapMenu.MenuItems.push_back(menuButton);
  buttonMapMenu.MenuItems.push_back(aButton);
  buttonMapMenu.MenuItems.push_back(bButton);
  buttonMapMenu.MenuItems.push_back(new MenuButton(textureBackIconId, "Back", posX,
                                                   posY += menuItemSize + 5, OnClickBackMove,
                                                   nullptr, nullptr));
  buttonMapMenu.BackPress = OnBackPressedMove;

  // move menu page
  posY = HEADER_HEIGHT + 20;
  yawButton = new MenuButton(texuterLeftRightIconId, "", posX, posY, nullptr,
                             OnClickMoveScreenYawLeft, OnClickMoveScreenYawRight);
  pitchButton = new MenuButton(textureUpDownIconId, "", posX, posY += menuItemSize, nullptr,
                               OnClickMoveScreenPitchLeft, OnClickMoveScreenPitchRight);
  rollButton = new MenuButton(textureResetIconId, "", posX, posY += menuItemSize, nullptr,
                              OnClickMoveScreenRollLeft, OnClickMoveScreenRollRight);
  distanceButton = new MenuButton(textureDistanceIconId, "", posX, posY += menuItemSize, nullptr,
                                  OnClickMoveScreenDistanceLeft, OnClickMoveScreenDistanceRight);
  scaleButton = new MenuButton(textureScaleIconId, "", posX, posY += menuItemSize, nullptr,
                               OnClickMoveScreenScaleLeft, OnClickMoveScreenScaleRight);

  moveMenu.MenuItems.push_back(yawButton);
  moveMenu.MenuItems.push_back(pitchButton);
  moveMenu.MenuItems.push_back(rollButton);
  moveMenu.MenuItems.push_back(distanceButton);
  moveMenu.MenuItems.push_back(scaleButton);

  moveMenu.MenuItems.push_back(new MenuButton(textureResetViewIconId, "Reset View", posX,
                                              posY += menuItemSize + 5, OnClickResetView, nullptr,
                                              nullptr));
  moveMenu.MenuItems.push_back(new MenuButton(textureBackIconId, "Back", posX,
                                              posY += menuItemSize + 5, OnClickBackMove, nullptr,
                                              nullptr));
  moveMenu.BackPress = OnBackPressedMove;

  // updates the visible values
  MoveYaw(yawButton, 0);
  MovePitch(pitchButton, 0);
  MoveRoll(rollButton, 0);
  ChangeDistance(distanceButton, 0);
  ChangeScale(scaleButton, 0);

  strVersionWidth = GetWidth(fontSmall, STR_VERSION);
  strMoveMenuTextWidth = GetWidth(fontMenu, strMoveMenu);
  strNoSaveWidth = GetWidth(fontSlot, strNoSave);

  currentMenu = &mainMenu;
}