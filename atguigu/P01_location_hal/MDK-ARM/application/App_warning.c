#include "App_warning.h"
// 当前步数
uint32_t step_count;
// 疑似跌倒时的步数
uint32_t pre_step_count;
uint32_t pre_tick_count;
// 加速地址
MPU6050_Accel warning_accel;
MPU6050_Gyro warning_gyro;

uint32_t accel_sum[5] = {0};
uint32_t gyro_sum[5] = {0};
uint8_t index = 0;

// 疑似跌倒flag
uint8_t pre_warning_flag = 0;

/**
 * @brief 初始化警告相关的接口
 *
 */
void App_Warning_Init(void)
{
    // 把灯关掉
    Int_led_send_data(0);
    Int_mpu6050_init();
}

/**
 * @brief 获取警告的状态
 *  0 没有警告
 *  1 有警告
 */
uint8_t App_Warning_Get_status(void)
{

    // 对时序要求严格
    // 获取当前加速度值
    Int_mpu6050_get_accel(&warning_accel);
    // 获取角速度值
    Int_mpu6050_get_gyro_with_filter(&warning_gyro);
    HAL_Delay(10);

    // 获取当前计步数
    step_count = Int_step_get_count();

    // 判断前置状态 => 疑似跌倒
    if (pre_warning_flag == 0)
    {
        // 还在平稳状态  没有发送疑似跌倒
        // 加速度绝对值相加
        accel_sum[index] = (uint32_t)(fabsf(warning_accel.accel_x) + fabsf(warning_accel.accel_y) + fabsf(warning_accel.accel_z - 16384));
        // 角速度的值累加
        gyro_sum[index] = (uint32_t)(fabsf(warning_gyro.gyro_x) + fabsf(warning_gyro.gyro_y) + fabsf(warning_gyro.gyro_z));
        debug_printf("%d,%d", accel_sum[index], gyro_sum[index]);
        index++;
        if (index >= 5)
        {
            index = 0;
        }
        pre_warning_flag = 1;
        // 如果数组中的5个数字都是大于阈值  疑似跌倒
        for (uint8_t i = 0; i < 5; i++)
        {
            if (accel_sum[i] < 2.5 * MPU6050_ACCEL_SCALE || gyro_sum[i] < 70 * MPU6050_GYRO_SCALE)
            {
                pre_warning_flag = 0;
            }
        }

        if (pre_warning_flag == 1)
        {
            // 疑似跌倒 => 记录下初始的步数
            pre_step_count = step_count;
            // 记录当前的时间
            pre_tick_count = HAL_GetTick();
            debug_printf("疑似跌倒");
        }
    }
    else
    {
        // 校验是否真的跌倒 2s之内 步数没有大的增加  增加范围在0-1之间
        // 1. 判断时间过了2s
        if (HAL_GetTick() - pre_tick_count > 2000)
        {
            if (step_count - pre_step_count <= 1)
            {
                // 不是老人在跑步 而是跌倒了
                debug_printf("老人跌倒了");
                return 1;
            }
            else
            {
                debug_printf("老人在跑步");
                // 老人在跑步
                pre_warning_flag = 0;
            }
        }
    }

    return 0;
}

/**
 * @brief 发出告警
 *
 */
void App_Warning_run(void)
{
    // 1. 闪灯
    Int_led_blink(LED_RED, 100);

    // 2. 发声音
    Int_buzzer_on();
}
