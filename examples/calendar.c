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
    UWORD rotate;
    Time_data now;

    const char *configFile;

};

struct calendar_s gstCalendar = 
{
    .configFile = "config.ini",
    .rotate = ROTATE_180,
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
    sprintf(str_buf, "Config loaded from ini: version=%d, mode=%d name=%s\n",
        config.version, config.mode, config.name);
    PrintString(str_buf);
    if (config.name)
        free((void*)config.name);
}

void CALENDAR_Open()
{
}

void _draw_date()
{
#define MAX_STR_LENGTH 64
    char * week_map[7] = {"Monday", "Tuesday", "Wednesday",
                          "Thursday", "Friday", "Saturday", "Sunday"};
    char str_temp[MAX_STR_LENGTH] = {0};
    int x = 0;
    int y = 0;

    sprintf(str_temp, "20%d-%d-%d",
            gstCalendar.now.years,
            gstCalendar.now.months,
            gstCalendar.now.days);
    Paint_DrawString_EN(0, 0, str_temp, &Font20, EPD_7IN3F_BLACK, EPD_7IN3F_TEXT_TRANSPARENT);

    memset(str_temp, 0, MAX_STR_LENGTH);
    sprintf(str_temp, "%d:%d:%d",
            gstCalendar.now.hours,
            gstCalendar.now.minutes,
            gstCalendar.now.seconds);
    Paint_DrawString_EN(0, 30, str_temp, &Font20, EPD_7IN3F_BLACK, EPD_7IN3F_TEXT_TRANSPARENT);

    Paint_DrawString_EN(0, 60, week_map[gstCalendar.now.weeks], &Font24, EPD_7IN3F_GREEN, EPD_7IN3F_TEXT_TRANSPARENT);
}

void _debug_info()
{
    char str_temp[64] = {0};
    memset(str_temp, 0, 64);
    sprintf(str_temp, "this is a calendar\n");
    PrintString(str_temp);

    // memset(str_temp, 0, 64);
    // sprintf(str_temp, "VBUS: %d\n", DEV_Digital_Read(VBUS));
    // PrintString(str_temp);

    // memset(str_temp, 0, 64);
    // sprintf(str_temp, "RTC_INT: %d\n", DEV_Digital_Read(RTC_INT));
    // PrintString(str_temp);

    // memset(str_temp, 0, 64);
    // sprintf(str_temp, "CHARGE_STATE: %d\n", DEV_Digital_Read(CHARGE_STATE));
    // PrintString(str_temp);
//当接电源的时候 110 ，不接 011

}

void _low_power_check(void *pdata)
{
    const float low_power_threshold = 5.0;
    float *pVoltage = (float *)pdata;
    char str_temp[64] = {0};

    if (pVoltage != NULL && *pVoltage < low_power_threshold) {
        memset(str_temp, 0, 64);
        sprintf(str_temp, "\nvoltage: %.2f, low power, please charge in time\n", *pVoltage);
        PrintString(str_temp);
    }
}

void CALENDAR_work(void *pdata)
{
    PCF85063_GetTimeNow(&gstCalendar.now);
    Paint_SetRotate(gstCalendar.rotate);

    if (FS_isSdCardMounted() == true)
        CALENDAR_GetConfig();

    _draw_date();

    _debug_info();
    _low_power_check(pdata);
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
    CALENDAR_work(NULL);
    CALENDAR_Close();
    CALENDAR_Deinit();
}
