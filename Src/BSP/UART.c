#include "stm32f10x.h"
#include "stdio.h"
#include "ADC.h"
#include "string.h"

#define REFRESH_OFF			0

/**
  * @brief  串口 1 初始化
  * @param  无
  * @retval 无
  * @note   引脚使用的是 PA9作为Tx
  * @note	没有配置 RX
  */
void UART_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_Mode = USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &USART_InitStruct);

	USART_Cmd(USART1, ENABLE);
}


/**
  * @brief  串口 1 输出 1 位字符
  * @param  Data：要输出的字符
  * @retval 无
  */
void UART_SendByte(uint8_t Data)
{
	USART_SendData(USART1, Data);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	//检测是否发送完毕
}


/**
  * @brief  串口 1 输出 字符串
  * @param  String：要输出的字符串首地址
  * @retval 无
  * @note   一直发送直达字符串里的 '\0'结束
  */
void UART_SendString(uint8_t *String)
{
	uint8_t i;
	for(i = 0; String[i] != '\0'; i++)
	{
		UART_SendByte(String[i]);
	}
}

/**
  * @brief  系统串口日志
  * @param  Refresh：刷新数据标志位
  * @param  Mode_Num：系统正在运行的模式
  * @param 	ADC_Struct ：传感器数据结构体地址
  * @retval 无
  * @note   标志位驱动：*Refresh 非 0 才输出，发送完自动清零
  */
void UART_Log_Display(volatile uint8_t *Refresh, uint8_t Mode_Num, ADC_Value *ADC_Struct)
{
	static uint8_t Log_String[268];
	static uint8_t Mode_Name[64];
	uint8_t *p = Log_String;


	if(*Refresh == REFRESH_OFF)return;

	switch(Mode_Num)
	{
		case Pot_Mode: 		sprintf ((char*)Mode_Name, "电位器调光");    										break;
		case Temp_Mode: 	sprintf ((char*)Mode_Name, "温度报警（报警条件 >= %d℃）",	TEMP_WARNING_VALUE);    	break;
		case Light_Mode:	sprintf ((char*)Mode_Name, "光敏夜灯（开灯条件 <= %d%%）", 	MINIMUM_BRIGHT_VALUE);     	break;
	}

	p += sprintf ((char*)p,   "\n/*****系统状态日志*****/\r\n");
	p += sprintf ((char*)p,   "-> 电位器控制PWM占空比：	%d/999\r\n", 	ADC_Struct->Pot);
	p += sprintf ((char*)p,   "-> 热敏电阻检测温度为：	%d℃\r\n", 			ADC_Struct->Temp);
	p += sprintf ((char*)p,   "-> 光敏电阻检测光照强度：	%d%%\r\n\n",	ADC_Struct->Light);

	p += sprintf ((char*)p,   "-> 现在是模式为：	%s \r\n",	Mode_Name);
	UART_SendString((uint8_t*)Log_String);
	*Refresh = REFRESH_OFF;
}

