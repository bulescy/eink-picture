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
    PAINT paint;
    UBYTE * image;
    Time_data now;

};
struct calendar_s gstCalendar;


void CALENDAR_Init()
{
    printf("CALENDAR Init and Clear...\r\n");
    EPD_7IN3F_Init();
}


void CALENDAR_Open()
{
    UDOUBLE Imagesize = ((EPD_7IN3F_WIDTH % 2 == 0)? (EPD_7IN3F_WIDTH / 2 ): (EPD_7IN3F_WIDTH / 2 + 1)) * EPD_7IN3F_HEIGHT;
    gstCalendar.image = (UBYTE *)malloc(Imagesize);

    if(gstCalendar.image == NULL) {
        printf("Failed to alloc memory...\r\n");
        return;
    }
    printf("CALENDAR_Open ok\r\n");
    Paint_NewImage(gstCalendar.image, EPD_7IN3F_WIDTH, EPD_7IN3F_HEIGHT, 0, EPD_7IN3F_WHITE);
    Paint_SetScale(7);
    printf("Display BMP\r\n");
    Paint_SelectImage(gstCalendar.image);
    Paint_Clear(EPD_7IN3F_WHITE);

    PCF85063_GetTimeNow(&gstCalendar.now);
}


void CALENDAR_Draw()
{
    // Paint_DrawBitMap(Image7color);
    Paint_SetRotate(270);
    Paint_DrawString_EN(10, 10, "this is a calendar.", &Font16, EPD_7IN3F_BLACK, EPD_7IN3F_WHITE);

    Time_data T;
    T = PCF85063_GetTime();
    printf("%d-%d-%d %d:%d:%d\r\n",T.years,T.months,T.days,T.hours,T.minutes,T.seconds);
    char str_temp[64] = {0};
    sprintf(str_temp, "%d-%d-%d %d:%d:%d\r\n",
            gstCalendar.now.years,
            gstCalendar.now.months,
            gstCalendar.now.days,
            gstCalendar.now.hours,
            gstCalendar.now.minutes,
            gstCalendar.now.seconds);

    Paint_DrawString_EN(10, 50, str_temp, &Font16, EPD_7IN3F_RED, EPD_7IN3F_WHITE);

    printf("EPD_Display\r\n");
    EPD_7IN3F_Display(gstCalendar.image);
}


void CALENDAR_Close()
{
    printf("Goto Sleep...\r\n\r\n");
    EPD_7IN3F_Sleep();
    if (gstCalendar.image != NULL) {
        free(gstCalendar.image);
    }
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