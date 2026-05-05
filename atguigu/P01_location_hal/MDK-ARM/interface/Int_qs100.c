#include "Int_qs100.h"

uint8_t qs100_buff[QS100_BUFF_SIZE];
uint16_t qs100_buff_len = 0;
uint16_t qs100_current_len = 0;

uint8_t cmd_buff[QS100_DATA_MAX_LEM] = {0};
// 限制单次发送的内容最多长度为512/2
uint8_t data_hex_buff[QS100_DATA_MAX_LEM] = {0};

void Int_qs100_send_cmd(uint8_t *cmd)
{
    // 1. 发送指令
    HAL_UART_Transmit(&huart3, cmd, strlen((char *)cmd), 1000);
    memset(qs100_buff, 0, QS100_BUFF_SIZE);
    qs100_buff_len = 0;
    // 2. 等待指令执行完毕 => OK或者ERROR
    uint8_t count = 0;
    while (strstr((char *)qs100_buff, "OK") == NULL && strstr((char *)qs100_buff, "ERROR") == NULL)
    {
        // 判断缓冲区是否已经满了
        if (qs100_buff_len >= QS100_BUFF_SIZE)
        {
            break;
        }
        // 需要继续进行接收
        HAL_UARTEx_ReceiveToIdle(&huart3, qs100_buff + qs100_buff_len, QS100_BUFF_SIZE - qs100_buff_len, &qs100_current_len, 1000);
        if (qs100_buff_len + qs100_current_len >= QS100_BUFF_SIZE)
        {
            qs100_buff_len = QS100_BUFF_SIZE;
            break;
        }
        // 累积数据长度
        qs100_buff_len += qs100_current_len;
        count++;
        if (count > QS100_MAX_RECV_COUNTS)
        {
            break;
        }
    }
}

/**
 * @brief 初始化qs100
 *
 */
void Int_qs100_init(void)
{

    // 1. 硬件唤醒芯片
    HAL_GPIO_WritePin(NB_WKUP_GPIO_Port, NB_WKUP_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(NB_WKUP_GPIO_Port, NB_WKUP_Pin, GPIO_PIN_RESET);

    // 2. 软件重启  => 不会回复OK或者ERROR 其他的都会
    HAL_UART_Transmit(&huart3, "AT+RB\r\n", 7, 1000);
    memset(qs100_buff, 0, QS100_BUFF_SIZE);
    HAL_UART_Receive(&huart3, qs100_buff, QS100_BUFF_SIZE, 3000);
    debug_printf("%s", qs100_buff);

    // HAL_Delay(2000);
    // 发送查询版本信息
    // memset(qs100_buff, 0, 512);
    // HAL_UART_Transmit(&huart3, "AT+CGMR\r\n", 9, 1000);
    // HAL_UARTEx_ReceiveToIdle(&huart3, qs100_buff, 512, &rx_len, 1000);
    // debug_printf("%s", qs100_buff);

    // 3. 使用cmd命令 发送查询版本信息
    Int_qs100_send_cmd("AT+CGMR\r\n");
    debug_printf("%s", qs100_buff);

    // 4. 设置打开回显
    // Int_qs100_send_cmd("ATE1\r\n");
    // debug_printf("%s", qs100_buff);

    // // 5. 测试回显效果
    // Int_qs100_send_cmd("AT+CGMR\r\n");
    // debug_printf("%s", qs100_buff);
}

/**
 * @brief 查询芯片的附着状态(能否连接运行商网络) 判断是否能够连接外网
 *
 */
IOT_Status Int_qs100_get_ip(void)
{
    Int_qs100_send_cmd("AT+CGATT?\r\n");
    debug_printf("%s", qs100_buff);
    if (strstr((char *)qs100_buff, "+CGATT:1") != NULL)
    {
        return IOT_OK;
    }
    return IOT_ERROR;
}

/**
 * @brief 打开socket
 *
 * @return IOT_Status
 */
IOT_Status Int_qs100_open_socket(uint8_t *socket_num)
{
    // 0: 分频port端口号 不自己指定  避免出现冲突
    // 0: 不接收云服务返回的消息
    Int_qs100_send_cmd("AT+NSOCR=STREAM,6,0,0\r\n");
    debug_printf("%s", qs100_buff);
    if (strstr((char *)qs100_buff, "OK") != NULL)
    {
        // 打开socket的函数中需要记录使用的socket编号
        char *p = strstr((char *)qs100_buff, "+NSOCR:");
        if (p != NULL)
        {
            p += 7;
            *socket_num = atoi(p);
            debug_printf("socket_num: %d", *socket_num);
        }
        return IOT_OK;
    }

    return IOT_ERROR;
}

/**
 * @brief 连接外网TCP云服务器
 * https://netlab.luatos.com/
 * @return IOT_Status
 */
IOT_Status Int_qs100_connect_server(uint8_t socker_num)
{
    memset(cmd_buff, 0, QS100_DATA_MAX_LEM);
    sprintf((char *)cmd_buff, "AT+NSOCO=%d,%s,%d\r\n", socker_num, SERVER_IP, SERVER_PORT);

    Int_qs100_send_cmd(cmd_buff);

    debug_printf("%s", qs100_buff);

    if (strstr((char *)qs100_buff, "OK") != NULL)
    {
        return IOT_OK;
    }
    return IOT_ERROR;
}

/**
 * @brief
 *
 * @param socker_num
 * @param data 最大低于250
 * @param len
 * @return IOT_Status
 */
IOT_Status Int_qs100_send(uint8_t socker_num, uint8_t *data, uint16_t len)
{
    memset(cmd_buff, 0, QS100_DATA_MAX_LEM);
    // 将data转换为16进制字符串
    memset(data_hex_buff, 0, QS100_DATA_MAX_LEM);
    for (int i = 0; i < len; i++)
    {
        sprintf((char *)data_hex_buff + i * 2, "%02X", data[i]);
    }

    sprintf((char *)cmd_buff, "AT+NSOSD=%d,%d,%s,0x200,%d\r\n", socker_num, len, data_hex_buff, QS100_SEQUENCE_ID);
    Int_qs100_send_cmd(cmd_buff);
    // debug_printf("%s", qs100_buff);

    // 通过发送AT+SEQUENCE=SOCKID,SEQID查询数据是否已经发送成功
    uint8_t status = '2';
    while (status == '2')
    {
        memset(cmd_buff, 0, QS100_DATA_MAX_LEM);
        sprintf((char *)cmd_buff, "AT+SEQUENCE=%d,%d\r\n", socker_num, QS100_SEQUENCE_ID);
        Int_qs100_send_cmd(cmd_buff);
        // status赋值 = 接收数据中的结果
        // for (uint8_t i = 0; i < qs100_buff_len; i++)
        // {
        //     // debug_printf("%d:%c", i, qs100_buff[i]);
        // }
        // debug_printf("%s", qs100_buff);
        // uint8_t status_index = strlen((char *)qs100_buff) - 9;
        // debug_printf("status_index: %d", status_index);

        // 如果出现NSOSTR 忽略当前一次的消息
        if (strstr((char *)qs100_buff, "NSOSTR") == NULL)
        {
            if (strchr((char *)qs100_buff, '2') != NULL)
            {
                status = '2';
            }
            if (strchr((char *)qs100_buff, '1') != NULL)
            {
                status = '1';
            }
            if (strchr((char *)qs100_buff, '0') != NULL)
            {
                status = '0';
            }
            // debug_printf("status: %c", status);
        }
        // 添加延迟
        HAL_Delay(10);
    }
    if (status == '0')
    {
        return IOT_ERROR;
    }

    return IOT_OK;
}

/**
 * @brief 关闭socket
 *
 * @param socker_num
 * @return IOT_Status
 */
IOT_Status Int_qs100_close_socket(uint8_t socker_num)
{
    memset(cmd_buff, 0, QS100_DATA_MAX_LEM);

    sprintf((char *)cmd_buff, "AT+NSOCL=%d\r\n", socker_num);

    Int_qs100_send_cmd(cmd_buff);

    debug_printf("%s", qs100_buff);

    if (strstr((char *)qs100_buff, "OK") == NULL)
    {
        return IOT_ERROR;
    }
    return IOT_OK;
}

/**
 * @brief 使用外网发送消息到云服务器
 *
 * @param msg
 * @param len
 */
IOT_Status Int_qs100_send_msg(uint8_t *msg, uint16_t len)
{
    // 1. 检查附着 是否能够连接到外网
    uint8_t count = 0;
    while (Int_qs100_get_ip() != IOT_OK && count < QS100_MAX_RECV_COUNTS)
    {
        HAL_Delay(1000);
        count++;
    }
    uint8_t socket_num = 0;
    // 2. 创建socket
    while (Int_qs100_open_socket(&socket_num) != IOT_OK && count < QS100_MAX_RECV_COUNTS)
    {
        HAL_Delay(100);
        count++;
    }
    // 3. 连接云服务器
    while (Int_qs100_connect_server(socket_num) != IOT_OK && count < QS100_MAX_RECV_COUNTS)
    {
        HAL_Delay(100);
        count++;
    }

    // 4. 发送数据
    uint8_t send_count = 0;
    IOT_Status status = IOT_ERROR;
    while (Int_qs100_send(socket_num, msg, len) != IOT_OK && send_count < 3)
    {
        HAL_Delay(100);
        send_count++;
    }
    if (send_count >= 3)
    {
        // 发送失败
        status = IOT_ERROR;
    }
    else
    {
        // 发送成功
        status = IOT_OK;
    }
    // 5. 关闭连接
    Int_qs100_close_socket(socket_num);

    return status;
}

/**
 * @brief 创建连接 一直保存当前连接可以继续使用
 *
 * @param socket_num 表示创建的socket编号
 * @return IOT_Status 表示是否创建成功
 */
IOT_Status Int_qs100_create_connection(uint8_t *socket_num)
{
    // 1. 检查附着 是否能够连接到外网
    uint8_t count = 0;
    while (Int_qs100_get_ip() != IOT_OK && count < QS100_MAX_RECV_COUNTS)
    {
        HAL_Delay(1000);
        count++;
    }

    // 2. 创建socket
    while (Int_qs100_open_socket(socket_num) != IOT_OK && count < QS100_MAX_RECV_COUNTS)
    {
        HAL_Delay(100);
        count++;
    }
    // 3. 连接云服务器
    while (Int_qs100_connect_server(*socket_num) != IOT_OK && count < QS100_MAX_RECV_COUNTS)
    {
        HAL_Delay(100);
        count++;
    }
    if (count >= QS100_MAX_RECV_COUNTS)
    {
        return IOT_ERROR;
    }
    return IOT_OK;
}
