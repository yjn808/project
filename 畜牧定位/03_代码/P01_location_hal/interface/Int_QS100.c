#include "Int_QS100.h"

#define IOT_BUFF_MAX_LEN 128
uint8_t iot_buff[IOT_BUFF_MAX_LEN];
uint16_t iot_buff_len;

#define IOT_FULL_BUFF_MAX_LEN 256
uint8_t iot_full_buff[IOT_FULL_BUFF_MAX_LEN];
uint16_t iot_full_buff_len;

// QS100发送数据的最大重试次数
#define QS100_SEND_MAX_RETRY_TIMES 5
#define SEQUENCE 1
void Int_QS100_Wkup(void)
{
    HAL_GPIO_WritePin(NB_WK_GPIO_Port, NB_WK_Pin, GPIO_PIN_SET);
    Com_Delay_ms(5);
    HAL_GPIO_WritePin(NB_WK_GPIO_Port, NB_WK_Pin, GPIO_PIN_RESET);
    Com_Delay_ms(5);
}

void Int_QS100_Reset(void)
{
    // 发送一条命令
    uint8_t *cmd = "AT+RB\r\n";
    HAL_UART_Transmit(&huart3, cmd, strlen((char *)cmd), 1000);

    // 处理接收数据
    // AT+RB 不会回复标志位ok或者error   等待满3s => 打印接收到的全部消息 => 中间会有多次idle信号
    HAL_UART_Receive(&huart3, iot_buff, IOT_BUFF_MAX_LEN, 5000);
    iot_buff_len = strlen((char *)iot_buff);
    if (iot_buff_len > 0)
    {
        debug_println("%s", iot_buff);
        debug_println("IOT芯片初始化完成");
        iot_buff_len = 0;
        memset(iot_buff, 0, IOT_BUFF_MAX_LEN);
    }
}

void Int_QS100_Send_Cmd(uint8_t *cmd)
{
    HAL_UART_Transmit(&huart3, cmd, strlen((char *)cmd), 1000);

    // 在每次需要重新接收数据的时候清除缓存
    memset(iot_full_buff, 0, IOT_FULL_BUFF_MAX_LEN);
    iot_full_buff_len = 0;

    do
    {
        HAL_UARTEx_ReceiveToIdle(&huart3, iot_buff, IOT_BUFF_MAX_LEN, &iot_buff_len, 3000);
        if (iot_buff_len > 0)
        {
            // 将接受到的数据拼接到一起  => 直到收到OK或者ERROR为止
            memcpy(&iot_full_buff[iot_full_buff_len], iot_buff, iot_buff_len);
            iot_full_buff_len += iot_buff_len;
            memset(iot_buff, 0, IOT_BUFF_MAX_LEN);
            iot_buff_len = 0;
        }
    } while (strstr((char *)iot_full_buff, "OK") == NULL && strstr((char *)iot_full_buff, "ERROR") == NULL);

    debug_println("%s", iot_full_buff);
    debug_println("============================");
}

/**
 * 串口通信 => 芯片在执行一条指令的时候  必须等待执行完成 才能执行下一条指令
 * AT+RB: 软重启命令 => 不会给你回复ok或者error  最好的处理方法 => 等待3s - 5s
 * 其他命令: 比如ATE1 => 会给你回复ok或者error 判断数据的处理方法 => 一直接收到ok或者error为止
 */
void Int_QS100_Init(void)
{
    // 1. 初始化驱动
    // MX_USART3_UART_Init();

    // 2. 唤醒芯片
    // Int_QS100_Wkup();

    // 3. 芯片软重启
    Int_QS100_Reset();

    // 4. 打开串口回显
    Int_QS100_Send_Cmd("ATE1\r\n");

    // 5. 查询软件版本信息
    Int_QS100_Send_Cmd("AT+CGMR\r\n");
}

CommmonStatus Int_QS100_GetIP(void)
{
    // 1. 查询芯片是否处于附着状态
    /**
     *  AT+CGATT?
        +CGATT:1
        OK
     */
    Int_QS100_Send_Cmd("AT+CGATT?\r\n");

    // 2. 判断结果 => 不能在打印完结果之后就清除缓存
    if (strstr((char *)iot_full_buff, "+CGATT:1") == NULL)
    {
        return COMMON_ERROR;
    }

    return COMMON_OK;
}

CommmonStatus Int_QS100_Create_Client(uint8_t *socket)
{
    // 发送创建客户端的命令

    Int_QS100_Send_Cmd("AT+NSOCR=STREAM,6,0,0\r\n");
    // 接收返回的socket的值
    if (strstr((char *)iot_full_buff, "OK") == NULL)
    {
        return COMMON_ERROR;
    }

    /**
     *  AT+NSOCR=STREAM,6,0,0
        +NSOCR:0
        OK
     */
    char *tmp = strstr((char *)iot_full_buff, "+NSOCR:");
    debug_println("socket:%s", tmp);
    // socket接收的值是字节 => 不是数字0 => 0字节对应的ASCII码就是48
    *socket = tmp[7] - 48;
    debug_println("socket11:%d", *socket);
    // 也可以使用sscanf匹配字符串 => %d会直接转换为数字 => 结果就是0
    // sscanf((char *)iot_full_buff, "%*[^:]:%hhu", socket);
    // debug_println("socket22:%d", *socket);
    return COMMON_OK;
}

CommmonStatus Int_QS100_Connect_Server(uint8_t socket, uint8_t *ip, uint16_t port)
{
    // 拼接命令
    uint8_t cmd[64] = {0};
    sprintf((char *)cmd, "AT+NSOCO=%d,%s,%d\r\n", socket, ip, port);

    // 发送连接命令
    Int_QS100_Send_Cmd(cmd);

    // 判断连接成功和失败 => 返回值OK和ERROR
    if (strstr((char *)iot_full_buff, "OK") == NULL)
    {
        return COMMON_ERROR;
    }

    return COMMON_OK;
}

CommmonStatus Int_QS100_Close_Client(uint8_t socket)
{
    // 拼接命令
    uint8_t cmd[32] = {0};
    sprintf((char *)cmd, "AT+NSOCL=%d\r\n", socket);

    // 发送命令
    Int_QS100_Send_Cmd(cmd);

    // 判断连接成功和失败 => 返回值OK和ERROR
    if (strstr((char *)iot_full_buff, "OK") == NULL)
    {
        return COMMON_ERROR;
    }

    return COMMON_OK;
}

// 拼接命令
uint8_t cmd[512] = {0};

CommmonStatus Int_QS100_Send_Data_to_Server(uint8_t socket, uint8_t *pData, uint16_t len)
{
    // 需要将数据转换为16进制字符串
    uint16_t hex_len = len * 2 + 1;
    uint8_t hex_data[hex_len];
    memset(hex_data, 0, hex_len);
    for (uint16_t i = 0; i < len; i++)
    {
        sprintf((char *)hex_data + i * 2, "%02X", pData[i]);
    }

    sprintf((char *)cmd, "AT+NSOSD=%d,%d,%s,0x200,%d\r\n", socket, len, hex_data, SEQUENCE);
    debug_println("%s", pData);
    // 发送命令
    Int_QS100_Send_Cmd(cmd);
    debug_println("%s", pData);
    // 判断发送数据成功和失败 => 不等于对方收到数据了 => ACK机制 => 回复ACK
    memset(cmd, 0, sizeof(cmd));

    // 发送确认收到ACK的命令
    /**
     *  AT+SEQUENCE=0,1

        2

        OK
     */
    sprintf((char *)cmd, "AT+SEQUENCE=%d,%d\r\n", socket, SEQUENCE);

    // 需要循环不断发送请求 => 一直等待显示发送成功或者失败
    char tmp = 0xff;

    while (tmp != '0' && tmp != '1')
    {
        // 手动在这里加延迟
        Com_Delay_ms(5);
        Int_QS100_Send_Cmd(cmd);
        tmp = iot_full_buff[iot_full_buff_len - 9];
    }
    if (tmp == '0')
    {
        return COMMON_ERROR;
    }

    return COMMON_OK;
}

/**
 * https://netlab.luatos.com/
 * 后续实际使用芯片 => 发送一次完整定位数据 => 进入低功耗模式
 */
CommmonStatus Int_QS100_SendData(uint8_t *pData, uint16_t len)
{
    CommmonStatus status = COMMON_ERROR;
    int8_t count = QS100_SEND_MAX_RETRY_TIMES;
    // 1. 判断是否能连接外网
    while (status == COMMON_ERROR && count > 0)
    {
        status = Int_QS100_GetIP();
        // 需要添加重试的机会  不能一次失败直接返回
        count--;
        Com_Delay_ms(100);
    }

    if (count == 0)
    {
        // 重试次数全部用完了
        debug_println("QS100不能连接外网");
        return COMMON_ERROR;
    }

    count = QS100_SEND_MAX_RETRY_TIMES;
    status = COMMON_ERROR;
    uint8_t socket = 0xff;
    // 2. 创建客户端
    while (status == COMMON_ERROR && count > 0)
    {
        status = Int_QS100_Create_Client(&socket);
        count--;
        Com_Delay_ms(100);
    }
    if (count == 0)
    {
        // 重试次数全部用完了
        debug_println("QS100创建客户端失败");
        return COMMON_ERROR;
    }
    else
    {
        debug_println("QS100创建客户端成功,socket:%d", socket);
    }

    count = QS100_SEND_MAX_RETRY_TIMES;
    status = COMMON_ERROR;

    // 3. 连接服务端
    while (status == COMMON_ERROR && count > 0)
    {
        status = Int_QS100_Connect_Server(socket, HTTP_SERVER_IP, HTTP_SERVER_PORT);
        count--;
    }

    if (count == 0)
    {
        debug_println("QS100连接服务端失败");
        // 关闭客户端
        Int_QS100_Close_Client(socket);
        return COMMON_ERROR;
    }

    count = QS100_SEND_MAX_RETRY_TIMES;
    status = COMMON_ERROR;
    // 4. 进入ES状态 => 可以发送数据
    // 5. 发送数据
    while (status == COMMON_ERROR && count > 0)
    {
        status = Int_QS100_Send_Data_to_Server(socket, pData, len);
        count--;
    }
    if (count == 0)
    {
        debug_println("发送数据失败");
        // 关闭客户端
        Int_QS100_Close_Client(socket);
        return COMMON_ERROR;
    }

    // 6. 关闭客户端
    count = QS100_SEND_MAX_RETRY_TIMES;
    status = COMMON_ERROR;
    while (status == COMMON_ERROR && count > 0)
    {
        status = Int_QS100_Close_Client(socket);
        count--;
    }
    if (count == 0)
    {
        debug_println("QS100关闭客户端失败");
        return COMMON_ERROR;
    }
    Com_Delay_s(1);
    return COMMON_OK;
}

void Int_QS100_Enter_Low_Power(void)
{
    uint8_t *cmd = "AT+FASTOFF=0\r\n";
    Int_QS100_Send_Cmd(cmd);
}

void Int_QS100_Exit_Low_Power(void)
{
    Int_QS100_Wkup();
}
