#ifndef __INT_MPU6050__
#define __INT_MPU6050__

#include "i2c.h"
#include "Com_debug.h"
#include "Com_Filter.h"
#define MPU6050_I2C_ADDR_W (0x68 << 1)
#define MPU6050_I2C_ADDR_R (0x68 << 1) + 1

// 寄存器地址
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_INT_ENABLE 0x38
#define MPU6050_USER_CTRL 0x6A
#define MPU6050_FIFO_EN 0x23
#define MPU6050_CONFIG 0x1A
#define MPU6050_SMPLRT_DIV 0x19
#define MPU6050_POWER_MANAGEMENT_1 0x6B
#define MPU6050_POWER_MANAGEMENT_2 0x6C
#define MPU6050_WHO_AM_I 0x75

// 数据寄存器地址
#define MPU6050_GYRO_XOUT_H 0x43
#define MPU6050_ACCEL_XOUT_H 0x3B

// 数值转换常量值
#define MPU6050_GYRO_SCALE 16.4
#define MPU6050_ACCEL_SCALE 16384.0

typedef struct
{
    float gyro_x;
    float gyro_y;
    float gyro_z;
} MPU6050_Gyro;

typedef struct
{
    float accel_x;
    float accel_y;
    float accel_z;
} MPU6050_Accel;

/**
 * @brief 初始化MPU6050
 *
 */
void Int_mpu6050_init(void);

/**
 * @brief 获取陀螺仪数据
 *
 * @param gyro
 */
void Int_mpu6050_get_gyro(MPU6050_Gyro *gyro);

void Int_mpu6050_get_gyro_with_filter(MPU6050_Gyro *gyro);

/**
 * @brief 获取加速度计数据
 *
 * @param accel
 */
void Int_mpu6050_get_accel(MPU6050_Accel *accel);

void Int_mpu6050_get_accel_with_filter(MPU6050_Accel *accel);

#endif // __INT_MPU6050__
