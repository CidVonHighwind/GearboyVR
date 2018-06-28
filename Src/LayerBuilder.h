#include "KingInclude.h"

using namespace OVR;

namespace LayerBuilder {

extern float screenPitch, screenYaw, screenRoll, radiusMenuScreen, screenSize;

void UpdateDirection(const ovrFrameInput &vrFrame);

void ResetValues();

ovrLayerCylinder2 BuildGameCylinderLayer(ovrTextureSwapChain *cylinderSwapChain,
                                         const int textureWidth,
                                         const int textureHeight,
                                         const ovrTracking2 *tracking,
                                         bool followHead);

ovrLayerCylinder2 BuildSettingsCylinderLayer(ovrTextureSwapChain *cylinderSwapChain,
                                             const int textureWidth, const int textureHeight,
                                             const ovrTracking2 *tracking, bool followHead, float offsetY);
}  // namespace LayerBuilder