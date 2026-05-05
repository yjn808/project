#include "App_Location.h"

Upload_Struct upload_data;

void App_Location_Get_GPS_Data(void)
{
    // 使用AT6558R获取定位数据
    Int_AT6558R_Init();

    // 死循环 => 一直等待获取有效的gps信息
    char *gnrmcStart = NULL;
    while (1)
    {
        Int_AT6558R_Read_GPS();
        if (gps_full_buff_len > 0)
        {
            // 读取到当前的一次数据
            /**
               核心定位数据RMC 时间,有效,维度,维度方向,经度,经度方向,速度,航向,日期....
               $GNRMC,013050.000,A,3106.67898,N,12113.52954,E,5.19,77.74,160125,,,A,V*31
           */
            uint8_t tmp = 0;
            gnrmcStart = strstr((char *)gps_full_buff, "$GNRMC");
            // gnrmcStart = "$GNRMC,203050.000,A,3106.67898,N,12113.52954,E,5.19,77.74,310125,,,A,V*31";
            sscanf(gnrmcStart, "%*[^AV]%c", &tmp);
            if (tmp == 'A')
            {
                // 获取到了有效的定位数据
                debug_println("获取到有效的定位数据");
                break;
            }
            else
            {
                // 获取到无效的定位数据
                debug_println("获取到无效的定位数据");
            }
        }
    }

    // 获取到有效的定位数据 => 解析定位数据
    char time[7];
    char date[7];
    double lat, lon;
    uint8_t lat_dir, lon_dir;
    // $GNRMC,%6c     %*6c,   %lf    ,%c, %lf        ,%c,%*lf,%*lf ,%6c
    //"$GNRMC,013050.000,A,3106.67898,N , 12113.52954,E ,5.19,77.74,160125,,,A,V*31"
    sscanf(gnrmcStart, "$GNRMC,%6c%*6c,%lf,%c,%lf,%c,%*f,%*f,%6c",
           time, &lat, (char *)&lat_dir, &lon, (char *)&lon_dir, date);

    // time:013050,lat:3106.678980,lat_dir:N,lon:12113.529540,lon_dir:E,data:160125
    debug_println("time:%s,lat:%lf,lat_dir:%c,lon:%lf,lon_dir:%c,data:%s", time, lat, lat_dir, lon, lon_dir, date);

    // 数据结构转换 yyyy-MM-dd HH:mm:ss
    sprintf((char *)upload_data.dateTime, "20%c%c-%c%c-%c%c %c%c:%c%c:%c%c",
            date[4],
            date[5],
            date[2],
            date[3],
            date[0],
            date[1],
            time[0],
            time[1],
            time[2],
            time[3],
            time[4],
            time[5]);

    // 将0时区的时间转换为北京时间
    Com_UTC_to_Bj_Time(upload_data.dateTime, upload_data.dateTime);

    // 转换经纬度
    // 3106.678980 => 31° 06.678980′ N = 31 + 06.678980/60 = 31.111316°
    // 12113.529540 => 121° 13.529540′ E
    upload_data.lat = (uint8_t)(lat / 100) + (lat - ((uint8_t)(lat / 100)) * 100) / 60;
    upload_data.lon = (uint8_t)(lon / 100) + (lon - ((uint8_t)(lon / 100)) * 100) / 60;

    upload_data.latDir[0] = lat_dir;
    upload_data.lonDir[0] = lon_dir;

    debug_println("datetime:%s,lat:%lf,lon:%lf,latDir:%c,lonDir:%c", upload_data.dateTime, upload_data.lat, upload_data.lon, upload_data.latDir[0], upload_data.lonDir[0]);
}

void App_Location_Get_Step_Count(void)
{
    // 1. 初始化DS3553芯片
    Int_DS3553_Init();

    // 2. 读取运动步数
    uint32_t step = 0;
    Int_DS3553_Get_Step(&step);
    upload_data.stepCount = step;

    debug_println("step:%d", upload_data.stepCount);
}

void App_Location_Data_to_JSON(void)
{
    // 添加UUID
    sprintf((char *)upload_data.uuid, "%08X%08X%08X", HAL_GetUIDw2(), HAL_GetUIDw1(), HAL_GetUIDw0());

    // 使用cjson配套的API即可完成结构体到JSON字符串的转换
    // 1. 创建cjson结构体对象
    cJSON *json = cJSON_CreateObject();

    // 2. 添加结构体数据到JSON对象中
    cJSON_AddStringToObject(json, "uuid", (char *)upload_data.uuid);
    cJSON_AddNumberToObject(json, "lat", upload_data.lat);
    cJSON_AddNumberToObject(json, "lon", upload_data.lon);
    cJSON_AddStringToObject(json, "latDir", (char *)upload_data.latDir);
    cJSON_AddStringToObject(json, "lonDir", (char *)upload_data.lonDir);
    cJSON_AddStringToObject(json, "dateTime", (char *)upload_data.dateTime);
    cJSON_AddNumberToObject(json, "stepCount", upload_data.stepCount);

    // 3. 将JSON对象转换为JSON字符串
    // Unformatted: json字符串没有格式(没有换行符号)
    // Print: 有换行符号
    char *json_str = cJSON_PrintUnformatted(json);
    upload_data.dataLen = strlen(json_str);
    memcpy(upload_data.data, (uint8_t *)json_str, upload_data.dataLen);
    debug_println("json_str:%s", upload_data.data);

    // 4. 释放堆内存 => C语言特色 用户自己管理堆内存
    cJSON_Delete(json);
    // 5. 清除常量池中的字符串
    cJSON_free(json_str);
}

void App_Location_Send_Data(void)
{
    // 1. 发送数据到IOT网络
    Int_QS100_Init();
    
    // 现在的逻辑是 优先使用IOT进行发送数据 => 成功后直接返回 => 失败后使用LoRa进行发送数据
    CommmonStatus cs = Int_QS100_SendData(upload_data.data, upload_data.dataLen);
    
    // 测试LoRa发送数据
    cs = COMMON_ERROR;
    if (cs == COMMON_OK)
    {
        debug_println("IOT发送数据成功");
    }
    else
    {
        debug_println("IOT发送数据失败");

        // 2. 发送数据到LoRa通信
        Int_LoRa_Init();
        // LoRa一次通信能够传输的字符串长度有限 => 需要进行分片处理
        uint8_t tmp = 0;

        // 如果数据长度超过255 分批发送
        while (upload_data.dataLen > 255)
        {
            Int_LoRa_Send_Data(&(upload_data.data[tmp * 255]), 255);
            tmp++;
            upload_data.dataLen -= 255;
        }

        // 最后一次发送剩余的数据
        Int_LoRa_Send_Data(&(upload_data.data[tmp * 255]), upload_data.dataLen);
    }
}
