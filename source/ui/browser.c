#include "browser.h"
#include "input.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

// Global definitions matching browser.h externs
FileItem itemList[MAX_ITEMS];
int itemCount = 0;
char currentPath[512];

// Internal cursor position for the UI grid/list
static int selected_index = 0;

// Helper: Check if filename ends with valid Sega ROM extensions
bool browser_is_rom(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return false;
    
    // Check for 32X, Genesis, and Mega Drive ROM formats
    return (strcasecmp(ext, ".32x") == 0 ||
            strcasecmp(ext, ".bin") == 0 ||
            strcasecmp(ext, ".md")  == 0 ||
            strcasecmp(ext, ".gen") == 0);
}

// Initialize browser state and load the starting directory
void browser_init(const char *startPath) {
    snprintf(currentPath, sizeof(currentPath), "%s", startPath);
    selected_index = 0;
    browser_load_directory(currentPath);
}

// Scan a directory path and populate itemList
bool browser_load_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return false;

    itemCount = 0;
    struct dirent *entry;

    // Always add a ".." parent directory option if not at the root
    if (strcmp(path, "sd:/") != 0 && strcmp(path, "usb:/") != 0 && itemCount < MAX_ITEMS) {
        snprintf(itemList[itemCount].name, sizeof(itemList[itemCount].name), "..");
        itemList[itemCount].isFolder = true;
        itemCount++;
    }

    while ((entry = readdir(dir)) != NULL && itemCount < MAX_ITEMS) {
        // Skip hidden files/directories
        if (entry->d_name[0] == '.') continue;

        bool is_dir = (entry->d_type == DT_DIR);
        bool is_rom = browser_is_rom(entry->d_name);

        // Include directories or valid ROM files
        if (is_dir || is_rom) {
            snprintf(itemList[itemCount].name, sizeof(itemList[itemCount].name), "%s", entry->d_name);
            itemList[itemCount].isFolder = is_dir;
            itemCount++;
        }
    }

    closedir(dir);
    
    // Reset cursor if out of bounds
    if (selected_index >= itemCount) {
        selected_index = itemCount > 0 ? itemCount - 1 : 0;
    }
    return true;
}

// Get the current highlighted index for render_frame(selectedIndex)
int browser_get_selected_index(void) {
    return selected_index;
}

// Handle directional input and selection, returning a ROM path if a game is chosen
const char* browser_update(UIAction action) {
    if (itemCount == 0) return NULL;

    // Handle navigation
    if (action == UI_ACTION_UP) {
        if (selected_index > 0) selected_index--;
    } else if (action == UI_ACTION_DOWN) {
        if (selected_index < itemCount - 1) selected_index++;
    } 
    // Handle opening folders or selecting ROMs
    else if (action == UI_ACTION_SELECT) {
        FileItem *selected = &itemList[selected_index];

        if (selected->isFolder) {
            if (strcmp(selected->name, "..") == 0) {
                // Navigate up one directory level
                char *last_slash = strrchr(currentPath, '/');
                if (last_slash && last_slash != currentPath) {
                    *last_slash = '\0';
                }
            } else {
                // Enter subdirectory
                strcat(currentPath, "/");
                strcat(currentPath, selected->name);
            }
            browser_load_directory(currentPath);
            selected_index = 0; // Reset cursor on directory change
        } else {
            // It's a ROM file! Build and return its full absolute path.
            static char full_rom_path[768];
            snprintf(full_rom_path, sizeof(full_rom_path), "%s/%s", currentPath, selected->name);
            return full_rom_path;
        }
    } 
    // Handle going back a folder when pressing B / Back button
    else if (action == UI_ACTION_BACK) {
        if (strcmp(currentPath, "sd:/") != 0 && strcmp(currentPath, "usb:/") != 0) {
            char *last_slash = strrchr(currentPath, '/');
            if (last_slash && last_slash != currentPath) {
                *last_slash = '\0';
                browser_load_directory(currentPath);
                selected_index = 0;
            }
        }
    }

    return NULL;
}
