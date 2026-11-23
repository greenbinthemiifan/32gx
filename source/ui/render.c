#include "render.h"
#include "browser.h"
#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>

static GXRModeObj *rmode = NULL;
static void *frameBuffer = NULL;

void render_init(void) {
    rmode = VIDEO_GetPreferredMode(NULL);
    
    // Allocate dual framebuffers for smooth double-buffering
    frameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

void render_frame(int selectedIndex) {
    // 1. Clear screen / set up GX drawing context
    GX_SetCopyClear((GXColor){ 30, 30, 40, 255 }, 0x00ffffff); // Dark modern background
    GX_CopyDisp(frameBuffer, GX_TRUE);
    GX_DrawDone();

    // 2. Grid layout configuration
    int startX = 60;
    int startY = 80;
    int cardWidth = 130;
    int cardHeight = 110;
    int gapX = 25;
    int gapY = 25;
    int columns = 4;

    // 3. Loop through items and draw their cards
    for (int i = 0; i < itemCount; i++) {
        int col = i % columns;
        int row = i / columns;
        
        int x = startX + col * (cardWidth + gapX);
        int y = startY + row * (cardHeight + gapY);

        bool isSelected = (i == selectedIndex);

        // TODO: Use GX primitives or bitmap font rendering to draw:
        // - A rounded card rectangle (lighter grey/white if selected, dark container otherwise)
        // - A folder icon or ROM file icon inside the card
        // - The truncated item name label at the bottom of the card
    }

    // Swap buffers to display the rendered frame
    VIDEO_SetNextFramebuffer(frameBuffer);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}
