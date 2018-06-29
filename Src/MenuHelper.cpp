#include "MenuHelper.h"

int SelectButton = BUTTON_A;
int BackButton = BUTTON_B;
bool SwappSelectBackButton = false;

ovrVector4f textColor = {0.9f, 0.9f, 0.9f, 0.9f};
ovrVector4f textSelectionColor = {0.15f, 0.8f, 0.6f, 0.8f};

void MenuLabel::DrawText(float offsetX, float transparency) {
  if (Visible)
    FontManager::RenderText(*Font, Text, PosX + offsetX + (Selected ? 5 : 0), PosY, 1.0f, Color,
                            transparency);
}

void MenuImage::DrawTexture(float offsetX, float transparency) {
  if (Visible)
    DrawHelper::DrawTexture(ImageId, PosX + offsetX + (Selected ? 5 : 0), PosY, Width, Height,
                            Color, transparency);
}

void MenuButton::DrawText(float offsetX, float transparency) {
  if (Visible)
    FontManager::RenderText(*Font, Text, PosX + 33 + (Selected ? 5 : 0) + offsetX, PosY, 1.0f,
                            Selected ? textSelectionColor : textColor, transparency);
}

void MenuButton::DrawTexture(float offsetX, float transparency) {
  if (IconId > 0 && Visible)
    DrawHelper::DrawTexture(IconId, PosX + (Selected ? 5 : 0) + offsetX,
                            PosY + Font->FontSize / 2 - 14, 28, 28,
                            Selected ? textSelectionColor : textColor, transparency);
}

void MenuContainer::DrawText(float offsetX, float transparency) {
  for (int i = 0; i < MenuItems.size(); ++i) {
    MenuItems.at(i)->DrawText(offsetX, transparency);
  }
}

void MenuContainer::DrawTexture(float offsetX, float transparency) {
  for (int i = 0; i < MenuItems.size(); ++i) {
    MenuItems.at(i)->DrawTexture(offsetX, transparency);
  }
}

void MenuList::DrawText(float offsetX, float transparency) {
  // draw rom list
  for (uint i = (uint)menuListState; i < menuListState + maxListItems; i++) {
    if (i < RomList->size()) {
      FontManager::RenderText(
          *Font, RomList->at(i).RomName,
          PosX + offsetX + scrollbarWidth + 44 + (((uint)CurrentSelection == i) ? 5 : 0),
          listStartY + itemOffsetY + listItemSize * (i - menuListState), 1.0f,
          ((uint)CurrentSelection == i) ? textSelectionColor : textColor, transparency);
    } else
      break;
  }
}