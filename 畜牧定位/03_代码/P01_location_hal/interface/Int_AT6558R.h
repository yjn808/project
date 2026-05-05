#ifndef __INT_AT6558R__
#define __INT_AT6558R__

#include "Com_Debug.h"
#include "usart.h"
#include "Com_Tool.h"
#define GPS_BUFF_MAX_LEN 512
#define GPS_FULL_BUFF_MAX_LEN 1024

extern uint8_t gps_full_buff[GPS_BUFF_MAX_LEN];
extern uint16_t gps_full_buff_len;

void Int_AT6558R_Init(void);

void Int_AT6558R_Read_GPS(void);

void Int_AT6558R_Enter_Low_Power(void);

void Int_AT6558R_Exit_Low_Power(void);

#endif // __INT_AT6558R__
