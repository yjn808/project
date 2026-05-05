#include "Com_Tool.h"

void Com_Delay_us(uint16_t us)
{
    // 主频 72MHz => 1/72 us
    uint32_t tmp = (us * 72) / 9;

    // 耗时代码 => 运行一次需要主频9次
    while (tmp--)
    {
        __NOP();
    }
}

void Com_Delay_ms(uint16_t ms)
{
    HAL_Delay(ms);
}

void Com_Delay_s(uint16_t s)
{
    while (s--)
    {
        HAL_Delay(1000);
    }
}

void Com_UTC_to_Bj_Time(uint8_t *UTC_Time, uint8_t *Bj_Time)
{
    struct tm utc;
    uint32_t year, month, day, hour, minute, second;
    sscanf((char *)UTC_Time, "%d-%d-%d %d:%d:%d",
           &year,
           &month,
           &day,
           &hour,
           &minute,
           &second);

    utc.tm_year = year - 1900;
    utc.tm_mon = month - 1;
    utc.tm_mday = day;
    utc.tm_hour = hour;
    utc.tm_min = minute;
    utc.tm_sec = second;

    // 对时间戳进行处理
    time_t time_stamp = mktime(&utc);
    
    time_stamp += 8 * 3600;
    debug_println("UTC时间戳：%ld", time_stamp);

    struct tm *local_time = localtime(&time_stamp);
    sprintf((char *)Bj_Time, "%04d-%02d-%02d %02d:%02d:%02d", local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday, local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

    debug_println("北京时间：%s", Bj_Time);
}
