#include "browser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>

FileItem itemList[MAX_ITEMS];
int itemCount = 0;
char currentPath[512];

// Check if a file has a valid 32X / Genesis ROM extension
bool browser_is_rom(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return false;

    return (strcasecmp(ext, ".32x") == 0 ||
            strcasecmp(ext, ".bin") == 0 ||
            strcasecmp(ext, ".md")  == 0 ||
            strcasecmp(ext, ".gen") == 0);
}

void browser_init(const char *startPath) {
    snprintf(currentPath, sizeof(currentPath), "%s", startPath);
    browser_load_directory(currentPath);
}

bool browser_load_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return false;

    itemCount = 0;
    struct dirent *ent;

    // Read directory entries
    while ((ent = readdir(dir)) != NULL && itemCount < MAX_ITEMS) {
        // Skip hidden files/directories and parent/current pointers
        if (ent->d_name[0] == '.') continue;

        // Check if it's a directory or a valid ROM file
        bool isDir = (ent->d_type == DT_DIR);
        bool isRom = browser_is_rom(ent->d_name);

        if (isDir || isRom) {
            snprintf(itemList[itemCount].name, sizeof(itemList[itemCount].name), "%s", ent->d_name);
            itemList[itemCount].isFolder = isDir;
            itemCount++;
        }
    }

    closedir(dir);
    return true;
}
