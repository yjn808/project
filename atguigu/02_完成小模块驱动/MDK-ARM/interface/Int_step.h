#ifndef __INT_STEP_H
#define __INT_STEP_H

#include "i2c.h"
#include "Com_debug.h"

#define DS3553_I2C_ADDR_W 0x4E
#define DS3553_I2C_ADDR_R 0x4F

#define DS3553_CHIP_ID 0X01
#define DS3553_USER_SET 0XC3
#define DS3553_CNT_L 0XC4
#define DS3553_CNT_M 0XC5
#define DS3553_CNT_H 0XC6


/**
 * @brief 初始化DS3553芯片  主要设置user_set寄存器
 *
 */
void Int_step_init(void);

/**
 * @brief 获取计步器的计数值
 * 
 * @return uint32_t 
 */
uint32_t Int_step_get_count(void);

#endif // !__INT_STEP_H
