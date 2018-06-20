#include <OVR_FileSys.h>
#include <OVR_Input.h>
#include <VrApi_Input.h>
#include <VrApi_Types.h>
#include <cstdlib>
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

#include <gli/clear.hpp>       // clear, clear_level, clear_layer
#include <gli/comparison.hpp>  // == and != operators on textures and images
#include <gli/convert.hpp>     // convert a texture from a format to another
#include <gli/copy.hpp>        // copy a texture or subset of a texture to another texture
#include <gli/duplicate.hpp>   // duplicate the data of a texture, allocating a new texture storage
#include <gli/dx.hpp>          // facilitate the use of GLI with Direct3D API
#include <gli/format.hpp>      // list of the supported formats
#include <gli/generate_mipmaps.hpp>  // generating the mipmaps of a texture
#include <gli/gl.hpp>                // translate GLI enums to OpenGL enums
#include <gli/gli.hpp>
#include <gli/image.hpp>  // use images, a representation of a single texture level.
#include <gli/levels.hpp>  // compute the number of mipmaps levels necessary to create a mipmap complete texture.
#include <gli/load.hpp>                // load DDS, KTX or KMG textures from files or memory.
#include <gli/load_dds.hpp>            // load DDS textures from files or memory.
#include <gli/load_kmg.hpp>            // load KMG textures from files or memory.
#include <gli/load_ktx.hpp>            // load KTX textures from files or memory.
#include <gli/make_texture.hpp>        // helper functions to create generic texture
#include <gli/reduce.hpp>              // include to perform reduction operations.
#include <gli/sampler.hpp>             // enumations for texture sampling
#include <gli/sampler1d.hpp>           // class to sample 1d texture
#include <gli/sampler1d_array.hpp>     // class to sample 1d array texture
#include <gli/sampler2d.hpp>           // class to sample 2d texture
#include <gli/sampler2d_array.hpp>     // class to sample 2d array texture
#include <gli/sampler3d.hpp>           // class to sample 3d texture
#include <gli/sampler_cube.hpp>        // class to sample cube texture
#include <gli/sampler_cube_array.hpp>  // class to sample cube array texture
#include <gli/save.hpp>                // save either a DDS, KTX or KMG file
#include <gli/save_dds.hpp>            // save a DDS texture file
#include <gli/save_kmg.hpp>            // save a KMG texture file
#include <gli/save_ktx.hpp>            // save a KTX texture file
#include <gli/target.hpp>              // helper function to query property of a generic texture
#include <gli/texture.hpp>          // generic texture class that can represent any kind of texture
#include <gli/texture1d.hpp>        // representation of a 1d texture
#include <gli/texture1d_array.hpp>  // representation of a 1d array texture
#include <gli/texture2d.hpp>        // representation of a 2d texture
#include <gli/texture2d_array.hpp>  // representation of a 2d array texture
#include <gli/texture3d.hpp>        // representation of a 3d texture
#include <gli/texture_cube.hpp>     // representation of a cube texture
#include <gli/texture_cube_array.hpp>  // representation of a cube array texture
#include <gli/transform.hpp>           // perform operation on source data to destination data
#include <gli/type.hpp>                // extent*d types
#include <gli/view.hpp>  // create a texture view, same storage but a different scope or interpretation of the data

#include <gearboy.h>
#include <gli/format.hpp>
#include "Audio/OpenSLWrap.h"

#include "DrawHelper.h"
#include "FontMaster.h"
#include "LayerBuilder.h"

using namespace OVR;

struct Rom {
  bool isGbc;
  std::string RomName;
  std::string FullPath;
  std::string FullPathNorm;
  std::string SavePath;
};

struct SaveState {
  bool filled;
  GB_Color *saveImage;
};

struct LoadedGame {
  SaveState saveStates[10];
};

#define VIDEO_WIDTH 160
#define VIDEO_HEIGHT 144

#define menuWidth 640
#define menuHeight 576

// const int menuWidth = 570;
// const int menuHeight = 500;

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

const int SAVE_FILE_VERSION = 1;

GB_Color *gearboy_frame_buf;
GearboyCore *core;

static s16 audio_buf[AUDIO_BUFFER_SIZE * 2];
static int audio_sample_count;
static int audio_sample_skip_count;

LoadedGame *currentGame;

Rom *CurrentRom;

std::vector<Rom> romFiles;

GlProgram glprog;

int listPosX = 0;
int listPosY = 0;
int scrollbarWidth = 10;
int scrollbarHeight = 400;
int listItemSize;
int menuItemSize;
int listStartY;

bool audioInit = false;
bool menuOpen = true;
bool romSelection = true;
bool loadedRom;

// saved variables
bool forceDMG = false;
bool followHead;
int selectedPalette = 20;
int saveSlot;
bool allowUpDownSlashLeftRight;  // probably not necessary

int currentRomListSelection;
int menuListState;
int maxListItems = 18;

std::string strForceDMG[] = {"Force DMG: Yes", "Force DMG: No"};
std::string strMove[] = {"Follow Head: Yes", "Follow Head: No"};
std::string strHeader = "GearboyVR";
std::string strNoSave = "--Empty Slot--";
std::string strMoveMenu = "<- Move Screen ->";
int strMoveMenuTextWidth;
int strNoSaveWidth;

unsigned int lastButtonState;

const int CylinderWidth = 160 * 2;
const int CylinderHeight = 144 * 2;

const char *appDir;
const char *storageDir;

std::string saveFilePath;
std::string stateFolderPath;
std::string romFolderPath;

Vector3f headerColor = Vector3f(1.0f, 1.0f, 1.0f);
Vector3f menuColor = Vector3f(1.0f, 1.0f, 1.0f);
Vector3f menuSelectionColor = Vector3f(0.0f, 0.5f, 0.8f);
Vector4f sliderColor = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);

int buttonDownCount;

FontManager::RenderFont fontHeader, fontMenu, fontList, fontSlot, fontSmall;

int strVersionWidth;

Vector4f CLEAR_COLOR(0.7f, 0.7f, 0.7f, 1.0f);
Vector4f MenuBackgroundColor{0.05f, 0.05f, 0.05f, 0.95f};

ovrTextureSwapChain *CylinderSwapChain;
ovrTextureSwapChain *MenuSwapChain;
GLuint MenuFrameBuffer = 0;

uint32_t *texData;

GLuint textureIdScreen, textureIdMenu, textureIdSlotImage, textureHeaderIconId, textureGbIconId,
    textureGbcIconId, textureSaveIconId, textureLoadIconId, textureWhiteId;

const int paletteCount = 31;
static GB_Color palettes[paletteCount][4] = {
    {{0x03, 0x96, 0x87, 0xFF},
     {0x03, 0x6B, 0x4D, 0xFF},
     {0x03, 0x55, 0x2B, 0xFF},
     {0x03, 0x44, 0x14, 0xFF}},
    {{0xEF, 0xFA, 0xF5, 0xFF},
     {0x70, 0xC2, 0x86, 0xFF},
     {0x57, 0x69, 0x2F, 0xFF},
     {0x20, 0x19, 0x0B, 0xFF}},
    {{0xFF, 0xFF, 0xFF, 0xFF},
     {0xAA, 0xAA, 0xAA, 0xFF},
     {0x55, 0x55, 0x55, 0xFF},
     {0x00, 0x00, 0x00, 0xFF}},
    {{0xC8, 0xE8, 0xF8, 0xFF},
     {0x48, 0x90, 0xD8, 0xFF},
     {0x20, 0x34, 0xA8, 0xFF},
     {0x50, 0x18, 0x30, 0xFF}},
    {{0xAA, 0xE0, 0xE0, 0xFF},
     {0x7C, 0xB8, 0xB0, 0xFF},
     {0x5B, 0x82, 0x72, 0xFF},
     {0x17, 0x34, 0x39, 0xFF}},
    {{0xA5, 0xEB, 0xD4, 0xFF},
     {0x7C, 0xB8, 0x62, 0xFF},
     {0x5D, 0x76, 0x27, 0xFF},
     {0x39, 0x39, 0x1D, 0xFF}},
    {{0xCE, 0xF7, 0xF7, 0xFF},
     {0xF7, 0x8E, 0x50, 0xFF},
     {0x9E, 0x00, 0x00, 0xFF},
     {0x1E, 0x00, 0x00, 0xFF}},
    {{0xA9, 0x68, 0x68, 0xFF},
     {0xED, 0xB4, 0xA1, 0xFF},
     {0x76, 0x44, 0x62, 0xFF},
     {0x2C, 0x21, 0x37, 0xFF}},
    {{0xAE, 0xDF, 0x1E, 0xFF},
     {0xB6, 0x25, 0x58, 0xFF},
     {0x04, 0x7E, 0x60, 0xFF},
     {0x2C, 0x17, 0x00, 0xFF}},
    {{0x7E, 0x84, 0x16, 0xFF},
     {0x57, 0x7B, 0x46, 0xFF},
     {0x38, 0x5D, 0x49, 0xFF},
     {0x2E, 0x46, 0x3D, 0xFF}},
    {{0xAC, 0xB5, 0x6B, 0xFF},
     {0x76, 0x84, 0x48, 0xFF},
     {0x3F, 0x50, 0x3F, 0xFF},
     {0x24, 0x31, 0x37, 0xFF}},
    {{0xF7, 0xBE, 0xF7, 0xFF},
     {0xE7, 0x86, 0x86, 0xFF},
     {0x77, 0x33, 0xE7, 0xFF},
     {0x2C, 0x2C, 0x96, 0xFF}},
    {{0xF7, 0xE7, 0xC6, 0xFF},
     {0xD6, 0x8E, 0x49, 0xFF},
     {0xA6, 0x37, 0x25, 0xFF},
     {0x33, 0x1E, 0x50, 0xFF}},
    {{0xC4, 0xCF, 0xA1, 0xFF},
     {0x8B, 0x95, 0x6D, 0xFF},
     {0x4D, 0x53, 0x3C, 0xFF},
     {0x1F, 0x1F, 0x1F, 0xFF}},
    {{0xCE, 0xCE, 0xCE, 0xFF},
     {0x6F, 0x9E, 0xDF, 0xFF},
     {0x42, 0x67, 0x8E, 0xFF},
     {0x10, 0x25, 0x33, 0xFF}},
    {{0xFF, 0xE4, 0xC2, 0xFF},
     {0xDC, 0xA4, 0x56, 0xFF},
     {0xA9, 0x60, 0x4C, 0xFF},
     {0x42, 0x29, 0x36, 0xFF}},
    {{0x8B, 0xE5, 0xFF, 0xFF},
     {0x60, 0x8F, 0xCF, 0xFF},
     {0x75, 0x50, 0xE8, 0xFF},
     {0x62, 0x2E, 0x4C, 0xFF}},
    {{0xE0, 0xDB, 0xCD, 0xFF},
     {0xA8, 0x9F, 0x94, 0xFF},
     {0x70, 0x6B, 0x66, 0xFF},
     {0x2B, 0x2B, 0x26, 0xFF}},
    {{0xA1, 0xEF, 0x8C, 0xFF},
     {0x3F, 0xAC, 0x95, 0xFF},
     {0x44, 0x61, 0x76, 0xFF},
     {0x2C, 0x21, 0x37, 0xFF}},
    {{0xFF, 0xFF, 0xFF, 0xFF},
     {0xB6, 0xB6, 0xB6, 0xFF},
     {0x67, 0x67, 0x67, 0xFF},
     {0x00, 0x00, 0x00, 0xFF}},
    {{0xC4, 0xF0, 0xC2, 0xFF},
     {0x5A, 0xB9, 0xA8, 0xFF},
     {0x1E, 0x60, 0x6E, 0xFF},
     {0x2D, 0x1B, 0x00, 0xFF}},
    {{0xE3, 0xEE, 0xC0, 0xFF},
     {0xAE, 0xBA, 0x89, 0xFF},
     {0x5E, 0x67, 0x45, 0xFF},
     {0x20, 0x20, 0x20, 0xFF}},
    {{0xFF, 0xEF, 0xFF, 0xFF},
     {0xF7, 0xB5, 0x8C, 0xFF},
     {0x84, 0x73, 0x9C, 0xFF},
     {0x18, 0x10, 0x10, 0xFF}},
    {{0xE0, 0xF8, 0xD0, 0xFF},
     {0x88, 0xC0, 0x70, 0xFF},
     {0x34, 0x68, 0x56, 0xFF},
     {0x08, 0x18, 0x20, 0xFF}},
    {{0xFF, 0xF5, 0xDD, 0xFF},
     {0xF4, 0xB2, 0x6B, 0xFF},
     {0xB7, 0x65, 0x91, 0xFF},
     {0x65, 0x29, 0x6C, 0xFF}},
    {{0xEB, 0xDD, 0x77, 0xFF},
     {0xA1, 0xBC, 0x00, 0xFF},
     {0x0D, 0x88, 0x33, 0xFF},
     {0x00, 0x43, 0x33, 0xFF}},
    {{0xFF, 0xF6, 0xD3, 0xFF},
     {0xF9, 0xA8, 0x75, 0xFF},
     {0xEB, 0x6B, 0x6F, 0xFF},
     {0x7C, 0x3F, 0x58, 0xFF}},
    {{0xEF, 0xF7, 0xB6, 0xFF},
     {0xDF, 0xA6, 0x77, 0xFF},
     {0x11, 0xC6, 0x00, 0xFF},
     {0x00, 0x00, 0x00, 0xFF}},
    {{0xFF, 0xFF, 0xB5, 0xFF},
     {0x7B, 0xC6, 0x7B, 0xFF},
     {0x6B, 0x8C, 0x42, 0xFF},
     {0x5A, 0x39, 0x21, 0xFF}},
    {{0xE2, 0xF3, 0xE4, 0xFF},
     {0x94, 0xE3, 0x44, 0xFF},
     {0x46, 0x87, 0x8F, 0xFF},
     {0x33, 0x2C, 0x50, 0xFF}},
    {{0xDB, 0xF4, 0xB4, 0xFF},
     {0xAB, 0xC3, 0x96, 0xFF},
     {0x7B, 0x92, 0x78, 0xFF},
     {0x4C, 0x62, 0x5A, 0xFF}},
};

static GB_Color *current_palette = palettes[selectedPalette];

// TODO version 1.1
// fix bugs
// look at the best possible menu resolution
// draw button functions on the bottom of the menu
// close emulator
// show slot even when there is no image but a save state

// TODO version 2.0
// turbo button
// time + batterie life
// sprite palette and tile palette
// change button placement
// menu themes?
// icons for gb and gbc games (maybe sort them together?)
// menu icons
// infinite states?
// error messages if game could not be loaded, etc
// sort paletts
// palette settings for single game

template <typename T>
std::string to_string(T value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

struct MenuItem {
  GLuint iconId;

  std::string Text;

  void (*PressFunction)(MenuItem *item);

  void (*LeftFunction)(MenuItem *item);

  void (*RightFunction)(MenuItem *item);
};

struct Menu {
  void (*BackPress)();

  int CurrentSelection;

  std::vector<MenuItem> MenuItems;
};

Menu *currentMenu;
Menu mainMenu, settingsMenu, moveMenu;

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
      SceneModel(NULL) {
  LOG("new gearboycore");
  core = new GearboyCore();
  LOG("init gearboy");

  core->Init();

  gearboy_frame_buf = new GB_Color[VIDEO_WIDTH * VIDEO_HEIGHT];

  currentGame = new LoadedGame();

  for (int i = 0; i < 10; ++i) {
    currentGame->saveStates[i].saveImage =
        (GB_Color *)malloc(VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint32_t));
    // new GB_Color[VIDEO_WIDTH * VIDEO_HEIGHT];
  }

  texData = (uint32_t *)malloc(CylinderWidth * CylinderHeight * sizeof(uint32_t));
}

OvrApp::~OvrApp() {
  delete SoundEffectPlayer;
  SoundEffectPlayer = NULL;

  delete SoundEffectContext;
  SoundEffectContext = NULL;

  OvrGuiSys::Destroy(GuiSys);
  if (SceneModel != NULL) {
    delete SceneModel;
  }

  SafeDelete(core);
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

void OvrApp::Configure(ovrSettings &settings) {
  settings.CpuLevel = 0;
  settings.GpuLevel = 0;

  /*
  settings.EyeBufferParms.multisamples = 4;
  settings.EyeBufferParms.resolutionWidth = 1500;
  settings.EyeBufferParms.resolutionWidth = 1500;
  */

  settings.RenderMode = RENDERMODE_MULTIVIEW;
  settings.UseSrgbFramebuffer = true;
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
  // emu screen layer
  CylinderSwapChain = vrapi_CreateTextureSwapChain(VRAPI_TEXTURE_TYPE_2D, VRAPI_TEXTURE_FORMAT_8888,
                                                   CylinderWidth, CylinderHeight, 1, false);

  textureIdScreen = vrapi_GetTextureSwapChainHandle(CylinderSwapChain, 0);
  glBindTexture(GL_TEXTURE_2D, textureIdScreen);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CylinderWidth, CylinderHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                  NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  GLfloat borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

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

void UpdateScreen() {
  for (int y = 0; y < CylinderHeight; y++) {
    for (int x = 0; x < CylinderWidth; x++) {
      // needs to change for other libretro cores
      /* texData[y * CylinderWidth + x] =
                      (gearboy_frame_buf[(y / 2) * VIDEO_WIDTH + (x / 2)].red <<
         0) | (gearboy_frame_buf[(y / 2) * VIDEO_WIDTH + (x / 2)].green << 8) |
                      (gearboy_frame_buf[(y / 2) * VIDEO_WIDTH + (x / 2)].blue
         << 16) | (gearboy_frame_buf[(y / 2) * VIDEO_WIDTH + (x / 2)].alpha <<
         24); */
      memcpy(&texData[y * CylinderWidth + x], &gearboy_frame_buf[(y / 2) * VIDEO_WIDTH + (x / 2)],
             4);
    }
  }

  glBindTexture(GL_TEXTURE_2D, textureIdScreen);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CylinderWidth, CylinderHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                  texData);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  GLfloat borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void CreateSlotImage() {
  glGenTextures(1, &textureIdSlotImage);
  glBindTexture(GL_TEXTURE_2D, textureIdSlotImage);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void CreateWhiteImage() {
  uint32_t white = 0xFFFFFFFF;
  glGenTextures(1, &textureWhiteId);
  glBindTexture(GL_TEXTURE_2D, textureWhiteId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void UpdateSlotImage() {
  glBindTexture(GL_TEXTURE_2D, textureIdSlotImage);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VIDEO_WIDTH, VIDEO_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                  currentGame->saveStates[saveSlot].saveImage);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void SetPalette(GB_Color *newPalette) {
  current_palette = newPalette;
  core->SetDMGPalette(current_palette[0], current_palette[1], current_palette[2],
                      current_palette[3]);

  // only update if it is in gb mode
  if (!core->IsCGB()) {
    core->RenderDMGFrame(gearboy_frame_buf);
    UpdateScreen();
  }
}

void SaveRam() {
  if (CurrentRom != nullptr && core->GetMemory()->GetCurrentRule()->GetRamSize() > 0) {
    LOG("save ram %s", CurrentRom->SavePath.c_str());
    std::ofstream outfile(CurrentRom->SavePath, std::ios::trunc | std::ios::binary);
    outfile.write((const char *)core->GetMemory()->GetCurrentRule()->GetRamBanks(),
                  core->GetMemory()->GetCurrentRule()->GetRamSize());
    outfile.close();

    LOG("finished writing ram file");
  }
}

void LoadRam() {
  std::ifstream file(CurrentRom->SavePath, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    long romBufferSize = file.tellg();
    u8 *memblock = new u8[romBufferSize];
    file.seekg(0, std::ios::beg);
    file.read((char *)memblock, romBufferSize);
    file.close();
    LOG("loaded ram %ld", romBufferSize);

    LOG("ram size %u", (unsigned int)core->GetMemory()->GetCurrentRule()->GetRamSize());

    if (romBufferSize != (unsigned int)core->GetMemory()->GetCurrentRule()->GetRamSize()) {
      LOG("ERROR loaded ram size is wrong");
    } else {
      memcpy(core->GetMemory()->GetCurrentRule()->GetRamBanks(), memblock,
             core->GetMemory()->GetCurrentRule()->GetRamSize());
      LOG("finished loading ram");
    }

    delete[] memblock;
  } else {
    LOG("could not load ram file: %s", CurrentRom->SavePath.c_str());
  }
}

void SaveSettings() {
  std::ofstream outfile(saveFilePath, std::ios::trunc | std::ios::binary);
  outfile.write(reinterpret_cast<const char *>(&SAVE_FILE_VERSION), sizeof(int));

  outfile.write(reinterpret_cast<const char *>(&currentRomListSelection), sizeof(int));
  outfile.write(reinterpret_cast<const char *>(&selectedPalette), sizeof(int));
  outfile.write(reinterpret_cast<const char *>(&saveSlot), sizeof(int));
  outfile.write(reinterpret_cast<const char *>(&forceDMG), sizeof(bool));
  outfile.write(reinterpret_cast<const char *>(&LayerBuilder::screenPitch), sizeof(float));
  outfile.write(reinterpret_cast<const char *>(&LayerBuilder::screenYaw), sizeof(float));
  outfile.write(reinterpret_cast<const char *>(&LayerBuilder::screenRoll), sizeof(float));
  outfile.write(reinterpret_cast<const char *>(&LayerBuilder::radiusMenuScreen), sizeof(float));
  outfile.write(reinterpret_cast<const char *>(&LayerBuilder::screenSize), sizeof(float));
  outfile.write(reinterpret_cast<const char *>(&followHead), sizeof(bool));

  outfile.close();
  LOG("Saved Settings");
}

void LoadSettings() {
  std::ifstream file(saveFilePath, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    file.seekg(0, std::ios::beg);

    int saveFileVersion = 0;
    file.read((char *)&saveFileVersion, sizeof(int));

    // only load if the save versions are compatible
    if (saveFileVersion == SAVE_FILE_VERSION) {
      file.read((char *)&currentRomListSelection, sizeof(int));
      file.read((char *)&selectedPalette, sizeof(int));
      file.read((char *)&saveSlot, sizeof(int));
      file.read((char *)&forceDMG, sizeof(bool));
      file.read((char *)&LayerBuilder::screenPitch, sizeof(float));
      file.read((char *)&LayerBuilder::screenYaw, sizeof(float));
      file.read((char *)&LayerBuilder::screenRoll, sizeof(float));
      file.read((char *)&LayerBuilder::radiusMenuScreen, sizeof(float));
      file.read((char *)&LayerBuilder::screenSize, sizeof(float));
      file.read((char *)&followHead, sizeof(bool));
    }

    // TODO: reset all loaded settings
    if (file.fail())
      LOG("Failed Loading Settings");
    else
      LOG("Settings Loaded");

    file.close();
  }
}

void SaveStateImage(int slot) {
  std::string savePath = stateFolderPath + CurrentRom->RomName + ".stateimg";
  if (slot > 0) savePath += ('0' + slot);

  LOG("save image of slot to %s", savePath.c_str());
  std::ofstream outfile(savePath, std::ios::trunc | std::ios::binary);
  outfile.write((const char *)currentGame->saveStates[slot].saveImage,
                sizeof(GB_Color) * VIDEO_WIDTH * VIDEO_HEIGHT);
  outfile.close();
  LOG("finished writing save image to file");
}

bool LoadStateImage(int slot) {
  std::string savePath = stateFolderPath + CurrentRom->RomName + ".stateimg";
  if (slot > 0) savePath += ('0' + slot);

  std::ifstream file(savePath, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    u8 *memblock = new u8[sizeof(GB_Color) * VIDEO_WIDTH * VIDEO_HEIGHT];
    file.seekg(0, std::ios::beg);
    file.read((char *)memblock, sizeof(GB_Color) * VIDEO_WIDTH * VIDEO_HEIGHT);
    file.close();

    memcpy(currentGame->saveStates[slot].saveImage, memblock,
           sizeof(GB_Color) * VIDEO_WIDTH * VIDEO_HEIGHT);

    delete[] memblock;

    LOG("loaded image file: %s", savePath.c_str());

    return true;
  }

  LOG("could not load image file: %s", savePath.c_str());
  return false;
}

void SaveState(int slot) {
  std::string savePath = stateFolderPath + CurrentRom->RomName + ".state";
  if (slot > 0) savePath += ('0' + slot);

  // get the size of the savestate
  size_t size = 0;
  core->SaveState(NULL, size);

  if (size > 0) {
    u8 *memblock = new u8[size];

    if (core->SaveState(memblock, size)) {
      LOG("save slot to %s", savePath.c_str());
      std::ofstream outfile(savePath, std::ios::trunc | std::ios::binary);
      outfile.write((const char *)memblock, size);
      outfile.close();
      LOG("finished writing slot to file");
    }
  }

  LOG("copy image");
  memcpy(currentGame->saveStates[slot].saveImage, gearboy_frame_buf,
         sizeof(GB_Color) * VIDEO_WIDTH * VIDEO_HEIGHT);
  LOG("update image");
  UpdateSlotImage();
  // save image for the slot
  SaveStateImage(slot);
  currentGame->saveStates[slot].filled = true;
}

void LoadState(int slot) {
  std::string savePath = stateFolderPath + CurrentRom->RomName + ".state";
  if (slot > 0) savePath += ('0' + slot);

  std::ifstream file(savePath, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    long romBufferSize = file.tellg();
    u8 *memblock = new u8[romBufferSize];
    file.seekg(0, std::ios::beg);
    file.read((char *)memblock, romBufferSize);
    file.close();
    LOG("loaded slot has size: %ld", romBufferSize);

    core->LoadState(memblock, romBufferSize);

    delete[] memblock;
  } else {
    LOG("could not load ram file: %s", CurrentRom->SavePath.c_str());
  }
}

void LoadRom(std::string path) {
  std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    long romBufferSize = file.tellg();
    u8 *memblock = new u8[romBufferSize];
    file.seekg(0, std::ios::beg);
    file.read((char *)memblock, romBufferSize);
    file.close();
    LOG("loaded file %ld", romBufferSize);

    LOG("try loading rom in core");
    bool loadedCore = core->LoadROMFromBuffer(memblock, (int)romBufferSize, forceDMG);
    LOG("loaded rom: %i", loadedCore);

    delete[] memblock;
  } else {
    LOG("could not load rom file: %s", path.c_str());
  }
}

void LoadGame(Rom *rom) {
  LOG("save ram");
  SaveRam();
  LOG("load ram");

  CurrentRom = rom;

  LoadRom(rom->FullPath);

  LOG("try loading ram");
  LoadRam();
  LOG("end loading ram");

  for (int i = 0; i < 10; ++i) {
    if (!LoadStateImage(i)) {
      currentGame->saveStates[i].filled = false;

      memset(currentGame->saveStates[i].saveImage, 0,
             sizeof(GB_Color) * VIDEO_WIDTH * VIDEO_HEIGHT);
    } else {
      currentGame->saveStates[i].filled = true;
    }
  }

  UpdateSlotImage();
}

bool SortByRomName(const Rom &first, const Rom &second) { return first.RomName < second.RomName; }

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

          Rom newRom;
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

    std::sort(romFiles.begin(), romFiles.end(), SortByRomName);
  } else {
    LOG("could not open folder");
  }
}

void InitAudio() { OpenSLWrap_Init(); }

void OvrApp::EnteredVrMode(const ovrIntentType intentType, const char *intentFromPackage,
                           const char *intentJSON, const char *intentURI) {
  OVR_UNUSED(intentFromPackage);
  OVR_UNUSED(intentJSON);
  OVR_UNUSED(intentURI);

  if (intentType == INTENT_LAUNCH) {
    // TODO move me to the launch of the app
    InitAudio();

    InitDrawHelper(menuWidth, menuHeight);

    LoadSettings();

    CreateSlotImage();
    CreateWhiteImage();

    LOG("set upt palette");
    SetPalette(palettes[selectedPalette]);

    FontManager::Init(menuWidth, menuHeight);
    FontManager::LoadFont(&fontHeader, const_cast<char *>("/system/fonts/Roboto-Regular.ttf"), 55);
    FontManager::LoadFont(&fontMenu, const_cast<char *>("/system/fonts/Roboto-Light.ttf"), 22);
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

    ScanDirectory();

    SetUpScrollList();

    SetUpMenu();

    LOG("Create Screen");
    CreateScreen();

    const ovrJava *java = app->GetJava();
    SoundEffectContext = new ovrSoundEffectContext(*java->Env, java->ActivityObject);
    SoundEffectContext->Initialize(&app->GetFileSys());
    SoundEffectPlayer = new OvrGuiSys::ovrDummySoundEffectPlayer();

    Locale = ovrLocale::Create(*java->Env, java->ActivityObject, "default");

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

bool OvrApp::OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) {
  if (GuiSys->OnKeyEvent(keyCode, repeatCount, eventType)) {
    return true;
  }

  return false;
}

void UpdateCoreInput(const ovrFrameInput &vrFrame) {
  if (vrFrame.Input.buttonState & BUTTON_LSTICK_UP || vrFrame.Input.buttonState & BUTTON_DPAD_UP)
    core->KeyPressed(Up_Key);
  else
    core->KeyReleased(Up_Key);

  if (vrFrame.Input.buttonState & BUTTON_LSTICK_DOWN ||
      vrFrame.Input.buttonState & BUTTON_DPAD_DOWN)
    core->KeyPressed(Down_Key);
  else
    core->KeyReleased(Down_Key);

  if (vrFrame.Input.buttonState & BUTTON_LSTICK_LEFT ||
      vrFrame.Input.buttonState & BUTTON_DPAD_LEFT)
    core->KeyPressed(Left_Key);
  else
    core->KeyReleased(Left_Key);

  if (vrFrame.Input.buttonState & BUTTON_LSTICK_RIGHT ||
      vrFrame.Input.buttonState & BUTTON_DPAD_RIGHT)
    core->KeyPressed(Right_Key);
  else
    core->KeyReleased(Right_Key);

  if (vrFrame.Input.buttonState & BUTTON_A && !(lastButtonState & BUTTON_A))
    core->KeyPressed(A_Key);
  else if (!(vrFrame.Input.buttonState & BUTTON_A) && lastButtonState & BUTTON_A)
    core->KeyReleased(A_Key);

  if (vrFrame.Input.buttonState & BUTTON_B && !(lastButtonState & BUTTON_B))
    core->KeyPressed(B_Key);
  else if (!(vrFrame.Input.buttonState & BUTTON_B) && lastButtonState & BUTTON_B)
    core->KeyReleased(B_Key);

  if (vrFrame.Input.buttonState & BUTTON_START && !(lastButtonState & BUTTON_START))
    core->KeyPressed(Start_Key);
  else if (!(vrFrame.Input.buttonState & BUTTON_START) && lastButtonState & BUTTON_START)
    core->KeyReleased(Start_Key);

  if (vrFrame.Input.buttonState & BUTTON_SELECT && !(lastButtonState & BUTTON_SELECT))
    core->KeyPressed(Select_Key);
  else if (!(vrFrame.Input.buttonState & BUTTON_SELECT) && lastButtonState & BUTTON_SELECT)
    core->KeyReleased(Select_Key);
}

void AudioFrame(s16 *audio, unsigned sampleCount) {
  if (!audioInit) {
    audioInit = true;
    StartPlaying();
  }

  SetBuffer((unsigned short *)audio, sampleCount);
}

void MergeAudioBuffer() {
  int newLength = audio_sample_skip_count / 2 + audio_sample_count / 2;
  if (audio_sample_skip_count < audio_sample_count)
    newLength = audio_sample_skip_count;
  else
    newLength = audio_sample_count;

  LOG("audio size: %i, %i, %i", audio_sample_count, audio_sample_skip_count, newLength);

  /*
  for (int i = 0; i < audio_sample_count / 2; ++i) {
      audio_buf[i * 2] = audio_buf[i *
                                   4];// (s16) (((int)audio_buf[i * 4] +
  (int)audio_buf[i * 4 + 2]) / 2); audio_buf[i * 2 + 1] = audio_buf[i * 4 +
                                       1];//(s16) (((int)audio_buf[i * 4 + 1] +
  (int)audio_buf[i * 4 + 3]) / 2);
  }
  for (int i = 0; i < audio_sample_skip_count / 2; ++i) {
      audio_buf[audio_sample_count / 2 + i * 2] = audio_buf_skip[i *
                                                                 4];// (s16)
  (((int)audio_buf_skip[i * 4] + (int)audio_buf_skip[i * 4 + 2]) / 2);
      audio_buf[audio_sample_count / 2 + i * 2 + 1] = audio_buf_skip[i * 4 +
                                                                     1];//(s16)
  (((int)audio_buf_skip[i * 4 + 1] + (int)audio_buf_skip[i * 4 + 3]) / 2);
  }
  */

  int blockStart = 0;
  int blockSize = 160;
  for (int i = 0; i < newLength; ++i) {
    blockStart = (i / blockSize) * (blockSize * 2) + i % blockSize;
    audio_buf[i] = audio_buf[blockStart];
  }

  audio_sample_count = newLength;
}

void EmulatorFrame(const ovrFrameInput &vrFrame) {
  UpdateCoreInput(vrFrame);

  // update the emulator
  core->RunToVBlank(gearboy_frame_buf, audio_buf, &audio_sample_count);

  if (vrFrame.Input.buttonState & BUTTON_LEFT_TRIGGER) {
    /*
    // append the audio buffer
    audio_buf_skip = &audio_buf[audio_sample_count];
    core->RunToVBlank(gearboy_frame_buf, audio_buf_skip,
    &audio_sample_skip_count); MergeAudioBuffer();
    */
    core->RunToVBlank(gearboy_frame_buf, audio_buf, &audio_sample_count);
  }
  if (vrFrame.Input.buttonState & BUTTON_RIGHT_TRIGGER) {
    core->RunToVBlank(gearboy_frame_buf, audio_buf, &audio_sample_count);
    core->RunToVBlank(gearboy_frame_buf, audio_buf, &audio_sample_count);
  }

  if (audio_sample_count > 0) {
    // LOG("sample count: %i", audio_sample_count);
    AudioFrame(audio_buf, (unsigned int)audio_sample_count);
    // audio_batch_cb(audio_buf, audio_sample_count / 2);
  }

  audio_sample_count = 0;

  // update draw texture
  UpdateScreen();
}

void UpdateGUI(const ovrFrameInput &vrFrame) {
  if (!romSelection) {
    if (currentMenu == &moveMenu) {
      // update the position of the screen
      // LayerBuilder::MoveScreen(vrFrame);
    }

    if (vrFrame.Input.buttonState & BUTTON_LSTICK_UP ||
        vrFrame.Input.buttonState & BUTTON_DPAD_UP) {
      if ((!(lastButtonState & BUTTON_LSTICK_UP) && !(lastButtonState & BUTTON_DPAD_UP)) ||
          buttonDownCount > SCROLL_DELAY) {
        currentMenu->CurrentSelection--;
        buttonDownCount -= SCROLL_TIME;
      }
      // @TODO use time instead
      buttonDownCount++;
    } else if (vrFrame.Input.buttonState & BUTTON_LSTICK_DOWN ||
               vrFrame.Input.buttonState & BUTTON_DPAD_DOWN) {
      if ((!(lastButtonState & BUTTON_LSTICK_DOWN) && !(lastButtonState & BUTTON_DPAD_DOWN)) ||
          buttonDownCount > SCROLL_DELAY) {
        currentMenu->CurrentSelection++;
        buttonDownCount -= SCROLL_TIME;
      }
      buttonDownCount++;
    } else if (vrFrame.Input.buttonState & BUTTON_LSTICK_LEFT ||
               vrFrame.Input.buttonState & BUTTON_DPAD_LEFT) {
      if ((!(lastButtonState & BUTTON_LSTICK_LEFT) && !(lastButtonState & BUTTON_DPAD_LEFT)) ||
          buttonDownCount > SCROLL_DELAY_MOVE) {
        if (currentMenu->MenuItems[currentMenu->CurrentSelection].LeftFunction != nullptr)
          currentMenu->MenuItems[currentMenu->CurrentSelection].LeftFunction(
              &currentMenu->MenuItems[currentMenu->CurrentSelection]);

        buttonDownCount -= SCROLL_TIME_MOVE;
      }
      buttonDownCount++;
    } else if (vrFrame.Input.buttonState & BUTTON_LSTICK_RIGHT ||
               vrFrame.Input.buttonState & BUTTON_DPAD_RIGHT) {
      if ((!(lastButtonState & BUTTON_LSTICK_RIGHT) && !(lastButtonState & BUTTON_DPAD_RIGHT)) ||
          buttonDownCount > SCROLL_DELAY_MOVE) {
        if (currentMenu->MenuItems[currentMenu->CurrentSelection].RightFunction != nullptr)
          currentMenu->MenuItems[currentMenu->CurrentSelection].RightFunction(
              &currentMenu->MenuItems[currentMenu->CurrentSelection]);

        buttonDownCount -= SCROLL_TIME_MOVE;
      }
      buttonDownCount++;
    } else {
      buttonDownCount = 0;
    }

    if (currentMenu->CurrentSelection < 0)
      currentMenu->CurrentSelection = (int)(currentMenu->MenuItems.size() - 1);
    else if (currentMenu->CurrentSelection >= currentMenu->MenuItems.size())
      currentMenu->CurrentSelection = 0;

    if (vrFrame.Input.buttonState & BUTTON_A && !(lastButtonState & BUTTON_A) &&
        currentMenu->MenuItems[currentMenu->CurrentSelection].PressFunction != nullptr) {
      currentMenu->MenuItems[currentMenu->CurrentSelection].PressFunction(
          &currentMenu->MenuItems[currentMenu->CurrentSelection]);
    }
    if (vrFrame.Input.buttonState & BUTTON_B && !(lastButtonState & BUTTON_B) &&
        currentMenu->BackPress != nullptr) {
      currentMenu->BackPress();
    }

  } else {
    if (vrFrame.Input.buttonState & BUTTON_LSTICK_UP ||
        vrFrame.Input.buttonState & BUTTON_DPAD_UP) {
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
    if (vrFrame.Input.buttonState & BUTTON_A && !(lastButtonState & BUTTON_A)) {
      SaveSettings();
      LoadGame(&romFiles[currentRomListSelection]);
      menuOpen = false;
      romSelection = false;
      loadedRom = true;
    }
    // go back
    if (vrFrame.Input.buttonState & BUTTON_B && !(lastButtonState & BUTTON_B) && loadedRom) {
      romSelection = false;
    }
  }
}

void BeginTextureDrawing() {
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_DST_ALPHA, GL_ONE);

  glBlendEquation(GL_FUNC_ADD);
}

void EndTextureDrawing() {}

void DrawMenu() {
  // draw menu strings
  FontManager::Begin();
  for (uint i = 0; i < currentMenu->MenuItems.size(); i++)
    FontManager::RenderText(
        fontMenu, currentMenu->MenuItems[i].Text, listPosX + 37,
        HEADER_HEIGHT + 20 + menuItemSize * i, 1.0f,
        ((uint)currentMenu->CurrentSelection == i) ? menuSelectionColor : menuColor);
  FontManager::Close();

  BeginTextureDrawing();

  // draw the menu icons
  for (uint i = 0; i < currentMenu->MenuItems.size(); i++)
    if (currentMenu->MenuItems[i].iconId > 0)
      DrawTexture(currentMenu->MenuItems[i].iconId, listPosX + 10,
                  HEADER_HEIGHT + 20 + menuItemSize * i + menuItemSize / 2 - 10, 17, 21,
                  {1.0f, 1.0f, 1.0f, 1.0f});

  // save slot image
  if (currentMenu == &mainMenu) {
    if (currentGame->saveStates[saveSlot].filled) {
      DrawTexture(textureIdSlotImage, menuWidth - 320 - 20, HEADER_HEIGHT + 20, 160 * 2, 144 * 2,
                  {1.0f, 1.0f, 1.0f, 1.0f});
    } else {
      // state screenshot background
      DrawTexture(textureWhiteId, menuWidth - 320 - 20, HEADER_HEIGHT + 20, 320, 288,
                  {0.0f, 0.0f, 0.0f, 0.045f});

      // menuWidth - 320 - 20, HEADER_HEIGHT + 20, 320, 288
      FontManager::Begin();
      FontManager::RenderText(fontSlot, strNoSave, menuWidth - 160 - 20 - strNoSaveWidth / 2,
                              HEADER_HEIGHT + 20 + 144 - 26, 1.0f, Vector3f(0.95f, 0.95f, 0.95f));
      FontManager::Close();
    }
  } else if (currentMenu == &settingsMenu) {
    DrawTexture(textureIdScreen, menuWidth - 320 - 20, HEADER_HEIGHT + 20, 320, 144 * 2,
                {1.0f, 1.0f, 1.0f, 1.0f});
  }
}

void DrawRomList() {
  // calculate the slider position
  GLfloat recHeight = (float)scrollbarHeight * maxListItems / romFiles.size();

  GLfloat sliderPercentage = 0;
  if (romFiles.size() > maxListItems)
    sliderPercentage = (menuListState / (float)(romFiles.size() - maxListItems));
  else
    sliderPercentage = 0;

  GLfloat recPosY = (scrollbarHeight - recHeight) * sliderPercentage;

  // slider background
  DrawTexture(textureWhiteId, listPosX, listPosY, scrollbarWidth, scrollbarHeight,
              {0.15f, 0.15f, 0.15f, 0.05f});
  // slider
  DrawTexture(textureWhiteId, listPosX, listPosY + recPosY, scrollbarWidth, recHeight, sliderColor);

  // draw the cartridge icons
  for (uint i = (uint)menuListState; i < menuListState + maxListItems; i++) {
    if (i < romFiles.size()) {
      DrawTexture(romFiles[i].isGbc ? textureGbcIconId : textureGbIconId,
                  listPosX + scrollbarWidth + 15, listStartY + listItemSize * (i - menuListState),
                  21, 24, {1.0f, 1.0f, 1.0f, 1.0f});
    }
  }

  FontManager::Begin();
  // draw rom list
  for (uint i = (uint)menuListState; i < menuListState + maxListItems; i++) {
    if (i < romFiles.size()) {
      FontManager::RenderText(
          fontList, romFiles[i].RomName, listPosX + scrollbarWidth + 44,
          listStartY + listItemSize * (i - menuListState), 1.0f,
          ((uint)currentRomListSelection == i) ? menuSelectionColor : menuColor);
    } else
      break;
  }
  FontManager::Close();
}

void DrawGUI() {
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_DST_ALPHA, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  glBindFramebuffer(GL_FRAMEBUFFER, MenuFrameBuffer);
  // Render on the whole framebuffer, complete from the lower left corner to the
  // upper right
  glViewport(0, 0, menuWidth, menuHeight);

  glClearColor(MenuBackgroundColor.x, MenuBackgroundColor.y, MenuBackgroundColor.z,
               MenuBackgroundColor.w);
  glClear(GL_COLOR_BUFFER_BIT);

  // header
  DrawTexture(textureWhiteId, 0, 0, menuWidth, HEADER_HEIGHT, {0.0f, 0.0f, 0.0f, 0.045f});
  // icon
  DrawTexture(textureHeaderIconId, 5, 5, 65, 65, {0.9f, 0.9f, 0.9f, 0.9f});

  FontManager::Begin();
  FontManager::RenderText(fontHeader, strHeader, 70, 0.0f, 1.0f, headerColor);
  FontManager::RenderText(fontSmall, STR_VERSION, menuWidth - strVersionWidth - 5.0f,
                          HEADER_HEIGHT - 19, 1.0f, Vector3f(0.8f, 0.8f, 0.8f));
  FontManager::Close();

  if (romSelection)
    DrawRomList();
  else
    DrawMenu();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ovrFrameResult OvrApp::Frame(const ovrFrameInput &vrFrame) {
  if (vrFrame.Input.buttonState & BUTTON_X && !(lastButtonState & BUTTON_X)) {
    menuOpen = !menuOpen;
  }

  if (!menuOpen) {
    EmulatorFrame(vrFrame);
  } else {
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
  res.Layers[res.LayerCount++].Cylinder = LayerBuilder::BuildCylinderLayer(
      CylinderSwapChain, CylinderWidth, CylinderHeight, &vrFrame.Tracking, followHead);

  res.Layers[res.LayerCount].Cylinder.Header.Flags |=
      VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

  if (menuOpen) {
    // menu layer
    DrawGUI();

    res.Layers[res.LayerCount++].Cylinder = LayerBuilder::BuildSettingsCylinderLayer(
        MenuSwapChain, menuWidth, menuHeight, &vrFrame.Tracking, followHead);

    res.Layers[res.LayerCount].Cylinder.Header.Flags |=
        VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;
  }

  lastButtonState = vrFrame.Input.buttonState;

  return res;
}

void OnClickResumGame(MenuItem *item) {
  LOG("Pressed RESUME GAME");
  menuOpen = false;
}

void OnClickResetGame(MenuItem *item) {
  LOG("RESET GAME");
  core->ResetROMPreservingRAM(forceDMG);
  menuOpen = false;
}

void OnClickSaveGame(MenuItem *item) {
  LOG("on click save game");
  SaveState(saveSlot);
  menuOpen = false;
}

void OnClickLoadGame(MenuItem *item) {
  LoadState(saveSlot);
  menuOpen = false;
}

void OnClickLoadRomGame(MenuItem *item) { romSelection = true; }

void OnClickSettingsGame(MenuItem *item) { currentMenu = &settingsMenu; }

void OnClickChangePaletteLeft(MenuItem *item) {
  selectedPalette--;
  if (selectedPalette < 0) {
    selectedPalette = paletteCount - 1;
  }
  SetPalette(palettes[selectedPalette]);
  item->Text = "Palette: " + to_string(selectedPalette);
  SaveSettings();
}

void OnClickChangePaletteRight(MenuItem *item) {
  selectedPalette++;
  if (selectedPalette >= paletteCount) {
    selectedPalette = 0;
  }
  SetPalette(palettes[selectedPalette]);
  item->Text = "Palette: " + to_string(selectedPalette);
  SaveSettings();
}

void OnClickEmulatedModel(MenuItem *item) {
  forceDMG = !forceDMG;
  item->Text = strForceDMG[forceDMG ? 0 : 1];
  SaveSettings();
}

void OnClickFollowMode(MenuItem *item) {
  followHead = !followHead;
  item->Text = strMove[followHead ? 0 : 1];
  SaveSettings();
}

void OnClickMoveScreen(MenuItem *item) { currentMenu = &moveMenu; }

void OnClickAllowLeftRight(MenuItem *item) {
  allowUpDownSlashLeftRight = !allowUpDownSlashLeftRight;
  item->Text = "Allow Up+Down / Left+Right: ";
  item->Text += (allowUpDownSlashLeftRight ? "Enabled" : "Disabled");
}

void OnClickBackAndSave(MenuItem *item) {
  currentMenu = &mainMenu;
  SaveSettings();
}

void OnBackPressedSettings() {
  currentMenu = &mainMenu;
  SaveSettings();
}

void OnClickBackMove(MenuItem *item) {
  currentMenu = &settingsMenu;
  SaveSettings();
}

void OnBackPressedMove() {
  currentMenu = &settingsMenu;
  SaveSettings();
}

void OnClickSaveSlotLeft(MenuItem *item) {
  saveSlot--;
  if (saveSlot < 0) saveSlot = MAX_SAVESLOTS - 1;
  item->Text = "Save Slot: " + to_string(saveSlot);
  UpdateSlotImage();
  SaveSettings();
}

void OnClickSaveSlotRight(MenuItem *item) {
  saveSlot++;
  if (saveSlot >= MAX_SAVESLOTS) saveSlot = 0;
  item->Text = "Save Slot: " + to_string(saveSlot);
  UpdateSlotImage();
  SaveSettings();
}

float ToDegree(float radian) { return (int)(180.0 / VRAPI_PI * radian * 10) / 10.0f; }

void MoveYaw(MenuItem *item, float dir) {
  LayerBuilder::screenYaw -= dir;
  item->Text = "Yaw: " + to_string(ToDegree(LayerBuilder::screenYaw));
}

void MovePitch(MenuItem *item, float dir) {
  LayerBuilder::screenPitch -= dir;
  item->Text = "Pitch: " + to_string(ToDegree(LayerBuilder::screenPitch));
}

void MoveRoll(MenuItem *item, float dir) {
  LayerBuilder::screenRoll -= dir;
  item->Text = "Roll: " + to_string(ToDegree(LayerBuilder::screenRoll));
}

void ChangeDistance(MenuItem *item, float dir) {
  LayerBuilder::radiusMenuScreen -= dir;

  if (LayerBuilder::radiusMenuScreen < MIN_RADIUS) LayerBuilder::radiusMenuScreen = MIN_RADIUS;
  if (LayerBuilder::radiusMenuScreen > MAX_RADIUS) LayerBuilder::radiusMenuScreen = MAX_RADIUS;

  item->Text = "Distance: " + to_string(LayerBuilder::radiusMenuScreen);
}

void ChangeScale(MenuItem *item, float dir) {
  LayerBuilder::screenSize -= dir;

  if (LayerBuilder::screenSize < 0.5f) LayerBuilder::screenSize = 0.5f;
  if (LayerBuilder::screenSize > 2.0f) LayerBuilder::screenSize = 2.0f;

  item->Text = "Scale: " + to_string(LayerBuilder::screenSize);
}

void OnClickResetView(MenuItem *item) {
  LayerBuilder::ResetValues();

  // @HACK: updates the visible values
  MoveYaw(&moveMenu.MenuItems[0], 0);
  MovePitch(&moveMenu.MenuItems[1], 0);
  MoveRoll(&moveMenu.MenuItems[2], 0);
  ChangeDistance(&moveMenu.MenuItems[3], 0);
  ChangeScale(&moveMenu.MenuItems[4], 0);

  SaveSettings();
}

void OnClickMoveScreenYawLeft(MenuItem *item) { MoveYaw(item, MoveSpeed); }

void OnClickMoveScreenYawRight(MenuItem *item) { MoveYaw(item, -MoveSpeed); }

void OnClickMoveScreenPitchLeft(MenuItem *item) { MovePitch(item, -MoveSpeed); }

void OnClickMoveScreenPitchRight(MenuItem *item) { MovePitch(item, MoveSpeed); }

void OnClickMoveScreenRollLeft(MenuItem *item) { MoveRoll(item, -MoveSpeed); }

void OnClickMoveScreenRollRight(MenuItem *item) { MoveRoll(item, MoveSpeed); }

void OnClickMoveScreenDistanceLeft(MenuItem *item) { ChangeDistance(item, ZoomSpeed); }

void OnClickMoveScreenDistanceRight(MenuItem *item) { ChangeDistance(item, -ZoomSpeed); }

void OnClickMoveScreenScaleLeft(MenuItem *item) { ChangeScale(item, MoveSpeed); }

void OnClickMoveScreenScaleRight(MenuItem *item) { ChangeScale(item, -MoveSpeed); }

void OvrApp::SetUpMenu() {
  std::string strSaveSlot = "Save Slot: " + to_string(saveSlot);

  mainMenu.MenuItems.push_back(
      {textureSaveIconId, "Resume Game", OnClickResumGame, nullptr, nullptr});
  mainMenu.MenuItems.push_back(
      {textureSaveIconId, "Reset Game", OnClickResetGame, nullptr, nullptr});
  mainMenu.MenuItems.push_back({textureSaveIconId, strSaveSlot, OnClickSaveSlotRight,
                                OnClickSaveSlotLeft, OnClickSaveSlotRight});
  mainMenu.MenuItems.push_back({textureSaveIconId, "Save", OnClickSaveGame, nullptr, nullptr});
  mainMenu.MenuItems.push_back({textureLoadIconId, "Load", OnClickLoadGame, nullptr, nullptr});
  mainMenu.MenuItems.push_back(
      {textureLoadIconId, "Load Rom", OnClickLoadRomGame, nullptr, nullptr});
  mainMenu.MenuItems.push_back(
      {textureLoadIconId, "Settings", OnClickSettingsGame, nullptr, nullptr});

  std::string strPalette = "Palette: " + to_string(selectedPalette);

  settingsMenu.MenuItems.push_back({0, strPalette, OnClickChangePaletteRight,
                                    OnClickChangePaletteLeft, OnClickChangePaletteRight});
  settingsMenu.MenuItems.push_back({0, strForceDMG[forceDMG ? 0 : 1], OnClickEmulatedModel,
                                    OnClickEmulatedModel, OnClickEmulatedModel});

  settingsMenu.MenuItems.push_back(
      {0, strMove[followHead ? 0 : 1], OnClickFollowMode, OnClickFollowMode, OnClickFollowMode});
  settingsMenu.MenuItems.push_back({0, "Move Screen", OnClickMoveScreen, nullptr, nullptr});
  settingsMenu.MenuItems.push_back({0, "Back", OnClickBackAndSave, nullptr, nullptr});
  settingsMenu.BackPress = OnBackPressedSettings;

  // move menu stuff
  moveMenu.MenuItems.push_back(
      {0, "", nullptr, OnClickMoveScreenYawLeft, OnClickMoveScreenYawRight});
  moveMenu.MenuItems.push_back(
      {0, "", nullptr, OnClickMoveScreenPitchLeft, OnClickMoveScreenPitchRight});
  moveMenu.MenuItems.push_back(
      {0, "", nullptr, OnClickMoveScreenRollLeft, OnClickMoveScreenRollRight});
  moveMenu.MenuItems.push_back(
      {0, "", nullptr, OnClickMoveScreenDistanceLeft, OnClickMoveScreenDistanceRight});
  moveMenu.MenuItems.push_back(
      {0, "", nullptr, OnClickMoveScreenScaleLeft, OnClickMoveScreenScaleRight});
  moveMenu.MenuItems.push_back({0, "Reset View", OnClickResetView, nullptr, nullptr});
  moveMenu.MenuItems.push_back({0, "Back", OnClickBackMove, nullptr, nullptr});
  moveMenu.BackPress = OnBackPressedMove;

  // @HACK: updates the visible values
  MoveYaw(&moveMenu.MenuItems[0], 0);
  MovePitch(&moveMenu.MenuItems[1], 0);
  MoveRoll(&moveMenu.MenuItems[2], 0);
  ChangeDistance(&moveMenu.MenuItems[3], 0);
  ChangeScale(&moveMenu.MenuItems[4], 0);

  strVersionWidth = GetWidth(fontSmall, STR_VERSION);
  strMoveMenuTextWidth = GetWidth(fontMenu, strMoveMenu);
  strNoSaveWidth = GetWidth(fontSlot, strNoSave);

  currentMenu = &mainMenu;
}