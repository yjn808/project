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
