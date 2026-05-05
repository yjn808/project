#ifndef __INT_W5500_H
#define __INT_W5500_H

#include "spi.h"
#include "w5500.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "Com_Debug.h"
#include "Com_Config.h"
void Int_W5500_Init(void);

// socket编程 => 实现客户端和服务端的通信
/* 启动一个客户端 连接电脑的服务端 */
CommmonStatus Int_W5500_Start_TCP_Client(void);

/* 发送数据到服务端 */
void Int_W5500_Send_Data(uint8_t data[],uint16_t data_len);
#endif
