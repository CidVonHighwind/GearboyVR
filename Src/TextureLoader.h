#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include "KingInclude.h"

namespace TextureLoader {

GLuint CreateWhiteTexture();

GLuint Load(App *app, const char *path);

}  // namespace TextureLoader

#endif