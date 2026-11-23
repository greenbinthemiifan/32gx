#ifndef BROWSER_H
#define BROWSER_H

#include <stdbool.h>

#define MAX_ITEMS 256

typedef struct {
    char name[256];
    bool isFolder;
} FileItem;

extern FileItem itemList[MAX_ITEMS];
extern int itemCount;
extern char currentPath[512];

void browser_init(const char *startPath);
bool browser_load_directory(const char *path);
bool browser_is_rom(const char *filename);

#endif
