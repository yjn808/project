#include "Int_DS3553.h"

/* 传入寄存器地址和寄存器接收数据的地址 */
void Int_DS3553_ReadReg(uint8_t reg_addr, uint8_t *reg_data)
{
    // 第一套API => 直接对字节地址进行读写操作
    // uint8_t reg_data;
    // HAL_I2C_Mem_Read(&hi2c1, DS3553_ADDR_R, 0X01, I2C_MEMADD_SIZE_8BIT, &reg_data, 1, 1000);

    // 第二套API => 调用底层读写字节方法
    // HAL_I2C_Master_Transmit(&hi2c1, DS3553_ADDR_W, 0X01, 1, 1000);
    // HAL_I2C_Master_Receive(&hi2c1, DS3553_ADDR_R, &reg_data, 1, 1000);

    // 1. 拉低片选
    DS3553_CS_L;

    Com_Delay_ms(5);

    // 2. 执行一次I2C的读取数据 读取寄存器
    HAL_I2C_Mem_Read(&hi2c1, DS3553_ADDR_R, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, 1, 1000);

    // 3. 拉高片选
    DS3553_CS_H;
    Com_Delay_ms(12);
}

void Int_DS3553_WriteReg(uint8_t reg_addr, uint8_t reg_data)
{
    // 1. 拉低片选
    DS3553_CS_L;

    Com_Delay_ms(5);

    // 2. 执行一次I2C的读取数据 读取寄存器
    HAL_I2C_Mem_Write(&hi2c1, DS3553_ADDR_W, reg_addr, I2C_MEMADD_SIZE_8BIT, &reg_data, 1, 1000);

    // 3. 拉高片选
    DS3553_CS_H;
    Com_Delay_ms(12);
}

void Int_DS3553_Init(void)
{
    // 1. 初始化I2C
    // MX_I2C1_Init();

    // 测试芯片使用 读取ID
    uint8_t reg_data = 0xff;
    Int_DS3553_ReadReg(0x01, &reg_data);
    debug_println("\nDS3553 ID: 0x%x", reg_data);

    // 2. 设置参数
    // 读取芯片默认值 0x18
    uint8_t tmp = 0x18;
    // 配置不同的参数
    // 2.1 关闭计步脉冲中断
    tmp &= ~(0x01 << 4);
    // 2.2 选择计步器算法 => 10
    tmp |= (0x01 << 1);
    Int_DS3553_WriteReg(DS3553_REG_USER_SET, tmp);
}

void Int_DS3553_Get_Step(uint32_t *step)
{
    // 读取寄存器中的步数
    uint8_t step_l = 0;
    uint8_t step_m = 0;
    uint8_t step_h = 0;
    Int_DS3553_ReadReg(DS3553_REG_STEP_CNT_L, &step_l);
    Int_DS3553_ReadReg(DS3553_REG_STEP_CNT_M, &step_m);
    Int_DS3553_ReadReg(DS3553_REG_STEP_CNT_H, &step_h);

    *step = (step_h << 16) | (step_m << 8) | step_l;
}
