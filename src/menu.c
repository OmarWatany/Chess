#include "chess.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Custom button colors
const Color Yellow = {249, 195, 40, 255};
const Color Red = {238, 35, 39, 255};
const Color Cyan = {127, 209, 218, 255}; // B0EFF5
const Color textColor = BLACK;           // Black color for text

#define NUM_OPTIONS 5

int gui_menu() {
    char *menuOptions[NUM_OPTIONS] = {
        "START GAME",
        "HOST",
        "CLIENT",
        "QUIT GAME",
    }; // Menu options

    Rectangle buttons[NUM_OPTIONS]; // Button rectangles

    // Buttons positions and sizes
    const float buttonWidth = 300;
    const float buttonHeight = 60;
    const float buttonSpacing = 20;
    const float startX = (GetScreenWidth() - buttonWidth) / 2;
    const float startY = (GetScreenWidth() + INFOBAR_HEIGHT - (4 * buttonHeight + 3 * buttonSpacing)) / 2;

    // Set positions for each button
    buttons[0] = (Rectangle){startX, startY + (buttonHeight + buttonSpacing) * 0, buttonWidth, buttonHeight};
    buttons[1] = (Rectangle){startX, startY + (buttonHeight + buttonSpacing) * 1, buttonWidth, buttonHeight};
    buttons[2] = (Rectangle){startX, startY + (buttonHeight + buttonSpacing) * 2, buttonWidth, buttonHeight};
    buttons[3] = (Rectangle){startX, startY + (buttonHeight + buttonSpacing) * 3, buttonWidth, buttonHeight};

    // Variable to track the time of the last button press
    double lastButtonClickTime = 0.0;
    // Minimum time between button presses (in seconds)
    const double buttonCooldown = 0.5; // Adjust as needed
    // Set default button colors before the loop
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(textColor));
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(textColor));
    GuiSetStyle(BUTTON, TEXT_SIZE, 60);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Yellow));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(Red));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(textColor));
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(textColor));
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
    // Main menu loop
    int r = -1;
    bool running = true;
    while (running) {
        ClearBackground(WHITE);
        BeginDrawing();
        for (int i = 0; i < NUM_OPTIONS && running; ++i) {
            if (GuiButton(buttons[i], menuOptions[i])) {
                r = i;
                running = false;
                // Debounce button click for other buttons
                if (GetTime() - lastButtonClickTime >= buttonCooldown) {
                    lastButtonClickTime = GetTime(); // Update last button click time
                }
            }
        }
        EndDrawing();
    }
    return r;
}
