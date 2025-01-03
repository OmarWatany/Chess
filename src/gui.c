#include "gui.h"
#include "chess.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chess.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Custom button colors
const Color Yellow = {249, 195, 40, 255};
const Color Red = {238, 35, 39, 255};
const Color Cyan = {127, 209, 218, 255}; // B0EFF5
const Color textColor = BLACK;           // Black color for text

int menu(char *menuOptions[], size_t sz) {
    Rectangle buttons[sz]; // Button rectangles
    // Buttons positions and sizes
    const float buttonWidth = 300;
    const float buttonHeight = 60;
    const float buttonSpacing = 20;
    const float startX = (GetScreenWidth() - buttonWidth) / 2;
    const float startY = (GetScreenWidth() + INFOBAR_HEIGHT - (sz * buttonHeight + 3 * buttonSpacing)) / 2;

    // Set positions for each button
    for (size_t i = 0; i < sz; i++)
        buttons[i] = (Rectangle){startX, startY + (buttonHeight + buttonSpacing) * i, buttonWidth, buttonHeight};

    // Set default button colors before the loop
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(textColor));
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(textColor));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(textColor));
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(textColor));
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Yellow));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(Red));
    GuiSetStyle(BUTTON, TEXT_SIZE, 60);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);

    bool supportedBottons[sz];
    for (int i = 0; i < sz; i++)
        supportedBottons[i] = true;
    supportedBottons[1] = false;

    // Main menu loop
    while (true) {
        ClearBackground(WHITE);
        BeginDrawing();
        for (int i = 0; i < sz; ++i)
            if (GuiButton(buttons[i], menuOptions[i]))
                if (supportedBottons[i]) return i;
        EndDrawing();
    }
    return -1;
}
