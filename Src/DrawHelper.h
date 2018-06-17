#include <OVR_FileSys.h>
#include <VrApi_Types.h>
#include <OVR_Input.h>
#include <VrApi_Input.h>
#include <iostream>
#include <fstream>
#include <map>

#include <dirent.h>
#include <VRMenu.h>
#include <algorithm>
#include "OvrApp.h"
#include "GuiSys.h"
#include "OVR_Locale.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifndef ANDROID_DRAWHELPER_H
#define ANDROID_DRAWHELPER_H

#endif //ANDROID_DRAWHELPER_H

void InitDrawHelper(GLfloat menuWidth, GLfloat menuHeight);

void DrawRectangle(GLfloat posX, GLfloat posY, GLfloat width, GLfloat height, ovrVector4f color);

void DrawTexture(GLuint textureId,
                 GLfloat posX, GLfloat posY, GLfloat width, GLfloat height, ovrVector4f color);