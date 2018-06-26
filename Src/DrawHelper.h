#ifndef DRAW_HELPER_H
#define DRAW_HELPER_H

#include "KingInclude.h"

namespace DrawHelper {

void Init(GLfloat menuWidth, GLfloat menuHeight);

void DrawTexture(GLuint textureId, GLfloat posX, GLfloat posY, GLfloat width, GLfloat height,
                 ovrVector4f color, float transparency);

}  // namespace DrawHelper

#endif