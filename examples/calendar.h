#ifndef _CALENDAR_H_
#define _CALENDAR_H_

extern char str_debug[];
#define DEBUG_PRINT(fmt, ...)     \
do \
{ \
    memset(str_debug, 0, 64); \
    sprintf(str_debug, fmt, ##__VA_ARGS__); \
    PrintString(str_debug); \
} while(0)

typedef struct calendar_s calendar_t;

void CALENDAR_Init();
int CALENDAR_work(void *pdata);
void CALENDAR_Test();
void rtc_time_initialize();
#endif

