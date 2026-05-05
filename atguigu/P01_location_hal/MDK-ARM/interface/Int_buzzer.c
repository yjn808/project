#include "Int_buzzer.h"

/**
 * @brief 打开蜂鸣器 发出声音
 *
 */
void Int_buzzer_on(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

/**
 * @brief 关闭蜂鸣器 停止发出声音
 *
 */
void Int_buzzer_off(void)
{
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

/**
 * @brief 设置当前蜂鸣器为不同的频率
 *
 * @param freq
 */
void Int_buzzer_set(uint16_t freq)
{
    // 因为 时钟周期是72MHz 预分频是7200-1  => 计数的频率10K
    __HAL_TIM_SetAutoreload(&htim3, 10000 / freq);

    // 比较值也需要同时修改 => 保持方波信号高低电平是一般长
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 5000 / freq);

    // 重置count值
    __HAL_TIM_SetCounter(&htim3, 0);
}

static void Int_buzzer_music_byte(uint16_t freq)
{
    Int_buzzer_set(freq);
    Int_buzzer_on();
    HAL_Delay(800);
    Int_buzzer_off();
    HAL_Delay(200);
}

static void Int_buzzer_music_byte_long(uint16_t freq)
{
    Int_buzzer_set(freq);
    Int_buzzer_on();
    HAL_Delay(1600);
    Int_buzzer_off();
    HAL_Delay(400);
}


/**
 * @brief 发送小星星的音乐声音
 *
 */
void Int_buzzer_music(void)
{
    // 每一个音节 响0.8s 间隔0.2s  => 5- 响1.6s  间隔0.4
    Int_buzzer_music_byte(BUZZER_FREQ_1);
    Int_buzzer_music_byte(BUZZER_FREQ_1);
    Int_buzzer_music_byte(BUZZER_FREQ_5);
    Int_buzzer_music_byte(BUZZER_FREQ_5);
    Int_buzzer_music_byte(BUZZER_FREQ_6);
    Int_buzzer_music_byte(BUZZER_FREQ_6);
    Int_buzzer_music_byte_long(BUZZER_FREQ_5);

    Int_buzzer_music_byte(BUZZER_FREQ_4);
    Int_buzzer_music_byte(BUZZER_FREQ_4);
    Int_buzzer_music_byte(BUZZER_FREQ_3);
    Int_buzzer_music_byte(BUZZER_FREQ_3);
    Int_buzzer_music_byte(BUZZER_FREQ_2);
    Int_buzzer_music_byte(BUZZER_FREQ_2);
    Int_buzzer_music_byte_long(BUZZER_FREQ_1);
}
