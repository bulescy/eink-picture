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

typedef enum {
    CALENDAR_MODE_SOLAR,
    CALENDAR_MODE_LUNAR,
    CALENDAR_MODE_MAX,
}CALENDAR_MODE_e;

typedef struct CALENDAR_date_s{
    int year;
    int month;
    int day;
    int reserved;
} CALENDAR_date_t;

#define CALENDAR_SPECIAL_DATE_MAX_NUMBER    10

typedef struct CALENDAR_special_date_s{
    CALENDAR_date_t date;
    CALENDAR_MODE_e mode;
    const char *note;
} CALENDAR_special_date_t;

struct calendar_s {
    UBYTE * image;
    UWORD rotate;
    Time_data now;

    const char *configFile;
    CALENDAR_date_t today[CALENDAR_MODE_MAX];
    CALENDAR_special_date_t special_day[CALENDAR_SPECIAL_DATE_MAX_NUMBER];

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

unsigned int lunar200y[199] = {
    0x04AE53,0x0A5748,0x5526BD,0x0D2650,0x0D9544,0x46AAB9,0x056A4D,0x09AD42,0x24AEB6,0x04AE4A,/*1901-1910*/
    0x6A4DBE,0x0A4D52,0x0D2546,0x5D52BA,0x0B544E,0x0D6A43,0x296D37,0x095B4B,0x749BC1,0x049754,/*1911-1920*/
    0x0A4B48,0x5B25BC,0x06A550,0x06D445,0x4ADAB8,0x02B64D,0x095742,0x2497B7,0x04974A,0x664B3E,/*1921-1930*/
    0x0D4A51,0x0EA546,0x56D4BA,0x05AD4E,0x02B644,0x393738,0x092E4B,0x7C96BF,0x0C9553,0x0D4A48,/*1931-1940*/
    0x6DA53B,0x0B554F,0x056A45,0x4AADB9,0x025D4D,0x092D42,0x2C95B6,0x0A954A,0x7B4ABD,0x06CA51,/*1941-1950*/
    0x0B5546,0x555ABB,0x04DA4E,0x0A5B43,0x352BB8,0x052B4C,0x8A953F,0x0E9552,0x06AA48,0x6AD53C,/*1951-1960*/
    0x0AB54F,0x04B645,0x4A5739,0x0A574D,0x052642,0x3E9335,0x0D9549,0x75AABE,0x056A51,0x096D46,/*1961-1970*/
    0x54AEBB,0x04AD4F,0x0A4D43,0x4D26B7,0x0D254B,0x8D52BF,0x0B5452,0x0B6A47,0x696D3C,0x095B50,/*1971-1980*/
    0x049B45,0x4A4BB9,0x0A4B4D,0xAB25C2,0x06A554,0x06D449,0x6ADA3D,0x0AB651,0x093746,0x5497BB,/*1981-1990*/
    0x04974F,0x064B44,0x36A537,0x0EA54A,0x86B2BF,0x05AC53,0x0AB647,0x5936BC,0x092E50,0x0C9645,/*1991-2000*/
    0x4D4AB8,0x0D4A4C,0x0DA541,0x25AAB6,0x056A49,0x7AADBD,0x025D52,0x092D47,0x5C95BA,0x0A954E,/*2001-2010*/
    0x0B4A43,0x4B5537,0x0AD54A,0x955ABF,0x04BA53,0x0A5B48,0x652BBC,0x052B50,0x0A9345,0x474AB9,/*2011-2020*/
    0x06AA4C,0x0AD541,0x24DAB6,0x04B64A,0x69573D,0x0A4E51,0x0D2646,0x5E933A,0x0D534D,0x05AA43,/*2021-2030*/
    0x36B537,0x096D4B,0xB4AEBF,0x04AD53,0x0A4D48,0x6D25BC,0x0D254F,0x0D5244,0x5DAA38,0x0B5A4C,/*2031-2040*/
    0x056D41,0x24ADB6,0x049B4A,0x7A4BBE,0x0A4B51,0x0AA546,0x5B52BA,0x06D24E,0x0ADA42,0x355B37,/*2041-2050*/
    0x09374B,0x8497C1,0x049753,0x064B48,0x66A53C,0x0EA54F,0x06B244,0x4AB638,0x0AAE4C,0x092E42,/*2051-2060*/
    0x3C9735,0x0C9649,0x7D4ABD,0x0D4A51,0x0DA545,0x55AABA,0x056A4E,0x0A6D43,0x452EB7,0x052D4B,/*2061-2070*/
    0x8A95BF,0x0A9553,0x0B4A47,0x6B553B,0x0AD54F,0x055A45,0x4A5D38,0x0A5B4C,0x052B42,0x3A93B6,/*2071-2080*/
    0x069349,0x7729BD,0x06AA51,0x0AD546,0x54DABA,0x04B64E,0x0A5743,0x452738,0x0D264A,0x8E933E,/*2081-2090*/
    0x0D5252,0x0DAA47,0x66B53B,0x056D4F,0x04AE45,0x4A4EB9,0x0A4D4C,0x0D1541,0x2D92B5          /*2091-2099*/
};

int monthTotal[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
CALENDAR_date_t toSolar(CALENDAR_date_t lunar){
    int year = lunar.year,
    month = lunar.month,
    day = lunar.day;
    int byNow, xMonth, i;
    CALENDAR_date_t solar;
    byNow = (lunar200y[year-1901] & 0x001F) - 1;
    if( ((lunar200y[year-1901]&0x0060)>>5) == 2)
        byNow += 31;
    for(i = 1; i < month; i ++){
        if( ( lunar200y[year - 1901] & (0x80000 >> (i-1)) ) ==0){
            byNow += 29;
        }
        else
            byNow += 30;
    }
    byNow += day;
    xMonth = (lunar200y[year - 1901] & 0xf00000)>>20;
    if(xMonth != 0){
        if(month > xMonth
           ||(month==xMonth && lunar.reserved == 1)){
            if((lunar200y[year-1901] & (0x80000>>(month-1)))==0)
                byNow += 29;
            else
                byNow += 30;
        }
    }
    if(byNow > 366
       ||(year%4!=0 && byNow == 365)){
        year += 1;
        if(year%4==0)
            byNow -= 366;
        else
            byNow -= 365;
    }
    for(i=1; i <= 13; i ++){
        if(monthTotal[i] >= byNow){
            month = i;
            break;
        }
    }
    solar.day = byNow - monthTotal[month-1];
    solar.month = month;
    solar.year = year;
    
    return solar;
}

CALENDAR_date_t toLunar(CALENDAR_date_t solar){
    int year = solar.year,
    month = solar.month,
    day = solar.day;
    int bySpring,bySolar,daysPerMonth;
    int index,flag;
    CALENDAR_date_t lunar;
    
    //bySpring 璁板綍鏄ヨ妭绂诲綋骞村厓鏃︾殑澶╂暟銆?
    //bySolar 璁板綍闃冲巻鏃ョ?诲綋骞村厓鏃︾殑澶╂暟銆?
    if( ((lunar200y[year-1901] & 0x0060) >> 5) == 1)
        bySpring = (lunar200y[year-1901] & 0x001F) - 1;
    else
        bySpring = (lunar200y[year-1901] & 0x001F) - 1 + 31;
    bySolar = monthTotal[month-1] + day - 1;
    if( (!(year % 4)) && (month > 2))
        bySolar++;
    
    //daysPerMonth璁板綍澶у皬鏈堢殑澶╂暟 29 鎴?30
    //index 璁板綍浠庡摢涓?鏈堝紑濮嬫潵璁＄畻銆?
    //flag 鏄?鐢ㄦ潵瀵归棸鏈堢殑鐗规畩澶勭悊銆?
    
    //鍒ゆ柇闃冲巻鏃ュ湪鏄ヨ妭鍓嶈繕鏄?鏄ヨ妭鍚?
    if (bySolar >= bySpring) {//闃冲巻鏃ュ湪鏄ヨ妭鍚庯紙鍚?鏄ヨ妭閭ｅぉ锛?
        bySolar -= bySpring;
        month = 1;
        index = 1;
        flag = 0;
        if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
            daysPerMonth = 29;
        else
            daysPerMonth = 30;
        while(bySolar >= daysPerMonth) {
            bySolar -= daysPerMonth;
            index++;
            if(month == ((lunar200y[year - 1901] & 0xF00000) >> 20) ) {
                flag = ~flag;
                if(flag == 0)
                    month++;
            }
            else
                month++;
            if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
                daysPerMonth=29;
            else
                daysPerMonth=30;
        }
        day = bySolar + 1;
    }
    else {//闃冲巻鏃ュ湪鏄ヨ妭鍓?
        bySpring -= bySolar;
        year--;
        month = 12;
        if ( ((lunar200y[year - 1901] & 0xF00000) >> 20) == 0)
            index = 12;
        else
            index = 13;
        flag = 0;
        if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
            daysPerMonth = 29;
        else
            daysPerMonth = 30;
        while(bySpring > daysPerMonth) {
            bySpring -= daysPerMonth;
            index--;
            if(flag == 0)
                month--;
            if(month == ((lunar200y[year - 1901] & 0xF00000) >> 20))
                flag = ~flag;
            if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
                daysPerMonth = 29;
            else
                daysPerMonth = 30;
        }
        
        day = daysPerMonth - bySpring + 1;
    }
    lunar.day = day;
    lunar.month = month;
    lunar.year = year;
    if(month == ((lunar200y[year - 1901] & 0xF00000) >> 20))
        lunar.reserved = 1;
    else
        lunar.reserved = 0;
    return lunar;
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
    const char *week_map[7] = {"Monday", "Tuesday", "Wednesday",
                          "Thursday", "Friday", "Saturday", "Sunday"};
    const char *ChDay[] = {"*","初一","初二","初三","初四","初五",
                               "初六","初七","初八","初九","初十",
                               "十一","十二","十三","十四","十五",
                               "十六","十七","十八","十九","二十",
                               "廿一","廿二","廿三","廿四","廿五",
                               "廿六","廿七","廿八","廿九","三十"};
    const char *ChMonth[] = {"*","正月","二月","三月","四月","五月","六月","七月",
                                "八月","九月","十月","十一月","腊月"};

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

    CALENDAR_date_t *pLunar = &gstCalendar.today[CALENDAR_MODE_LUNAR];
    Paint_DrawString_CN(0, 90, ChMonth[pLunar->month], &Font12CN, EPD_7IN3F_GREEN, EPD_7IN3F_TEXT_TRANSPARENT);
    Paint_DrawString_CN(40, 90, ChDay[pLunar->day], &Font12CN, EPD_7IN3F_GREEN, EPD_7IN3F_TEXT_TRANSPARENT);
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
//褰撴帴鐢垫簮鐨勬椂鍊? 110 锛屼笉鎺? 011

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

void _calendar_area(int first_weekday, int days_now, int days)
{
    char str_temp[64] = {0};
    char str_print[64] = {0};
    UWORD text_x_start = 5;
    UWORD text_y_start = 120;
    UBYTE pos_x = text_x_start, pos_y = text_y_start;
    sFONT *pFont = &Font16;
    UBYTE font_h = pFont->Height;
    UBYTE font_w = pFont->Width;
    UBYTE font_color = EPD_7IN3F_BLACK;
    UBYTE font_color_bg = EPD_7IN3F_TEXT_TRANSPARENT;

    memset(str_temp, 0, 64);
    sprintf(str_temp, "Mo Tu We Th Fr Sa Su");
    Paint_DrawString_EN(pos_x, pos_y, str_temp, pFont, font_color, font_color_bg);
    pos_y += font_h;

    memset(str_temp, 0, 64);
    sprintf(str_temp, "   ");
    for (int i = 0; i < first_weekday; i++) {
        strcat(str_print, str_temp);
    }
    Paint_DrawString_EN(pos_x, pos_y, str_print, pFont, font_color, font_color_bg);
    pos_x += font_w * 3 * first_weekday;

    for (int day = 1; day <= days; day++) {
        memset(str_temp, 0, 64);
        sprintf(str_temp, "%2d ", day);

        if (day == days_now) {
            font_color = EPD_7IN3F_RED;
            font_color_bg = EPD_7IN3F_YELLOW;
        } else {
            font_color = EPD_7IN3F_BLACK;
            font_color_bg = EPD_7IN3F_TEXT_TRANSPARENT;
        }
        Paint_DrawString_EN(pos_x, pos_y, str_temp, pFont, font_color, font_color_bg);
        pos_x += font_w * 3;

        if ((day + first_weekday) % 7 == 0) {
            pos_x = text_x_start;
            pos_y += font_h;
        }
    }
}


void _draw_date_full_month()
{
    UBYTE days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    UWORD years_now = 2000 + gstCalendar.now.years;
    UWORD months_now = gstCalendar.now.months;
    UWORD days_now = gstCalendar.now.days;
    UWORD weeks_now = gstCalendar.now.weeks;

    // Check for leap year
    if ((years_now % 4 == 0 && years_now % 100 != 0) || (years_now % 400 == 0)) {
        days_in_month[1] = 29; // February has 29 days in a leap year
    }

    int first_weekday = (7 - ((days_now - 1) % 7) + weeks_now) % 7;
    int days = days_in_month[months_now - 1];   // Number of days in the month
    _calendar_area(first_weekday, days_now, days);
}

bool isTodaySpecial(CALENDAR_MODE_e mode, CALENDAR_date_t check)
{
    CALENDAR_date_t *pToday = NULL;

    pToday = &gstCalendar.today[mode];
    if (pToday->day == check.day && pToday->month == check.month) {
        return true;
    }
    return false;
}

void _special_day_check()
{
    CALENDAR_special_date_t *pSpecial;
    for (int i = 0; i < CALENDAR_SPECIAL_DATE_MAX_NUMBER; ++i) {
        pSpecial = &gstCalendar.special_day[i];

        if (isTodaySpecial(pSpecial->mode, pSpecial->date)) {
            ;
        }
    }

}

void _get_calendar_info()
{
    CALENDAR_date_t *pSolar = &gstCalendar.today[CALENDAR_MODE_SOLAR];
    CALENDAR_date_t *pLunar = &gstCalendar.today[CALENDAR_MODE_LUNAR];

    pSolar->year = 2000+gstCalendar.now.years;
    pSolar->month = gstCalendar.now.months;
    pSolar->day = gstCalendar.now.days;
    *pLunar = toLunar(*pSolar);
}

void CALENDAR_work(void *pdata)
{
    PCF85063_GetTimeNow(&gstCalendar.now);
    Paint_SetRotate(gstCalendar.rotate);

    if (FS_isSdCardMounted() == true)
        CALENDAR_GetConfig();

    _get_calendar_info();
    _draw_date();
    _special_day_check();
    _draw_date_full_month();

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
