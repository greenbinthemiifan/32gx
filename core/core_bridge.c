#include "core_bridge.h"
#include "libretro/libretro.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

// 32-byte aligned buffer required by Wii GX hardware
static u16 *gx_tex_buffer = NULL;
static int current_width = 320;
static int current_height = 224;

// Helper: Converts linear RGB565 pixels into Wii GX 4x4 tiled RGB565 layout
static void tile_rgb565_to_gx(const u16 *src, u16 *dst, int width, int height, int pitch_pixels) {
    for (int y = 0; y < height; y += 4) {
        for (int x = 0; x < width; x += 4) {
            for (int row = 0; row < 4; row++) {
                const u16 *src_row = src + (y + row) * pitch_pixels + x;
                for (int col = 0; col < 4; col++) {
                    *dst++ = src_row[col];
                }
            }
        }
    }
}

// Libretro Video Refresh Callback (Called every frame by PicoDrive)
static void video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data || !gx_tex_buffer) return;

    current_width = width;
    current_height = height;
    int pitch_pixels = pitch / sizeof(u16);

    // Tile the linear framebuffer into GX memory
    tile_rgb565_to_gx((const u16 *)data, gx_tex_buffer, width, height, pitch_pixels);

    // Crucial: Flush CPU cache so the Wii GPU (GX) reads fresh pixels
    DCFlushRange(gx_tex_buffer, CORE_MAX_WIDTH * CORE_MAX_HEIGHT * sizeof(u16));
}

// Required stub callbacks for Libretro
static void env_cb(unsigned cmd, void *data) {
    if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) {
        enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;
        *fmt = RETRO_PIXEL_FORMAT_RGB565;
    }
}
static void input_poll_cb(void) {}
static int16_t input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id) { return 0; }
static void audio_sample_cb(int16_t left, int16_t right) {}
static size_t audio_sample_batch_cb(const int16_t *data, size_t frames) { return frames; }

bool core_init(const char *rom_path) {
    // Allocate 32-byte aligned memory for GX texture
    size_t buf_size = CORE_MAX_WIDTH * CORE_MAX_HEIGHT * sizeof(u16);
    gx_tex_buffer = (u16 *)memalign(32, buf_size);
    if (!gx_tex_buffer) return false;
    memset(gx_tex_buffer, 0, buf_size);

    // Register callbacks and boot PicoDrive
    retro_set_environment(env_cb);
    retro_set_video_refresh(video_refresh_cb);
    retro_set_input_poll(input_poll_cb);
    retro_set_input_state(input_state_cb);
    retro_set_audio_sample(audio_sample_cb);
    retro_set_audio_sample_batch(audio_sample_batch_cb);

    retro_init();

    // Load the ROM file into memory
    FILE *f = fopen(rom_path, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    size_t rom_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *rom_data = malloc(rom_size);
    if (!rom_data) {
        fclose(f);
        return false;
    }
    fread(rom_data, 1, rom_size, f);
    fclose(f);

    struct retro_game_info game = {
        .path = rom_path,
        .data = rom_data,
        .size = rom_size,
        .meta = NULL
    };

    bool loaded = retro_load_game(&game);
    free(rom_data);
    return loaded;
}

void core_run_frame(void) {
    retro_run();
}

void* core_get_gx_texture(int *out_width, int *out_height) {
    if (out_width) *out_width = current_width;
    if (out_height) *out_height = current_height;
    return gx_tex_buffer;
}

void core_shutdown(void) {
    retro_unload_game();
    retro_deinit();
    if (gx_tex_buffer) {
        free(gx_tex_buffer);
        gx_tex_buffer = NULL;
    }
}
