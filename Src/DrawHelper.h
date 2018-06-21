#include <OVR_FileSys.h>
#include <OVR_Input.h>
#include <VrApi_Input.h>
#include <VrApi_Types.h>
#include <fstream>
#include <iostream>
#include <map>

#include <VRMenu.h>
#include <dirent.h>
#include <algorithm>
#include "GuiSys.h"
#include "OVR_Locale.h"
#include "OvrApp.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace DrawHelper {

void Init(GLfloat menuWidth, GLfloat menuHeight);

void DrawTexture(GLuint textureId, GLfloat posX, GLfloat posY, GLfloat width, GLfloat height,
                 ovrVector4f color);

}  // namespace DrawHelper