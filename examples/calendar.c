#include "ImageData.h"
#include "run_File.h"
#include "EPD_7in3f.h"
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "calendar.h"
#include "waveshare_PCF85063.h"


#include <stdlib.h> // malloc() free()
#include <string.h>


struct calendar_s {
    UBYTE * image;
    Time_data now;
};
struct calendar_s gstCalendar;


void CALENDAR_Init()
{
    Time_data now = {24, 5, 26, 15, 45, 0, 6};
    rtcSetTime(&now);
}


void CALENDAR_Open()
{
}


void CALENDAR_Draw()
{
    PCF85063_GetTimeNow(&gstCalendar.now);
    Paint_SetRotate(270);
    Paint_DrawString_EN(10, 100, "this is a calendar.", &Font16, EPD_7IN3F_BLACK, EPD_7IN3F_WHITE);

    char str_temp[64] = {0};
    sprintf(str_temp, "%d-%d-%d %d:%d:%d %d",
            gstCalendar.now.years,
            gstCalendar.now.months,
            gstCalendar.now.days,
            gstCalendar.now.hours,
            gstCalendar.now.minutes,
            gstCalendar.now.seconds,
			gstCalendar.now.weeks);

    Paint_DrawString_EN(10, 150, str_temp, &Font16, EPD_7IN3F_RED, EPD_7IN3F_WHITE);

    int offset_y = 400, height = 20;
    memset(str_temp, 0, 64);
    sprintf(str_temp, "VBUS: %d", DEV_Digital_Read(VBUS));
    Paint_DrawString_EN(10, offset_y, str_temp, &Font16, EPD_7IN3F_RED, EPD_7IN3F_WHITE);

    memset(str_temp, 0, 64);
    sprintf(str_temp, "RTC_INT: %d", DEV_Digital_Read(RTC_INT));
    Paint_DrawString_EN(10, offset_y+height, str_temp, &Font16, EPD_7IN3F_RED, EPD_7IN3F_WHITE);

    memset(str_temp, 0, 64);
    sprintf(str_temp, "CHARGE_STATE: %d", DEV_Digital_Read(CHARGE_STATE));
    Paint_DrawString_EN(10, offset_y+height*2, str_temp, &Font16, EPD_7IN3F_RED, EPD_7IN3F_WHITE);
//当接电源的时候 110 ，不接 011
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
