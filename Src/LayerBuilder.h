#include "OVR_Input.h"

using namespace OVR;

namespace LayerBuilder {

    extern float screenYaw, screenPitch, screenRoll, radiusMenuScreen, screenSize;

    void MoveScreen(const ovrFrameInput &vrFrame);

    void UpdateDirection(const ovrFrameInput &vrFrame);

    void ResetValues();

    ovrLayerCylinder2 BuildCylinderLayer(ovrTextureSwapChain *cylinderSwapChain,
                                         const int textureWidth, const int textureHeight,
                                         const ovrTracking2 *tracking, bool followHead);

    ovrLayerCylinder2 BuildSettingsCylinderLayer(ovrTextureSwapChain *cylinderSwapChain,
                                                 const int textureWidth, const int textureHeight,
                                                 const ovrTracking2 *tracking, bool followHead);
}