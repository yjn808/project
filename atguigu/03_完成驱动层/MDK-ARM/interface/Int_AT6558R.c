#include "Int_AT6558R.h"

GPS_Data gps_data = {0};

uint8_t gps_full_buff[GPS_FULL_BUFFER_SIZE] = {0};
uint16_t gps_full_buff_len = 0;

uint8_t gps_line_buff[GPS_LINE_BUFFER_SIZE] = {0};
uint16_t gps_line_buff_len = 0;

/**
 * @brief 初始化GPS定位模块
 *
 */
void Int_GPS_Init(void)
{
}

/**
 * @brief 判断当前一行数据的类型
 *
 * @param buff
 * @return NMEA_Type
 */
static NMEA_Type GPS_Get_Type(char *buff)
{
    NMEA_Type nmea_type = NMEA_UNKNOWN;

    if (strstr(buff, "RMC"))
    {
        nmea_type = NMEA_RMC;
    }
    if (strstr(buff, "VTG"))
    {
        nmea_type = NMEA_VTG;
    }
    if (strstr(buff, "GGA"))
    {
        nmea_type = NMEA_GGA;
    }
    if (strstr(buff, "GSA"))
    {
        nmea_type = NMEA_GSA;
    }
    if (strstr(buff, "GSV"))
    {
        nmea_type = NMEA_GSV;
    }
    if (strstr(buff, "TXT"))
    {
        nmea_type = NMEA_TXT;
    }
    return nmea_type;
}
/**
 * @brief
 *
 * @param line_buff
 * @return uint8_t 0: 校验失败 1: 校验成功
 */
static uint8_t GPS_Check_Sum(char *line_buff)
{
    char *end_flag = strchr(line_buff, '*');
    if (end_flag == NULL || line_buff[0] != '$')
    {
        return 0;
    }

    // 计算校验和
    uint8_t check_sum = 0;
    for (int i = 1; i < end_flag - line_buff; i++)
    {
        check_sum ^= line_buff[i];
    }

    // 读取数据的校验和
    char check_sum_str[3] = {0};
    strncpy(check_sum_str, end_flag + 1, 2);
    // 字符串转换为数字
    uint8_t check_sum_num = strtoul(check_sum_str, NULL, 16);
    debug_printf("check_sum:%02x,check_sum_num:%02x", check_sum, check_sum_num);
    return check_sum == check_sum_num;
}

/**
 * @brief 解析单行RMC数据
 *
 * @param line_buff
 */
static void GPS_Parse_RMC(uint8_t *line_buff)
{
    uint8_t time[7];
    uint8_t date[7];
    char status;
    double latitude;
    char latitude_dir;
    double longitude;
    char longitude_dir;
    sscanf((char *)line_buff, "$GNRMC,%6c%*4c,%c,%lf,%c,%lf,%c,  %*f,%*f,%6c", time, &status, &latitude, &latitude_dir, &longitude, &longitude_dir, date);

    // 打印
    debug_printf("date:%s,time:%s,status:%c,latitude:%lf,latitude_dir:%c,longitude:%lf,longitude_dir:%c", date, time, status, latitude, latitude_dir, longitude, longitude_dir);

    if (status == 'A')
    {
        gps_data.fix_satus = 1;
        // 赋值日期
        gps_data.date[0] = date[4];
        gps_data.date[1] = date[5];
        gps_data.date[2] = date[2];
        gps_data.date[3] = date[3];
        gps_data.date[4] = date[0];
        gps_data.date[5] = date[1];
        // 赋值时间 0时区的时间
        gps_data.time[0] = time[0];
        gps_data.time[1] = time[1];
        gps_data.time[2] = time[2];
        gps_data.time[3] = time[3];
        gps_data.time[4] = time[4];
        gps_data.time[5] = time[5];

        // 经纬度方向
        gps_data.lat_dir = latitude_dir;
        gps_data.long_dir = longitude_dir;

        // 转换经纬度的值 3106.67691 = 31 + 06.67691/60
        // 12113.52666 => 121 + 13.52666/60
        gps_data.latitude = (uint8_t)(latitude / 100) + (latitude - (uint8_t)(latitude / 100) * 100) / 60;
        gps_data.longitude = (uint8_t)(longitude / 100) + (longitude - (uint8_t)(longitude / 100) * 100) / 60;

        // 记录当前的时间戳
        gps_data.timestamp = HAL_GetTick();

        debug_printf("date:%s,time:%s,latitude:%f,lat_dir:%c,longitude:%f,long_dir:%c", gps_data.date, gps_data.time, gps_data.latitude, gps_data.lat_dir, gps_data.longitude, gps_data.long_dir);
    }
    else if (status == 'V')
    {
        gps_data.fix_satus = 0;
    }
}

/**
 * @brief 解析单行GGA数据
 *
 * @param line_buff
 */
static void GPS_Parse_GGA(uint8_t *line_buff)
{
    uint8_t fs;
    uint8_t numSv;
    float HDOP;

    sscanf((char *)line_buff, "$GNGGA,%*f,%*f,%*c,%*f,%*c,  %hhu,%hhu,%f", &fs, &numSv, &HDOP);

    debug_printf("fs:%d,numSv:%d,HDOP:%f", fs, numSv, HDOP);

    // 数据的保存
    if (fs != 0)
    {
        gps_data.satellites = numSv;
        gps_data.hdop = HDOP;
    }
}

static void GPS_Parse_VTG(uint8_t *line_buff)
{

    float kph;
    // sscanf使用方便  兼容性不好
    // sscanf((char *)line_buff, "$GNVTG,%*f,%*c,,%*c,%*f,%*c, %f", &kph);
    // 使用逗号区分的方式兼容性更好  第7个逗号后面的浮点数
    char *tmp = (char *)line_buff;
    for (uint8_t i = 0; i < 7; i++)
    {
        tmp = strstr(tmp, ",");
        tmp++;
        //    debug_printf("%d:%s", i, tmp);
    }
    sscanf(tmp, "%f", &kph);
    // debug_printf("kph:%f", kph);
    gps_data.speed_kph = kph;
}

/**
 * @brief 更新GPS定位数据
 *
 */
void Int_GPS_Update_Data(void)
{
    memset(gps_full_buff, 0, GPS_FULL_BUFFER_SIZE);
    gps_full_buff_len = 0;
    // GPS芯片发送完当次的定位数据之后  会发送一个空闲帧数据 => 当做接收完整数据的标记
    // 1. 达到超时时间 或者 缓冲区接收满了   =>  GPS芯片没有正常工作 / 缓冲区太小
    // 2. 接收到了空闲帧数据  => 接收完成
    HAL_UARTEx_ReceiveToIdle(&huart2, gps_full_buff, GPS_FULL_BUFFER_SIZE, &gps_full_buff_len, 1000);
    if (gps_full_buff_len > 0)
    {
        // debug_printf("GPS数据:\n%s", gps_full_buff);
        // char *buff_p = gps_full_buff;
        char *buff_p = "$GNGGA,081621.000,3106.67691,N,12113.52666,E,1,11,2.9,36.9,M,10.3,M,,*71\n$GNRMC,081621.000,A,3106.67691,N,12113.52666,E,0.00,0.00,040126,,,A,V*0D\n$GNVTG,0.00,T,,M,0.00,N,5.00,K,A*26\n";

        while (1)
        {
            /// 处理数据
            // 1. 寻找一行的开始和结尾
            char *start_p = strstr(buff_p, "$");
            char *end_p = strstr(buff_p, "\n");

            if (start_p == NULL || end_p == NULL)
            {
                debug_printf("GPS数据结束");
                break;
            }

            // 2. 保存当前一行的数据
            gps_line_buff_len = end_p - start_p + 1;
            if (gps_line_buff_len > GPS_LINE_BUFFER_SIZE)
            {
                gps_line_buff_len = GPS_LINE_BUFFER_SIZE;
            }
            memset(gps_line_buff, 0, GPS_LINE_BUFFER_SIZE);
            strncpy((char *)gps_line_buff, start_p, gps_line_buff_len);

            // 3. 判断当前一行的数据类型 => 解析当前行数据
            // 先进行校验
            if (GPS_Check_Sum((char *)gps_line_buff))
            {
                NMEA_Type nmea_type = GPS_Get_Type((char *)gps_line_buff);
                switch (nmea_type)
                {
                case NMEA_RMC:
                    GPS_Parse_RMC(gps_line_buff);
                    break;
                case NMEA_VTG:
                    GPS_Parse_VTG(gps_line_buff);
                    break;
                case NMEA_GGA:
                    GPS_Parse_GGA(gps_line_buff);
                    break;
                case NMEA_GSA:
                    /* code */
                    break;
                case NMEA_GSV:
                    /* code */
                    break;
                case NMEA_TXT:
                    /* code */
                    break;
                default:
                    break;
                }
            }
            else
            {
                debug_printf("GPS数据校验失败");
            }

            // 过渡到下一行
            buff_p = end_p + 1;
        }
    }
}
