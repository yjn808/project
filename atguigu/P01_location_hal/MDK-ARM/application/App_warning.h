#ifndef __APP_WARNING__
#define __APP_WARNING__

#include "Int_led.h"
#include "Int_buzzer.h"
#include "Int_mpu6050.h"
#include "Int_step.h"
#include "stdlib.h"
#include "math.h"
/**
 * @brief 初始化警告相关的接口
 *
 */
void App_Warning_Init(void);

/**
 * @brief 获取警告的状态
 *  0 没有警告
 *  1 有警告
 */
uint8_t App_Warning_Get_status(void);

/**
 * @brief 发出告警
 *
 */
void App_Warning_run(void);

#endif // __APP_WARNING__
