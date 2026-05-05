#include "Int_w5500.h"

uint8_t ip[4] = {192, 168, 31, 88};
uint8_t ga[4] = {192, 168, 31, 1};
uint8_t sub[4] = {255, 255, 255, 0};
uint8_t mac[6] = {110, 120, 13, 140, 150, 16};

extern void user_wizchip_reg_func(void);

void Inf_W5500_Reset(void)
{
    // 拉低重置引脚 => 超过500us
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);

    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(5);
}

void Int_W5500_Init(void)
{
    // 1. 初始化驱动
    // MX_SPI2_Init();

    // 2. 软重启芯片
    Inf_W5500_Reset();

    // 3. 注册函数
    user_wizchip_reg_func();

    // 4. 设置W5500参数 => ip地址  mac地址 网关 子网掩码
    // 封装完整的API => set寄存器名称=> 直接写入
    // get寄存器名称 => 直接读取
    // 4.1 设置网关
    setGAR(ga);
    // 4.2 设置子网掩码
    setSUBR(sub);
    // 4.3 设置mac地址
    setSHAR(mac);
    // 4.4 设置ip地址
    setSIPR(ip);

    debug_println("w5500初始化完成");
}

// 选择使用的socket 0-7
#define SN 0
#define CLIENT_PORT 8080

uint8_t SERVER_IP[4] = {192, 168, 31, 122};
#define SERVER_PORT 8888
CommmonStatus Int_W5500_Start_TCP_Client(void)
{
    // 0. 判断当前的状态
    uint8_t sn_sr = getSn_SR(SN);
    // 1. 创建客户端
    if (sn_sr == SOCK_CLOSED)
    {
        // 资源被释放 能够用来创建客户端
        int8_t r = socket(SN, Sn_MR_TCP, CLIENT_PORT, 0);
        if (r == SN)
        {
            // 创建socket成功 初始化客户端成功
            debug_printf("创建socket成功");
        }
        else
        {
            debug_printf("创建socket失败");
        }
    }
    // 2. 连接服务端
    else if (sn_sr == SOCK_INIT)
    {
        // 主动连接服务端
        int8_t c_r = connect(SN, SERVER_IP, SERVER_PORT);
        if (c_r == SOCK_OK)
        {
            debug_println("连接服务端成功");
        }
        else
        {
            debug_println("连接服务端失败");
        }
    }
    // 3. 等待进入ES状态
    else if (sn_sr == SOCK_ESTABLISHED)
    {
        // 创建客户端成功 连接服务端成功
        return COMMON_OK;
    }
    // 4. 如果断开 => 关闭客户端
    else if (sn_sr == SOCK_CLOSE_WAIT)
    {
        // 断开连接
        close(SN);
    }

    return COMMON_ERROR;
}

void Int_W5500_Send_Data(uint8_t data[], uint16_t data_len)
{
    // 健壮性判断
    if (data_len == 0)
    {
        return;
    }

    // 创建客户端等待连接服务端成功 => 才能发送数据
    while (Int_W5500_Start_TCP_Client() != COMMON_OK)
    {
        HAL_Delay(10);
    }

    // 只有当前SN的状态为ES的时候  才能发送数据
    // 0. 判断当前的状态
    uint8_t sn_sr = getSn_SR(SN);
    if (sn_sr == SOCK_ESTABLISHED)
    {
        // 才能发送数据
        send(SN, data, data_len);
    }
}
