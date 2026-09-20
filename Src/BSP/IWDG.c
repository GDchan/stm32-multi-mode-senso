#include "stm32f10x.h"
#include "KEY.h"
#include "Delay.h"
#include "UART.h"


/**
  * @brief  独立看门狗初始化
  * @param  无
  * @retval 无
  * @note   超过 1 秒没进行“喂狗”操作看门狗激活，系统重启
  */
void IWDG_Init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_16);		// LSI = 40000 Hz  ,LSI / 16 = 2500Hz
	IWDG_SetReload(2499);
	IWDG_ReloadCounter();
	IWDG_Enable();

}


/**
  * @brief  看门狗功能验证测试
  * @param  无
  * @retval 无
  * @note   使用阻塞扫描按键事件
  * @note	需要按住测试按键（PC13）超过一秒以上
  */
void IWDG_Test(void)
{
	const char *P_String = "\n->独立看门狗功能测试，请按测试按钮的时间超过1秒以上.......\n";
	if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == SET)
	{
		UART_SendString((uint8_t*)P_String);	//串口打印测试条件
		Delay_ms(15);
		while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == SET);	//阻塞等待按键释放，此期间不喂狗
		Delay_ms(15);
	}
}
