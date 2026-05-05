#ifndef __APP_LOCATION__
#define __APP_LOCATION__

#include "Int_AT6558R.h"
#include "Int_DS3553.h"
#include "Int_QS100.h"
#include "Int_LoRa.h"
#include "cJSON.h"

void App_Location_Get_GPS_Data(void);

void App_Location_Get_Step_Count(void);

void App_Location_Data_to_JSON(void);

void App_Location_Send_Data(void);

#endif // __APP_LOCATION__
