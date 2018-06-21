using namespace OVR;

namespace FontManager {

struct Character {
  glm::fvec4 Position;  // the position of the character inside of the texture
  glm::ivec2 Size;      // Size of glyph
  glm::ivec2 Bearing;   // Offset from baseline to left/top of glyph
  GLuint Advance;       // Offset to advance to next glyph
};

struct RenderFont {
  int FontSize;
  GLuint textureID;
  std::map<GLchar, Character> Characters;
};

void Init(GLfloat menuWidth, GLfloat menuHeight);

void CloseFontLoader();

int GetWidth(RenderFont font, std::string text);

void LoadFont(RenderFont *font, char *filePath, uint fontSize);

void Begin();

void RenderText(RenderFont font, std::string text, GLfloat x, GLfloat y, GLfloat scale,
                Vector3f color);

void Close();

}  // namespace FontManager