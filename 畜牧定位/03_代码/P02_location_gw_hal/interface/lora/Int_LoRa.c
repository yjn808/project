#include "Int_LoRa.h"

llcc68_handle_t gs_handle = {
    .spi_init = llcc68_interface_spi_init,
    .spi_deinit = llcc68_interface_spi_deinit,
    .spi_write_read = llcc68_interface_spi_write_read,
    .reset_gpio_init = llcc68_interface_reset_gpio_init,
    .reset_gpio_deinit = llcc68_interface_reset_gpio_deinit,
    .reset_gpio_write = llcc68_interface_reset_gpio_write,
    .busy_gpio_init = llcc68_interface_busy_gpio_init,
    .busy_gpio_deinit = llcc68_interface_busy_gpio_deinit,
    .busy_gpio_read = llcc68_interface_busy_gpio_read,
    .delay_ms = llcc68_interface_delay_ms,
    .debug_print = llcc68_interface_debug_print,
    .receive_callback = llcc68_interface_receive_callback,
};

void Int_LoRa_Init(void)
{
    uint8_t res;
    uint32_t reg;
    uint8_t modulation;
    uint8_t config;

    // 判断结构体内部的函数指针是否完整
    /* init the llcc68 */
    res = llcc68_init(&gs_handle);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: init failed.\n");
    }

    // 进入待机模式
    /* enter standby */
    res = llcc68_set_standby(&gs_handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set standby failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 停止定时器
    /* set stop timer on preamble */
    res = llcc68_set_stop_timer_on_preamble(&gs_handle, LLCC68_LORA_DEFAULT_STOP_TIMER_ON_PREAMBLE);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: stop timer on preamble failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 配置电源模式
    /* set regulator mode */
    res = llcc68_set_regulator_mode(&gs_handle, LLCC68_LORA_DEFAULT_REGULATOR_MODE);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set regulator mode failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 配置功率
    /* set pa config */
    res = llcc68_set_pa_config(&gs_handle, LLCC68_LORA_DEFAULT_PA_CONFIG_DUTY_CYCLE, LLCC68_LORA_DEFAULT_PA_CONFIG_HP_MAX);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set pa config failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 进入修改模式
    /* enter to stdby xosc mode */
    res = llcc68_set_rx_tx_fallback_mode(&gs_handle, LLCC68_RX_TX_FALLBACK_MODE_STDBY_XOSC);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set rx tx fallback mode failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    // 设置lora模式
    /* set lora mode */
    res = llcc68_set_packet_type(&gs_handle, LLCC68_PACKET_TYPE_LORA);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set packet type failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置输出的参数
    /* set tx params */
    res = llcc68_set_tx_params(&gs_handle, LLCC68_LORA_DEFAULT_TX_DBM, LLCC68_LORA_DEFAULT_RAMP_TIME);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set tx params failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置调制解调参数
    /* set lora modulation params */
    res = llcc68_set_lora_modulation_params(&gs_handle, LLCC68_LORA_DEFAULT_SF, LLCC68_LORA_DEFAULT_BANDWIDTH,
                                            LLCC68_LORA_DEFAULT_CR, LLCC68_LORA_DEFAULT_LOW_DATA_RATE_OPTIMIZE);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set lora modulation params failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 转换频率
    /* convert the frequency */
    res = llcc68_frequency_convert_to_register(&gs_handle, LLCC68_LORA_DEFAULT_RF_FREQUENCY, (uint32_t *)&reg);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: convert to register failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置频率
    /* set the frequency */
    res = llcc68_set_rf_frequency(&gs_handle, reg);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set rf frequency failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置基地址
    /* set base address */
    res = llcc68_set_buffer_base_address(&gs_handle, 0x00, 0x00);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set buffer base address failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置同步的超时时间
    /* set lora symb num */
    res = llcc68_set_lora_symb_num_timeout(&gs_handle, LLCC68_LORA_DEFAULT_SYMB_NUM_TIMEOUT);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set lora symb num timeout failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 重置状态
    /* reset stats */
    res = llcc68_reset_stats(&gs_handle, 0x0000, 0x0000, 0x0000);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: reset stats failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 清除错误
    /* clear device errors */
    res = llcc68_clear_device_errors(&gs_handle);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: clear device errors failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 重要: 设置同步字 同步字相同的设备才能通信
    /* set the lora sync word */
    res = llcc68_set_lora_sync_word(&gs_handle, LLCC68_LORA_DEFAULT_SYNC_WORD);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set lora sync word failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 输出调制
    /* get tx modulation */
    res = llcc68_get_tx_modulation(&gs_handle, (uint8_t *)&modulation);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: get tx modulation failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    modulation |= 0x04;

    /* set the tx modulation */
    res = llcc68_set_tx_modulation(&gs_handle, modulation);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set tx modulation failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置输入的增益
    /* set the rx gain */
    res = llcc68_set_rx_gain(&gs_handle, LLCC68_LORA_DEFAULT_RX_GAIN);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set rx gain failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置过电流保护
    /* set the ocp */
    res = llcc68_set_ocp(&gs_handle, LLCC68_LORA_DEFAULT_OCP);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set ocp failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 发射器限流配置
    /* get the tx clamp config */
    res = llcc68_get_tx_clamp_config(&gs_handle, (uint8_t *)&config);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: get tx clamp config failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    config |= 0x1E;

    /* set the tx clamp config */
    res = llcc68_set_tx_clamp_config(&gs_handle, config);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set tx clamp config failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    // 设置默认的LoRa模式 => 接收模式
    Int_LoRa_Enter_RX_Mode();
}

/**
 * return: 1 失败 0 成功
 */
uint8_t Int_LoRa_Enter_RX_Mode(void)
{
    // 变换引脚
    HAL_GPIO_WritePin(LoRa_RXEN_GPIO_Port, LoRa_RXEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LoRa_TXEN_GPIO_Port, LoRa_TXEN_Pin, GPIO_PIN_RESET);

    uint8_t setup;

    /* set dio irq */
    if (llcc68_set_dio_irq_params(&gs_handle, LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  0x0000, 0x0000) != 0)
    {
        return 1;
    }

    /* clear irq status */
    if (llcc68_clear_irq_status(&gs_handle, 0x03FFU) != 0)
    {
        return 1;
    }

    /* set lora packet params */
    if (llcc68_set_lora_packet_params(&gs_handle, LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH,
                                      LLCC68_LORA_DEFAULT_HEADER, LLCC68_LORA_DEFAULT_BUFFER_SIZE,
                                      LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ) != 0)
    {
        return 1;
    }

    /* get iq polarity */
    if (llcc68_get_iq_polarity(&gs_handle, (uint8_t *)&setup) != 0)
    {
        return 1;
    }

    setup |= 1 << 2;

    /* set the iq polarity */
    if (llcc68_set_iq_polarity(&gs_handle, setup) != 0)
    {
        return 1;
    }

    /* start receive */
    if (llcc68_continuous_receive(&gs_handle) != 0)
    {
        return 1;
    }

    return 0;
}

uint8_t Int_LoRa_Enter_TX_Mode(void)
{
    // 变换引脚
    HAL_GPIO_WritePin(LoRa_RXEN_GPIO_Port, LoRa_RXEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LoRa_TXEN_GPIO_Port, LoRa_TXEN_Pin, GPIO_PIN_SET);

    /* set dio irq */
    if (llcc68_set_dio_irq_params(&gs_handle, LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  0x0000, 0x0000) != 0)
    {
        return 1;
    }

    /* clear irq status */
    if (llcc68_clear_irq_status(&gs_handle, 0x03FFU) != 0)
    {
        return 1;
    }

    return 0;
}

uint8_t Int_LoRa_Send_Data(uint8_t *pData, uint16_t Size)
{
    // 切换到发送模式
    Int_LoRa_Enter_TX_Mode();

    /* send the data */
    if (llcc68_lora_transmit(&gs_handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ,
                             LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH, LLCC68_LORA_DEFAULT_HEADER,
                             LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ,
                             (uint8_t *)pData, Size, 0) != 0)
    {
        Int_LoRa_Enter_RX_Mode();
        return 1;
    }

    Int_LoRa_Enter_RX_Mode();

    return 0;
}

uint8_t Int_LoRa_Receive_Data(uint8_t *pData, uint16_t *Size)
{
    // 调用处理函数
    if (llcc68_irq_handler(&gs_handle) != 0)
    {
        return 1;
    }

    // 判断接收到数据
    if (gs_handle.receive_len > 0)
    {
        // 接收到数据
        memcpy(pData, gs_handle.receive_buf, gs_handle.receive_len);
        *Size = gs_handle.receive_len;

        pData[*Size] = '\0';
        // 处理完接收数据 清空缓冲区
        memset(gs_handle.receive_buf, 0, sizeof(gs_handle.receive_buf));
        gs_handle.receive_len = 0;
    }
    return 0;
}
