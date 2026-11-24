#include "browser.h"
#include "input.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

FileItem itemList[MAX_ITEMS];
int itemCount = 0;
char currentPath[512];

static int selected_index = 0;

bool browser_is_rom(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return false;
    
    return (strcasecmp(ext, ".32x") == 0 ||
            strcasecmp(ext, ".bin") == 0 ||
            strcasecmp(ext, ".md")  == 0 ||
            strcasecmp(ext, ".gen") == 0);
}

void browser_init(const char *preferredPath) {
    DIR *dir = opendir(preferredPath);
    
    if (dir) {
        closedir(dir);
        snprintf(currentPath, sizeof(currentPath), "%s", preferredPath);
    } else {
        const char *usbFallback = "usb:/roms/32x";
        dir = opendir(usbFallback);
        
        if (dir) {
            closedir(dir);
            snprintf(currentPath, sizeof(currentPath), "%s", usbFallback);
        } else {
            snprintf(currentPath, sizeof(currentPath), "sd:/");
        }
    }

    selected_index = 0;
    browser_load_directory(currentPath);
}

bool browser_load_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return false;

    itemCount = 0;
    struct dirent *entry;

    if (strcmp(path, "sd:/") != 0 && strcmp(path, "usb:/") != 0 && itemCount < MAX_ITEMS) {
        snprintf(itemList[itemCount].name, sizeof(itemList[itemCount].name), "..");
        itemList[itemCount].isFolder = true;
        itemCount++;
    }

    while ((entry = readdir(dir)) != NULL && itemCount < MAX_ITEMS) {
        if (entry->d_name[0] == '.') continue;

        bool is_dir = (entry->d_type == DT_DIR);
        bool is_rom = browser_is_rom(entry->d_name);

        if (is_dir || is_rom) {
            snprintf(itemList[itemCount].name, sizeof(itemList[itemCount].name), "%s", entry->d_name);
            itemList[itemCount].isFolder = is_dir;
            itemCount++;
        }
    }

    closedir(dir);
    
    if (selected_index >= itemCount) {
        selected_index = itemCount > 0 ? itemCount - 1 : 0;
    }
    return true;
}

int browser_get_selected_index(void) {
    return selected_index;
}

const char* browser_update(UIAction action) {
    if (itemCount == 0) return NULL;

    if (action == UI_ACTION_UP) {
        if (selected_index > 0) selected_index--;
    } else if (action == UI_ACTION_DOWN) {
        if (selected_index < itemCount - 1) selected_index++;
    } else if (action == UI_ACTION_SELECT) {
        FileItem *selected = &itemList[selected_index];

        if (selected->isFolder) {
            if (strcmp(selected->name, "..") == 0) {
                char *last_slash = strrchr(currentPath, '/');
                if (last_slash && last_slash != currentPath) {
                    *last_slash = '\0';
                }
            } else {
                strcat(currentPath, "/");
                strcat(currentPath, selected->name);
            }
            browser_load_directory(currentPath);
            selected_index = 0;
        } else {
            static char full_rom_path[768];
            snprintf(full_rom_path, sizeof(full_rom_path), "%s/%s", currentPath, selected->name);
            return full_rom_path;
        }
    } else if (action == UI_ACTION_BACK) {
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
