#include "DrawHelper.h"
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

using namespace OVR;

GlProgram glRectangleProgram, glTextureProgram;

static const char RECTANGLE_VERTEX_SHADER[] =
    "layout (location = 0) in vec2 position;\n"

    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = projection * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const char RECTANGLE_FRAGMENT_SHADER[] =
    "uniform vec4 textColor;\n"
    "out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "	outColor = textColor;//vec4(textColor.xyz, 0.5);\n"
    "}\n";

static const char VERTEX_SHADER_TEXTURE[] =
    "#version 330 core\n"
    "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"

    "out vec2 TexCoords;\n"

    "uniform mat4 projection;\n"

    "void main()\n"
    "{\n"
    "	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
    "	TexCoords = vertex.zw;\n"
    "}\n";

static const char FRAGMENT_SHADER_TEXTURE[] =
    "#version 330 core\n"
    "in vec2 TexCoords;\n"

    "out vec4 color;\n"

    "uniform sampler2D text;\n"
    //"uniform vec3 textColor;\n"

    "void main()\n"
    "{\n"
    "	color = texture(text, TexCoords);\n"
    "}\n";

GLuint vao;
GLuint vbo;
static GLfloat vertices[] = {20.0f, 20.0f, 20.0f, 120.0f, 40.0f, 20.0f, 40.0f, 120.0f};

GLuint texture_vao;
GLuint texture_vbo;

void InitDrawHelper(GLfloat menuWidth, GLfloat menuHeight) {
  glRectangleProgram =
      GlProgram::Build(RECTANGLE_VERTEX_SHADER, RECTANGLE_FRAGMENT_SHADER, NULL, 0);

  glTextureProgram = GlProgram::Build(VERTEX_SHADER_TEXTURE, FRAGMENT_SHADER_TEXTURE, NULL, 0);

  glUseProgram(glRectangleProgram.Program);
  glm::mat4 projection = glm::ortho(0.0f, menuWidth, 0.0f, menuHeight);
  glUniformMatrix4fv(glGetUniformLocation(glRectangleProgram.Program, "projection"), 1, GL_FALSE,
                     glm::value_ptr(projection));

  glUseProgram(glTextureProgram.Program);
  glUniformMatrix4fv(glGetUniformLocation(glRectangleProgram.Program, "projection"), 1, GL_FALSE,
                     glm::value_ptr(projection));

  // Create Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  // Create a Vertex Buffer Object and copy the vertex data to it
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
  // Specify the layout of the vertex data
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // Create Vertex Array Object
  glGenVertexArrays(1, &texture_vao);
  glBindVertexArray(texture_vao);
  // Create a Vertex Buffer Object and copy the vertex data to it
  glGenBuffers(1, &texture_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, texture_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  // Specify the layout of the vertex data
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
}

void DrawRectangle(GLfloat posX, GLfloat posY, GLfloat width, GLfloat height, ovrVector4f color) {
  glUseProgram(glRectangleProgram.Program);
  glBindVertexArray(vao);

  GLfloat vert[16] = {posX,         posY, posX,         posY + height,
                      posX + width, posY, posX + width, posY + height};

  memcpy(&vertices, &vert, sizeof(vert));

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUniform4f(glGetUniformLocation(glRectangleProgram.Program, "textColor"), color.x, color.y,
              color.z, color.w);

  // Draw a triangle from the 3 vertices
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void DrawTexture(GLuint textureId, GLfloat posX, GLfloat posY, GLfloat width, GLfloat height,
                 ovrVector4f color) {
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(texture_vao);

  glUseProgram(glTextureProgram.Program);
  glBindTexture(GL_TEXTURE_2D, textureId);

  GLfloat charVertices[6][4] = {
      {posX, posY + height, 0.0f, 1.0f},        {posX, posY, 0.0f, 0.0f},
      {posX + width, posY, 1.0f, 0.0f},

      {posX, posY + height, 0.0f, 1.0f},        {posX + width, posY, 1.0f, 0.0f},
      {posX + width, posY + height, 1.0f, 1.0f}};

  glBindBuffer(GL_ARRAY_BUFFER, texture_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(charVertices), charVertices);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Draw a triangle from the 3 vertices
  glDrawArrays(GL_TRIANGLES, 0, 6);
}