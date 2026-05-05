#ifndef __INT_QS100__
#define __INT_QS100__

#include "usart.h"
#include "Com_Tool.h"
#include "string.h"
#include "Com_Debug.h"
#include "Com_Config.h"

void Int_QS100_Init(void);

CommmonStatus Int_QS100_SendData(uint8_t *pData, uint16_t len);

void Int_QS100_Enter_Low_Power(void);

void Int_QS100_Exit_Low_Power(void);

#endif // __INT_QS100__
