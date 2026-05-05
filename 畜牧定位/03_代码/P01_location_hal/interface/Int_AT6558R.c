#include "Int_AT6558R.h"

uint8_t gps_buff[GPS_BUFF_MAX_LEN];
uint16_t gps_buff_len;

uint8_t gps_full_buff[GPS_BUFF_MAX_LEN];
uint16_t gps_full_buff_len;

// 设置参数的宏定义
#define AT6558R_FREQ "PCAS02,1000"
#define AT6558R_MODE "PCAS04,3"
#define AT6558R_INFO "PCAS06,2"

// 发送命令缓冲区
uint8_t send_cmd[64];
void Int_AT6558R_Send_CMD(uint8_t *cmd)
{
    // 计算校验和 => 所有的字节异或的结果
    uint8_t tmp = cmd[0];

    for (uint8_t i = 1; cmd[i] != '\0'; i++)
    {
        tmp ^= cmd[i];
    }

    // 拼接发送的名称 => 拼接校验和的细节 8和08不一样  1A 和 1a不一样
    sprintf((char *)send_cmd, "$%s*%02X\r\n", cmd, tmp);

    // 打印输出命令 => 比对
    debug_println("\n%s", send_cmd);

    HAL_UART_Transmit(&huart2, send_cmd, strlen((char *)send_cmd), 1000);
}

void Int_AT6558R_Init(void)
{
    // 1. 初始化底层驱动 => USART2
    // MX_USART2_UART_Init();

    // 2. GPS_EN引脚
    // HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_SET);

    // 3. 修改参数
    // 3.1 设置刷新率  $PCAS02,1000*2E\r\n
    Int_AT6558R_Send_CMD(AT6558R_FREQ);

    // 3.2 设置定位模式
    Int_AT6558R_Send_CMD(AT6558R_MODE);

    // // 3.3 交互芯片 读取信息
    // // 读取芯片的初始数据
    // HAL_UART_Receive(&huart2, gps_buff, GPS_BUFF_MAX_LEN, 1000);
    // debug_println("\n%s", gps_buff);
    // memset(gps_buff, 0, GPS_BUFF_MAX_LEN);

    // // 发送读取芯片参数
    // Int_AT6558R_Send_CMD(AT6558R_INFO);
    // HAL_UART_Receive(&huart2, gps_buff, GPS_BUFF_MAX_LEN, 1000);
    // debug_println("\n%s", gps_buff);
    // memset(gps_buff, 0, GPS_BUFF_MAX_LEN);
}

void Int_AT6558R_Read_GPS(void)
{
    // 读取GPS信息 => 接收USART2的信息

    /**
     *  $GNGGA,013050.000,3106.67898,N,12113.52954,E,1,07,3.1,-10.7,M,10.3,M,,*5A
        $GNGLL,3106.67898,N,12113.52954,E,013050.000,A,A*43
        $GNGSA,A,3,10,16,26,194,,,,,,,,,5.2,3.1,4.2,1*0C
        $GNGSA,A,3,13,23,38,,,,,,,,,,5.2,3.1,4.2,4*3F
        $GPGSV,3,1,11,03,20,308,,10,12,183,17,16,27,221,33,25,23,043,,0*61
        $GPGSV,3,2,11,26,59,238,41,28,59,010,,29,31,085,,31,52,320,,0*67
        $GPGSV,3,3,11,32,63,126,44,194,66,109,35,195,08,160,,0*54
        $BDGSV,2,1,06,07,,,37,13,32,220,34,14,15,159,,23,43,269,30,0*42
        $BDGSV,2,2,06,25,63,008,,38,20,197,35,0*78
        核心定位数据RMC 时间,有效,维度,维度方向,经度,经度方向,速度,航向,日期....
        $GNRMC,013050.000,A,3106.67898,N,12113.52954,E,5.19,77.74,160125,,,A,V*31
        $GNVTG,77.74,T,,M,5.19,N,9.61,K,A*13
        $GNZDA,013050.000,16,01,2025,00,00*4C
        $GPTXT,01,01,01,ANTENNA OPEN*25
     */
    // HAL_UARTEx_ReceiveToIdle(&huart2, gps_buff, GPS_BUFF_MAX_LEN, &gps_buff_len, 1000);
    // debug_println("\n%s", gps_buff);
    // memset(gps_buff, 0, GPS_BUFF_MAX_LEN);

    // 首先初始化FULL_BUFF
    memset(gps_full_buff, 0, GPS_FULL_BUFF_MAX_LEN);
    gps_full_buff_len = 0;
    // 一直接收数据 => 直到数据完整包含GGA和TXT, 如果接收数据中间出现中断 => 拼接两次接收的数据
    while (1)
    {
        // 判断数据接收完整
        if (strstr((char *)gps_full_buff, "GGA") != NULL && strstr((char *)gps_full_buff, "TXT") != NULL)
        {
            // 当前一次接收数据完整
            break;
        }
        HAL_UARTEx_ReceiveToIdle(&huart2, gps_buff, GPS_BUFF_MAX_LEN, &gps_buff_len, 1000);
        if (gps_buff_len > 0)
        {
            // 当前接收到数据 => 拼接到大的缓冲区
            memcpy(&gps_full_buff[gps_full_buff_len], gps_buff, gps_buff_len);
            // 记录长度
            gps_full_buff_len += gps_buff_len;
            // 初始化接收缓冲区
            memset(gps_buff, 0, GPS_BUFF_MAX_LEN);
            gps_buff_len = 0;
        }
    }

    // 打印输出查看gps信息
    debug_println("\n%s", gps_full_buff);
}

void Int_AT6558R_Enter_Low_Power(void)
{
    // 进入低功耗
    HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_RESET);
}

void Int_AT6558R_Exit_Low_Power(void)
{
    // 退出低功耗
    HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_SET);
    Com_Delay_s(1);
}
