#include "App_GW.h"

uint8_t lora_data_full_buff[512];
uint16_t lora_data_len = 0;

void App_GW_Init(void)
{
    Int_LoRa_Init();
    Int_W5500_Init();
}

void App_GW_Receive_LoRa_Data(void)
{
    Int_LoRa_Receive_Data(lora_data_full_buff, &lora_data_len);

    if (lora_data_len > 0)
    {

        // 接收到定位器发送的汇总数据
        Int_W5500_Send_Data(lora_data_full_buff, lora_data_len);

        // 发送完成之后  删除数据
        memset(lora_data_full_buff, 0, sizeof(lora_data_full_buff));
        lora_data_len = 0;
    }
}
