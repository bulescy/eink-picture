#ifndef _RUN_FILE_H_
#define _RUN_FILE_H_

#include "DEV_Config.h"

#define fileNumber 100
#define fileLen 100

uint8_t sdTest(void);
void sdInitTest(void);

void run_mount(void);
void run_unmount(void);

void file_cat(void);

void sdScanDir(void);

char isFileExist(const char *path);
void setFilePath(void);

void updatePathIndex(void);
void file_sort();

void logtest(int value);



int8_t FS_Init();
int8_t FS_DeInit();
bool FS_isSdCardMounted();
bool FS_isFileExist(const char *filename);
#endif
