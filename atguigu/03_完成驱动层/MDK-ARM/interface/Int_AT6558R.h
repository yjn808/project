#ifndef __INT_AT6558R__
#define __INT_AT6558R__

#include "stdlib.h"
#include "usart.h"
#include "Com_debug.h"
#define GPS_FULL_BUFFER_SIZE 512
#define GPS_LINE_BUFFER_SIZE 128




// NMEA语句类型
typedef enum
{
    NMEA_RMC,
    NMEA_VTG,
    NMEA_GGA,
    NMEA_GSA,
    NMEA_GSV,
    NMEA_TXT,
    NMEA_UNKNOWN
} NMEA_Type;

// GPS定位数据结构
typedef struct
{
    float latitude;     // 纬度 (十进制格式)
    uint8_t lat_dir;    // 纬度方向
    float longitude;    // 经度 (十进制格式)
    uint8_t long_dir;   // 经度方向
    float speed_kph;    // 速度 (千米/小时)
    float hdop;         // 水平定位精度因子
    char time[7];       // GPS时间
    char date[7];       // GPS日期
    uint8_t satellites; // 卫星数量
    uint8_t fix_satus;  // 定位状态  0:无效  1:有效
    uint32_t timestamp; // 定时数据的时间戳
} GPS_Data;


extern GPS_Data gps_data;

/**
 * @brief 初始化GPS定位模块
 *
 */
void Int_GPS_Init(void);

/**
 * @brief 更新GPS定位数据
 *
 */
void Int_GPS_Update_Data(void);

#endif // __INT_AT6558R__
