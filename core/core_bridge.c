#include "core_bridge.h"
#include "libretro/libretro.h"
#include <asndlib.h>
#include <wiiuse/wpad.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

static u16 *gx_tex_buffer = NULL;
static int current_width = 320;
static int current_height = 224;

// --- Audio Ring Buffer ---
#define AUDIO_RING_SIZE 4096
#define AUDIO_VOICE 0
static int16_t audio_ring[AUDIO_RING_SIZE];
static volatile int ring_head = 0;
static volatile int ring_tail = 0;

static void audio_ring_write(const int16_t *samples, size_t frames) {
    for (size_t i = 0; i < frames * 2; i++) {
        int next = (ring_head + 1) % AUDIO_RING_SIZE;
        if (next != ring_tail) {
            audio_ring[ring_head] = samples[i];
            ring_head = next;
        }
    }
}

void core_update_audio(void) {
    int16_t chunk[512];
    int count = 0;
    while (count < 512 && ring_tail != ring_head) {
        chunk[count++] = audio_ring[ring_tail];
        ring_tail = (ring_tail + 1) % AUDIO_RING_SIZE;
    }
    if (count > 0) {
        ASND_AddVoice(AUDIO_VOICE, chunk, count * sizeof(int16_t));
    }
}

// --- GX 4x4 Texture Tiler ---
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

// --- Libretro Callbacks ---
static void video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data || !gx_tex_buffer) return;
    current_width = width;
    current_height = height;
    tile_rgb565_to_gx((const u16 *)data, gx_tex_buffer, width, height, pitch / sizeof(u16));
    DCFlushRange(gx_tex_buffer, CORE_MAX_WIDTH * CORE_MAX_HEIGHT * sizeof(u16));
}

static void env_cb(unsigned cmd, void *data) {
    if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) {
        enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;
        *fmt = RETRO_PIXEL_FORMAT_RGB565;
    }
}

static void input_poll_cb(void) {
    PAD_ScanPads();
    WPAD_ScanPads();
}

static int16_t input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (port != 0 || device != RETRO_DEVICE_JOYPAD) return 0;

    u32 wpad = WPAD_ButtonsHeld(0);
    u32 pad  = PAD_ButtonsHeld(0);

    switch (id) {
        case RETRO_DEVICE_ID_JOYPAD_UP:     return (wpad & (WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_UP))    || (pad & PAD_BUTTON_UP);
        case RETRO_DEVICE_ID_JOYPAD_DOWN:   return (wpad & (WPAD_BUTTON_LEFT  | WPAD_CLASSIC_BUTTON_DOWN))  || (pad & PAD_BUTTON_DOWN);
        case RETRO_DEVICE_ID_JOYPAD_LEFT:   return (wpad & (WPAD_BUTTON_UP    | WPAD_CLASSIC_BUTTON_LEFT))  || (pad & PAD_BUTTON_LEFT);
        case RETRO_DEVICE_ID_JOYPAD_RIGHT:  return (wpad & (WPAD_BUTTON_DOWN  | WPAD_CLASSIC_BUTTON_RIGHT)) || (pad & PAD_BUTTON_RIGHT);
        case RETRO_DEVICE_ID_JOYPAD_B:      return (wpad & (WPAD_BUTTON_1     | WPAD_CLASSIC_BUTTON_Y))     || (pad & PAD_BUTTON_B);     // Genesis A
        case RETRO_DEVICE_ID_JOYPAD_Y:      return (wpad & (WPAD_BUTTON_2     | WPAD_CLASSIC_BUTTON_B))     || (pad & PAD_BUTTON_A);     // Genesis B
        case RETRO_DEVICE_ID_JOYPAD_A:      return (wpad & (WPAD_BUTTON_A     | WPAD_CLASSIC_BUTTON_A))     || (pad & PAD_BUTTON_X);     // Genesis C
        case RETRO_DEVICE_ID_JOYPAD_START:  return (wpad & (WPAD_BUTTON_PLUS  | WPAD_CLASSIC_BUTTON_PLUS))  || (pad & PAD_BUTTON_START);
        default: return 0;
    }
}

static void audio_sample_cb(int16_t left, int16_t right) {
    int16_t s[2] = { left, right };
    audio_ring_write(s, 1);
}

static size_t audio_sample_batch_cb(const int16_t *data, size_t frames) {
    audio_ring_write(data, frames);
    return frames;
}

// --- Public Core API ---
bool core_init(const char *rom_path) {
    size_t buf_size = CORE_MAX_WIDTH * CORE_MAX_HEIGHT * sizeof(u16);
    gx_tex_buffer = (u16 *)memalign(32, buf_size);
    if (!gx_tex_buffer) return false;
    memset(gx_tex_buffer, 0, buf_size);

    ASND_SetVoice(AUDIO_VOICE, VOICE_STEREO_16BIT, 22050, 0, NULL, 0, 127, 127, NULL);

    retro_set_environment(env_cb);
    retro_set_video_refresh(video_refresh_cb);
    retro_set_input_poll(input_poll_cb);
    retro_set_input_state(input_state_cb);
    retro_set_audio_sample(audio_sample_cb);
    retro_set_audio_sample_batch(audio_sample_batch_cb);

    retro_init();

    FILE *f = fopen(rom_path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    size_t rom_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *rom_data = malloc(rom_size);
    if (!rom_data) { fclose(f); return false; }
    fread(rom_data, 1, rom_size, f);
    fclose(f);

    struct retro_game_info game = { .path = rom_path, .data = rom_data, .size = rom_size };
    bool loaded = retro_load_game(&game);
    free(rom_data);
    return loaded;
}

void core_run_frame(void) {
    retro_run();
    core_update_audio();
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
