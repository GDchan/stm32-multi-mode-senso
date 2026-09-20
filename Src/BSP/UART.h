
#ifndef BSP_UART_H_
#define BSP_UART_H_

#include "ADC.h"

void UART_Init(void);
void UART_SendByte(uint8_t Data);
void UART_SendString(uint8_t *String);
void UART_Log_Display(volatile uint8_t *Refresh, uint8_t Mode_Num, ADC_Value *ADC_Struct);

#endif /* BSP_UART_H_ */
