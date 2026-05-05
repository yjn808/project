#include "Com_Debug.h"

void Com_Debug_Init(void)
{
    // 初始化日志打印输出
    // MX_USART1_UART_Init();
}

int fputc(int ch, FILE *file)
{
    // 让printf方法重定向 使用串口USART1输出到电脑
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
    return ch;
}
