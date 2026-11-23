#include <gccore.h>
#include <asndlib.h>
#include <fat.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ui/browser.h"
#include "ui/render.h"
#include "ui/input.h"
#include "../core/core_bridge.h"

typedef enum {
    APP_STATE_BROWSER,
    APP_STATE_EMULATING
} AppState;

int main(int argc, char **argv) {
    // 1. Initialize Wii hardware subsystems
    VIDEO_Init();
    ASND_Init();
    AUDIO_SetDSPSpeakerMode(AUDIO_SPEAKER_STEREO);
    fatInitDefault();
    input_init();
    render_init();

    // 2. Initialize file browser pointing to SD card ROM directory
    browser_init("sd:/roms/32x");

    AppState state = APP_STATE_BROWSER;

    // 3. Main 60 FPS Event Loop
    while (1) {
        input_update();
        UIAction action = input_get_action();

        if (state == APP_STATE_BROWSER) {
            // Exit back to Homebrew Channel if requested
            if (action == UI_ACTION_EXIT) break;

            // Update browser selection/navigation based on controller input
            const char *selected_rom = browser_update(action);
            if (selected_rom != NULL) {
                // User pressed A/Select on a ROM file; boot the emulation core
                if (core_init(selected_rom)) {
                    state = APP_STATE_EMULATING;
                }
            }

            // Render the card grid browser using your render_frame function
            render_begin_frame();
            render_frame(browser_get_selected_index());
            render_end_frame();
        } 
        else if (state == APP_STATE_EMULATING) {
            // Press Home / Start to exit back to the ROM browser
            if (action == UI_ACTION_EXIT) {
                core_shutdown();
                state = APP_STATE_BROWSER;
                continue;
            }

            // Run one frame of PicoDrive emulation and audio streaming
            core_run_frame();

            // Render the emulator's active video texture full-screen
            int w, h;
            void *tex = core_get_gx_texture(&w, &h);
            
            render_begin_frame();
            render_emulator_frame(tex, w, h);
            render_end_frame();
        }
    }

    // 4. Safe Cleanup on Exit
    if (state == APP_STATE_EMULATING) {
        core_shutdown();
    }
    return 0;
}
