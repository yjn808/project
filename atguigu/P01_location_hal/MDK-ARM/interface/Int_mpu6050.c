#include "Int_mpu6050.h"

MPU6050_Gyro calibration_gyro;
MPU6050_Accel calibration_accel;

void Int_mpu6050_write_reg(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_I2C_ADDR_W, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

uint8_t Int_mpu6050_read_reg(uint8_t reg)
{
    uint8_t data = 0;
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2C_ADDR_W, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 3000);
    return data;
}

void Int_mpu6050_read_regs(uint8_t reg, uint8_t *data, uint8_t len)
{
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2C_ADDR_W, reg, I2C_MEMADD_SIZE_8BIT, data, len, 3000);

}

void Int_mpu6050_calibration(void)
{
    // 1. 等待陀螺仪稳定  角速度的值小于300
    MPU6050_Gyro tmp_gyro;
    MPU6050_Accel tmp_accel;
    uint8_t count = 0;
    while (1)
    {
        Int_mpu6050_get_gyro(&tmp_gyro);
        if (tmp_gyro.gyro_x < 300 && tmp_gyro.gyro_y < 300 && tmp_gyro.gyro_z < 300)
        {
            count++;
            if (count >= 50)
            {
                break;
            }
        }
        HAL_Delay(10);
    }
    // 2. 采样稳定数据 => 记录为偏移量 消除零点误差
    int32_t gyro_x_offset = 0;
    int32_t gyro_y_offset = 0;
    int32_t gyro_z_offset = 0;
    int32_t accel_x_offset = 0;
    int32_t accel_y_offset = 0;
    int32_t accel_z_offset = 0;
    for (uint8_t i = 0; i < 50; i++)
    {
        // 角速度误差
        Int_mpu6050_get_gyro(&tmp_gyro);
        gyro_x_offset += tmp_gyro.gyro_x;
        gyro_y_offset += tmp_gyro.gyro_y;
        gyro_z_offset += tmp_gyro.gyro_z;

        // 加速度误差
        Int_mpu6050_get_accel(&tmp_accel);
        accel_x_offset += tmp_accel.accel_x;
        accel_y_offset += tmp_accel.accel_y;
        accel_z_offset += (tmp_accel.accel_z - 16384);
        HAL_Delay(10);
    }

    calibration_gyro.gyro_x = gyro_x_offset / 50;
    calibration_gyro.gyro_y = gyro_y_offset / 50;
    calibration_gyro.gyro_z = gyro_z_offset / 50;
    calibration_accel.accel_x = accel_x_offset / 50;
    calibration_accel.accel_y = accel_y_offset / 50;
    calibration_accel.accel_z = accel_z_offset / 50;
}

/**
 * @brief 初始化MPU6050
 *
 */
void Int_mpu6050_init(void)
{

    // 使用i2c读取mpu6050的who am i寄存器 => 0x68表示通信正常
    // uint8_t data = 0;
    // HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2C_ADDR_W, MPU6050_WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
    // debug_printf("data: %d", data);

    // 1. 复位mpu6050
    Int_mpu6050_write_reg(MPU6050_POWER_MANAGEMENT_1, 0x80);
    HAL_Delay(100);
    // 2. 等待复位成功
    uint8_t data = Int_mpu6050_read_reg(MPU6050_POWER_MANAGEMENT_1);
    while (data != 0x40)
    {
        data = Int_mpu6050_read_reg(MPU6050_POWER_MANAGEMENT_1);
    }
    // 3. 设置电池管理1寄存器 => 使用外部的X时钟 可以提高频率
    Int_mpu6050_write_reg(MPU6050_POWER_MANAGEMENT_1, 0x01);

    // 4. 设置陀螺仪量程  2000dps
    Int_mpu6050_write_reg(MPU6050_GYRO_CONFIG, 0x03 << 3);
    // 5. 设置加速度量程  2g
    Int_mpu6050_write_reg(MPU6050_ACCEL_CONFIG, 0x00);
    // 6. 关闭所有的中断
    Int_mpu6050_write_reg(MPU6050_INT_ENABLE, 0x00);
    // 7. 用户控制寄存器 关闭FIFO 关闭I2C主模式 关闭DMP
    Int_mpu6050_write_reg(MPU6050_USER_CTRL, 0x00);
    // 8. FIFO使能寄存器
    Int_mpu6050_write_reg(MPU6050_FIFO_EN, 0x00);
    // 9. 设置采样分频寄存器 200HZ
    Int_mpu6050_write_reg(MPU6050_SMPLRT_DIV, 0x04);
    // 10. 配置低通滤波器
    Int_mpu6050_write_reg(MPU6050_CONFIG, 0x02);
   
    // 11. 使能加速度传感器和角速度传感器
    Int_mpu6050_write_reg(MPU6050_POWER_MANAGEMENT_2, 0x00);

    // 12. 记录下设置的零点误差
    Int_mpu6050_calibration();
}

/**
 * @brief 获取陀螺仪数据
 *
 * @param gyro
 */
void Int_mpu6050_get_gyro(MPU6050_Gyro *gyro)
{
    // 提高数据读取的效率 => 一次性读取多个寄存器的值
    uint8_t buff[6] = {0};
    Int_mpu6050_read_regs(MPU6050_GYRO_XOUT_H, buff, 6);
    gyro->gyro_x = (int16_t)(buff[0] << 8 | buff[1]);
    gyro->gyro_y = (int16_t)(buff[2] << 8 | buff[3]);
    gyro->gyro_z = (int16_t)(buff[4] << 8 | buff[5]);
}

/**
 * @brief 获取陀螺仪数据 使用低通滤波  用于后续的数据获取
 *
 * @param gyro
 */
void Int_mpu6050_get_gyro_with_filter(MPU6050_Gyro *gyro)
{
    // 提高数据读取的效率 => 一次性读取多个寄存器的值
    uint8_t buff[6] = {0};
    Int_mpu6050_read_regs(MPU6050_GYRO_XOUT_H, buff, 6);
    // for (uint8_t i = 0; i < 6; i++)
    // {
    //     buff[i] = Int_mpu6050_read_reg(MPU6050_GYRO_XOUT_H + i);    
    // }
    
    int16_t gyro_x_tmp = 0;
    int16_t gyro_y_tmp = 0;
    int16_t gyro_z_tmp = 0;

    gyro_x_tmp = (int16_t)(buff[0] << 8 | buff[1]) - calibration_gyro.gyro_x;
    gyro_y_tmp = (int16_t)(buff[2] << 8 | buff[3]) - calibration_gyro.gyro_y;
    gyro_z_tmp = (int16_t)(buff[4] << 8 | buff[5]) - calibration_gyro.gyro_z;

    // 对结果数据进行低通滤波 => 低通滤波的第一个测量值最好是0  稳定之后再使用
    gyro->gyro_x = Com_Filter_LowPass(gyro_x_tmp, gyro->gyro_x);
    gyro->gyro_y = Com_Filter_LowPass(gyro_y_tmp, gyro->gyro_y);
    gyro->gyro_z = Com_Filter_LowPass(gyro_z_tmp, gyro->gyro_z);
}

/**
 * @brief 获取加速度计数据
 *
 * @param accel
 */
void Int_mpu6050_get_accel(MPU6050_Accel *accel)
{
    uint8_t buff[6] = {0};

    Int_mpu6050_read_regs(MPU6050_ACCEL_XOUT_H, buff, 6);

    accel->accel_x = (int16_t)(buff[0] << 8 | buff[1]);

    accel->accel_y = (int16_t)(buff[2] << 8 | buff[3]);

    accel->accel_z = (int16_t)(buff[4] << 8 | buff[5]);
}

/**
 * @brief 获取加速度计数据  使用卡尔曼滤波  用于毛刺更多 抖动更大的数据源
 *
 * @param accel
 */
void Int_mpu6050_get_accel_with_filter(MPU6050_Accel *accel)
{
    uint8_t buff[6] = {0};

    Int_mpu6050_read_regs(MPU6050_ACCEL_XOUT_H, buff, 6);

    // for (uint8_t i = 0; i < 6; i++)
    // {
    //     buff[i] = Int_mpu6050_read_reg(MPU6050_ACCEL_XOUT_H + i);
    // }
    
    int16_t accel_x_tmp = (int16_t)(buff[0] << 8 | buff[1]) - calibration_accel.accel_x;

    int16_t accel_y_tmp = (int16_t)(buff[2] << 8 | buff[3]) - calibration_accel.accel_y;

    int16_t accel_z_tmp = (int16_t)(buff[4] << 8 | buff[5]) - calibration_accel.accel_z;

    accel->accel_x = Com_Filter_KalmanFilter(&kfs[0], accel_x_tmp);

    accel->accel_y = Com_Filter_KalmanFilter(&kfs[1], accel_y_tmp);

    accel->accel_z = Com_Filter_KalmanFilter(&kfs[2], accel_z_tmp);
}
