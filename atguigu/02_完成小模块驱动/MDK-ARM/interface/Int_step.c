#include "Int_step.h"

uint8_t step_count[3] = {0};

/**
 * @brief 初始化DS3553芯片  主要设置user_set寄存器
 *
 */
void Int_step_init(void)
{

    // 配置寄存器之前 也可以测试一下I2C是否连接成功
    // uint8_t chip_id = 0;
    // HAL_I2C_Mem_Read(&hi2c1, DS3553_I2C_ADDR_R, DS3553_CHIP_ID, I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
    // debug_printf("chip_id: %d", chip_id);

    // 打开片选引脚
    HAL_GPIO_WritePin(DS3553_CS_GPIO_Port, DS3553_CS_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    // 配置user_set寄存器  => 正常工作模式  关闭抬手中断 打开计步中断  使用计步器算法 开启去噪 正常灵敏度
    uint8_t data = 0x1A;
    // 使用I2C寄存器的写入函数
    // (1)i2c编号 (2) I2C从设备地址区分读写  (3) 寄存器地址 (4) 寄存器长度 (5) 数据指针 (6) 数据长度 (7) 超时时间
    HAL_I2C_Mem_Write(&hi2c1, DS3553_I2C_ADDR_W, DS3553_USER_SET, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
    HAL_GPIO_WritePin(DS3553_CS_GPIO_Port, DS3553_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

/**
 * @brief 获取计步器的计数值
 *
 * @return uint32_t
 */
uint32_t Int_step_get_count(void)
{
    // 打开片选引脚
    HAL_GPIO_WritePin(DS3553_CS_GPIO_Port, DS3553_CS_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_I2C_Mem_Read(&hi2c1,DS3553_I2C_ADDR_R,DS3553_CNT_L,I2C_MEMADD_SIZE_8BIT,step_count,3,1000);
    
    HAL_Delay(10);
    HAL_GPIO_WritePin(DS3553_CS_GPIO_Port, DS3553_CS_Pin, GPIO_PIN_SET);
    return (step_count[0] | (step_count[1] << 8) | (step_count[2] << 16));
}
