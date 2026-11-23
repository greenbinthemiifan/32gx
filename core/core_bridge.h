#ifndef CORE_BRIDGE_H
#define CORE_BRIDGE_H

#include <gccore.h>
#include <stdbool.h>

// Maximum 32X/Genesis framebuffer size
#define CORE_MAX_WIDTH  320
#define CORE_MAX_HEIGHT 240

bool core_init(const char *rom_path);
void core_run_frame(void);
void core_shutdown(void);

// Returns the GX-ready tiled RGB565 texture and current dimensions
void* core_get_gx_texture(int *out_width, int *out_height);

#endif // CORE_BRIDGE_H
