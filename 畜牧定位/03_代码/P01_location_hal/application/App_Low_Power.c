#include "App_Low_Power.h"

void App_Enter_Low_Power(void)
{
    // 外设芯片进入低功耗
    Int_AT6558R_Enter_Low_Power();
    Int_QS100_Enter_Low_Power();

    RTC_TimeTypeDef rtc_time;
    RTC_AlarmTypeDef alarm;
    // 主控芯片必须最后进入低功耗 => 一旦进入低功耗 整个程序会关机
    // 设置实时时钟唤醒待机模式  => 当前的时间+需要定时的时间
    // 1. 获取当前时间
    HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);

    // 2. 设置的闹钟时间是在原先的基础上+定时的时间
    // 测试时间20s  实际开发使用1h
    alarm.AlarmTime.Hours = rtc_time.Hours;
    alarm.AlarmTime.Minutes = rtc_time.Minutes;
    alarm.AlarmTime.Seconds = rtc_time.Seconds + 20;
    HAL_RTC_SetAlarm(&hrtc, &alarm, RTC_FORMAT_BIN);
    // stm32待机模式 => 最省电 整个程序不再运行 => 寄存器存储的内容也会全部清空
    // 唤醒待机模式 => 1. 使用wakeup引脚 2. 使用RTC定时器

    // 清除待机标志位
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // 唤醒标志位
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); // 待机标志位
    debug_println("准备进入低功耗");

    // 之前最好延迟一会
    HAL_Delay(100);
    HAL_PWR_EnterSTANDBYMode();
}

void App_Exit_Low_Power(void)
{
    // 外设芯片退出低功耗

    Int_AT6558R_Exit_Low_Power();
    Int_QS100_Exit_Low_Power();
    debug_println("退出低功耗");
}
