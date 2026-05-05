#ifndef __INT_LED_H
#define __INT_LED_H

#include "main.h"

// 24bit数据表示不同的颜色
#define LED_GREEN 0xFF0000
#define LED_RED 0x00FF00
#define LED_BLUE 0x0000FF

/**
 * @brief 发送数据到LED -> 32位数据只有24位有效
 *
 * @param data 24bit数据表示不同的颜色
 */
void Int_led_send_data(uint32_t data);

/**
 * @brief LED单颜色闪烁  闪烁中亮和灭的时间都是ms
 *
 * @param color
 */
void Int_led_blink(uint32_t color, uint16_t ms);

/**
 * @brief 循环一圈彩虹的颜色  => 每次切换颜色的时间是ms
 *
 * @param ms
 */
void Int_led_rainbow(uint16_t ms);

#endif // !__INT_LED_H
