#include "App_upload_data.h"

uint8_t connect_flag = 0;
uint8_t socket_id = 0xff;

extern uint32_t step_count;

/**
 * @brief 初始化上传数据相关代码
 *
 */
void App_Upload_Data_init(void)
{
    // 初始化相关的接口
    Int_GPS_Init();
    Int_llcc68_init();
    Int_qs100_init();
    Int_step_init();

    // 首先创建一次连接
    IOT_Status ret = Int_qs100_create_connection(&socket_id);
    if (ret == IOT_OK)
    {
        connect_flag = 1;
    }
}

void static App_upload_func(uint8_t *data, uint16_t len)
{
    uint8_t count = 0;
    // 判断当前是否有连接
    if (connect_flag)
    {
        // 已经创建好连接了 => 可以直接使用 => 失败重试再发一次
        while (Int_qs100_send(socket_id, data, len) == IOT_ERROR && count < 2)
        {
            count++;
        }
        if (count >= 2)
        {
            // 调用方法关闭socket
            Int_qs100_close_socket(socket_id);
            connect_flag = 0;
            // QS100的IOT网发送失败  还可以使用LLCC68的LoRa网发送
            Int_llcc68_send(data, len);
        }
    }
    else
    {
        // 需要创建连接
        if (Int_qs100_create_connection(&socket_id) == IOT_OK)
        {
            // 创建成功
            connect_flag = 1;
        }
    }
}

/**
 * @brief 上传数据 json类型
 * 保存数据的结构 GPS_Data gps_data 其余信息使用变量 steps uuid(主控芯片编号 绝对独一无二)
 *
 */
void App_upload_data(uint8_t status)
{
    // uint32_t step_count = Int_step_get_count();
    uint32_t uuid = (HAL_GetUIDw2() << 16) | (HAL_GetUIDw1() << 8) | HAL_GetUIDw0();

    // 构建上传的json数据
    // 1. 创建json对象
    cJSON *root = cJSON_CreateObject();

    // 添加当前状态 是否为告警
    cJSON_AddNumberToObject(root, "status", status);

    // 2. 添加内容到json中   数据类型不同调用的API不同
    cJSON_AddNumberToObject(root, "step", step_count);
    cJSON_AddNumberToObject(root, "uuid", uuid);
    // 添加gps信息 => 修改小数的精度
    uint8_t lat_buf[16] = {0};
    sprintf((char *)lat_buf, "%.4f", gps_data.latitude);
    cJSON_AddStringToObject(root, "lat", (char *)lat_buf);
    uint8_t lon_buf[16] = {0};
    sprintf((char *)lon_buf, "%.4f", gps_data.longitude);
    cJSON_AddStringToObject(root, "lon", (char *)lon_buf);
    // 将方向转换为字符串
    char lat_dir[3] = {0};
    lat_dir[0] = gps_data.lat_dir;
    lat_dir[1] = '\0';
    cJSON_AddStringToObject(root, "lat_dir", lat_dir);
    char lon_dir[3] = {0};
    lon_dir[0] = gps_data.long_dir;
    lon_dir[1] = '\0';
    cJSON_AddStringToObject(root, "lon_dir", lon_dir);
    // 添加时间
    cJSON_AddStringToObject(root, "date", gps_data.date);
    cJSON_AddStringToObject(root, "time", gps_data.time);
    // 添加速度
    cJSON_AddNumberToObject(root, "speed", gps_data.speed_kph);

    // 添加水平定位精度因子
    uint8_t hdop_buf[16] = {0};
    sprintf((char *)hdop_buf, "%.2f", gps_data.hdop);
    cJSON_AddStringToObject(root, "hdop", (char *)hdop_buf);
    // 添加定位状态
    cJSON_AddNumberToObject(root, "fix_satus", gps_data.fix_satus);
    // 添加时间戳
    cJSON_AddNumberToObject(root, "timestamp", gps_data.timestamp);

    // 使用cjson要注意堆空间不足的问题 => 默认只有512字节
    char *json_str = cJSON_PrintUnformatted(root);
    // debug_printf("json_str:%s", json_str);

    // 调用函数  需要判断当前是否有连接的
    App_upload_func((uint8_t *)json_str, strlen(json_str));
    HAL_Delay(50);
    // 清空数据
    cJSON_Delete(root);
    cJSON_free(json_str);
}
