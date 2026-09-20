
#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stdio.h"

#include "LED.h"
#include "Timer.h"
#include "KEY.h"
#include "Delay.h"
#include "ADC.h"
#include "UART.h"
#include "IWDG.h"

#define FEATURE_COUNT			3					//模式总数
#define NEXT_FEATURE			(Mode_Num + 1)		//切换下一个模式
#define FIRST_FEATURE			1					//返回第一个模式
#define KEY_SCAN_MS     		10					//按键扫描周期
#define	LOG_REFRESH_MS  		2000				//日志刷新周期
#define RISING_EDGE				1					//按下按钮产生上升沿标志位



#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

const char *IWDG_String = "\n系统卡死，正在重启\n";		//存放系统卡死输出字符
ADC_Value Read_Value;								//存放三个传感器输出数值
volatile uint8_t Log_Refresh_SW = 1;				//初始化日志刷新标志位
static uint8_t Key_Status; 							//按键事件状态
static uint8_t Mode_Num = 1;						//当前模式，初值 1


int main(void)
{

	//初始化对应外设
	LED_Init();
	Timer_Init();
	Key_Init();
	ADC_INIT();
	UART_Init();
	IWDG_Init();

	/*判断看门狗重启标志位，看是不是看门狗重启*/
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
	{
		UART_SendString((uint8_t*)IWDG_String);
		RCC_ClearFlag();//清空标志位
	}

	while(1)
	{
		IWDG_ReloadCounter();		//喂狗
		IWDG_Test();				//按键（PC13）阻塞程序，验证看门狗功能是否在工作

		//一圈只采一次，后续复用，避免滤波窗口被重复推进
		Key_Status = Key_GetKeyNum();
		Read_Value.Pot   = Pot_GetValue();
		Read_Value.Temp  =Temp_GetValue();
		Read_Value.Light = Light_GetValue();

		if(Key_Status == RISING_EDGE)	//当检测到按键处于上升沿时
			Mode_Num = (Mode_Num < FEATURE_COUNT) ? NEXT_FEATURE : FIRST_FEATURE;

		//模式选择
		switch(Mode_Num)
		{
				/*电位器调光模式*/
			case Pot_Mode:		GPIO_SetBits(GPIOA,GPIO_Pin_3);				//关闭光敏夜灯模式的LED
								TIM_SetCompare2(TIM2, Read_Value.Pot);		//控制对应LED的PWM占空比来控制亮度
								break;
				/*温度报警模式*/
			case Temp_Mode:		if(Read_Value.Temp >= TEMP_WARNING_VALUE)	//判断是否高于预设值
									GPIO_SetBits(GPIOA,GPIO_Pin_2);			//蜂鸣器响
								else
									GPIO_ResetBits(GPIOA,GPIO_Pin_2);		//蜂鸣器不响
								break;
				/*光敏夜灯模式*/
			case Light_Mode:	GPIO_ResetBits(GPIOA,GPIO_Pin_2);			//切出温度报警模式时关闭蜂鸣器
								if(Read_Value.Light < MINIMUM_BRIGHT_VALUE)	//判断亮度是否低于预设值
									GPIO_ResetBits(GPIOA,GPIO_Pin_3);		//开灯
								else
									GPIO_SetBits(GPIOA,GPIO_Pin_3);			//关灯
								break;
		}

		UART_Log_Display(&Log_Refresh_SW, Mode_Num, &Read_Value);			//串口日志显示系统的状态与信息
	}
}



/* 1ms 中断：按键扫描 + 日志刷新标志 */
void TIM3_IRQHandler(void)
{
	static uint16_t con_1 = 0;							//按键扫描周期计时
	static uint16_t con_2 = 0;							//串口日志刷新周期计时
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		con_1++;
		con_2++;
		if(con_1 >= KEY_SCAN_MS)
		{
			Key_Scan();
			con_1 = 0;
		}

		if(con_2 >= LOG_REFRESH_MS)
		{
			Log_Refresh_SW = 1;							//置串口日志刷新标志
			con_2 = 0;
		}

	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);			//清空定时器中断标志位
}
