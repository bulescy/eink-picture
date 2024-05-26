#include "ImageData.h"
#include "run_File.h"
#include "EPD_7in3f.h"
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "calendar.h"
#include "waveshare_PCF85063.h"
#include "ini.h"

#include <stdlib.h> // malloc() free()
#include <string.h>


struct calendar_s {
    UBYTE * image;
    Time_data now;

    const char *configFile;
    
};

struct calendar_s gstCalendar = 
{
    .configFile = "config.ini",
};

typedef struct config_s
{
    int version;
    int mode;
    const char* name;
} config_t;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    config_t* pconfig = (config_t*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("common", "version")) {
        pconfig->version = atoi(value);
    } else if (MATCH("common", "name")) {
        pconfig->name = strdup(value);
    } else if (MATCH("common", "mode")) {
        pconfig->mode = atoi(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


void CALENDAR_Init()
{
    Time_data now = {24, 5, 26, 15, 45, 0, 6};
    rtcSetTime(&now);
}

void CALENDAR_GetConfig()
{
    config_t config;
    config.version = 0;  /* set defaults */
    config.mode = 0;
    config.name = NULL;
    char str_buf[100];

    if (ini_parse(gstCalendar.configFile, handler, &config) < 0) {
        printf("Can't load ini");
        Paint_DrawString_EN(10, 700, "Can't load ini", &Font16, EPD_7IN3F_BLACK, EPD_7IN3F_WHITE);
        return;
    }
    sprintf(str_buf, "Config loaded from ini: version=%d, mode=%d name=%s",
        config.version, config.mode, config.name);
    PrintString(str_buf);
    if (config.name)
        free((void*)config.name);
}

void CALENDAR_Open()
{
}


void CALENDAR_Draw()
{
    char str_temp[64] = {0};
    UWORD rotate = ROTATE_180;

    PCF85063_GetTimeNow(&gstCalendar.now);
    Paint_SetRotate(rotate);

    memset(str_temp, 0, 64);
    sprintf(str_temp, "this is a calendar, rotate: %d\n", rotate);
    PrintString(str_temp);

    memset(str_temp, 0, 64);
    sprintf(str_temp, "%d-%d-%d %d:%d:%d %d\n",
            gstCalendar.now.years,
            gstCalendar.now.months,
            gstCalendar.now.days,
            gstCalendar.now.hours,
            gstCalendar.now.minutes,
            gstCalendar.now.seconds,
			gstCalendar.now.weeks);
    PrintString(str_temp);

    memset(str_temp, 0, 64);
    sprintf(str_temp, "VBUS: %d\n", DEV_Digital_Read(VBUS));
    PrintString(str_temp);

    memset(str_temp, 0, 64);
    sprintf(str_temp, "RTC_INT: %d\n", DEV_Digital_Read(RTC_INT));
    PrintString(str_temp);

    memset(str_temp, 0, 64);
    sprintf(str_temp, "CHARGE_STATE: %d\n", DEV_Digital_Read(CHARGE_STATE));
    PrintString(str_temp);
//当接电源的时候 110 ，不接 011

    if (FS_isSdCardMounted() == true)
        CALENDAR_GetConfig();
}


void CALENDAR_Close()
{
}

void CALENDAR_Deinit()
{

}

void CALENDAR_Test()
{
    CALENDAR_Init();
    CALENDAR_Open();
    CALENDAR_Draw();
    CALENDAR_Close();
    CALENDAR_Deinit();
}
