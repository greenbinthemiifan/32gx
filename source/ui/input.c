#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <gccore.h>

void input_init(void) {
    // WPAD_Init() is usually called in main.c, but safe to ensure here
    WPAD_Init();
}

// Updates the cursor position and checks for button actions
// Returns 1 if the user pressed 'A' to select an item, otherwise 0.
int input_update(int *selectedIndex, int maxItems, int columns) {
    WPAD_ScanPads();
    u32 pressed = WPAD_ButtonsDown(0); // Read Player 1 controller

    // Handle D-Pad Navigation
    if (pressed & WPAD_BUTTON_RIGHT) {
        if (*selectedIndex + 1 < maxItems) {
            (*selectedIndex)++;
        }
    }
    if (pressed & WPAD_BUTTON_LEFT) {
        if (*selectedIndex > 0) {
            (*selectedIndex)--;
        }
    }
    if (pressed & WPAD_BUTTON_DOWN) {
        if (*selectedIndex + columns < maxItems) {
            (*selectedIndex) += columns;
        }
    }
    if (pressed & WPAD_BUTTON_UP) {
        if (*selectedIndex - columns >= 0) {
            (*selectedIndex) -= columns;
        }
    }

    // Bounds safety check
    if (*selectedIndex < 0) *selectedIndex = 0;
    if (*selectedIndex >= maxItems && maxItems > 0) *selectedIndex = maxItems - 1;

    // Actions
    if (pressed & WPAD_BUTTON_A) {
        return 1; // Selection confirmed (Open folder or load ROM)
    }

    if (pressed & WPAD_BUTTON_HOME) {
        exit(0); // Exit back to Homebrew Channel
    }

    return 0;
}
