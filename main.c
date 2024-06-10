#include "EPD_Test.h"   // Examples
#include "run_File.h"

#include "led.h"
#include "waveshare_PCF85063.h" // RTC
#include "DEV_Config.h"

#include "picture.h"
#include "calendar.h"

#include <time.h>
#include "ini.h"

#include "GUI_Paint.h"
#include <stdlib.h> // malloc() free()
#include <string.h>

#define enChargingRtc 0

/*
Mode 0: Automatically get pic folder names and sort them
Mode 1: Automatically get pic folder names but not sorted
Mode 2: pic folder name is not automatically obtained, users need to create fileList.txt file and write the picture name in TF card by themselves
*/
#define Mode 1


float measureVBAT(void)
{
    float Voltage=0.0;
    const float conversion_factor = 3.3f / (1 << 12);
    uint16_t result = adc_read();
    Voltage = result * conversion_factor * 3;
    printf("Raw value: 0x%03x, voltage: %f V\n", result, Voltage);
    return Voltage;
}

void chargeState_callback() 
{
    if(DEV_Digital_Read(VBUS)) {
        if(!DEV_Digital_Read(CHARGE_STATE)) {  // is charging
            ledCharging();
        }
        else {  // charge complete
            ledCharged();
        }
    }
}

void create_test_ini_file()
{
    const char *name = "config.ini";
    FRESULT fr;
    FIL file;
    char str_tmp[64] = {0};
// [special_0]
// date = 04-05
// mode = 0
// note = mom's birthday


    if (FS_isSdCardMounted()) {
        if (FS_isFileExist(name)) {
            f_unlink(name);

            fr =  f_open(&file, name, FA_CREATE_ALWAYS|FA_WRITE);
            if(FR_OK != fr) {
                return;
            }
            f_printf(&file, "[common]\nversion = 13\nmode = 1\nname = \"testini\"\n");
            f_printf(&file, "[special_0]\ndate = 6-10\nmode = 0\nnote = duanwu\n");
            f_printf(&file, "[special_2]\ndate = 04-65\nmode = 0\nnote = mom's birthday\n");

            f_close(&file);
        }

    }
}


void eink_display()
{
    Time_data target = {0};
    target.hours = 1;
    PCF85063_clear_alarm_flag();    // clear RTC alarm flag
    // rtcRunAlarm(Time, alarmTime);  // RTC run alarm
    rtcSetAlarm(target);
    float voltage = measureVBAT();

    // create_test_ini_file();

    EPD_Init();
    DISPLAY_Open();

    PICTURE_Draw();
    // CALENDAR_Init();
    rtc_time_initialize();
    CALENDAR_work(&voltage);
    

    DISPLAY_Draw();
    DISPLAY_Close();
}

int main()
{
    printf("Init...\r\n");
    if(DEV_Module_Init() != 0) {  // DEV init
        return -1;
    }
    
    watchdog_enable(8*1000, 1);    // 8s
    DEV_Delay_ms(1000);
    gpio_set_irq_enabled_with_callback(CHARGE_STATE, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, chargeState_callback);

    if(measureVBAT() < 3.1) {   // battery power is low
        printf("low power ...\r\n");
        PCF85063_alarm_Time_Disable();
        ledLowPower();  // LED flash for Low power
        powerOff(); // BAT off
        return 0;
    }
    else {
        printf("work ...\r\n");
        ledPowerOn();
    }

    FS_Init();

    if(!DEV_Digital_Read(VBUS)) {    // no charge state
        eink_display();
    }
    else {  // charge state
        chargeState_callback();
        while(DEV_Digital_Read(VBUS)) {
            measureVBAT();
            
            #if enChargingRtc
            if(!DEV_Digital_Read(RTC_INT)) {    // RTC interrupt trigger
                printf("rtc interrupt\r\n");
                run_display(Time, alarmTime, isCard);
            }
            #endif

            if(!DEV_Digital_Read(BAT_STATE)) {  // KEY pressed
                printf("key interrupt\r\n");
                eink_display();
            }
            DEV_Delay_ms(200);
        }
    }

    //DISPLAY_Close();
    FS_DeInit();
    printf("power off ...\r\n");
    powerOff(); // BAT off

    return 0;

}

