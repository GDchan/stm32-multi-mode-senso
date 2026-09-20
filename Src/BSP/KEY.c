#include "stm32f10x.h"

volatile static uint8_t Key_Num;		//按键事件标志：检测到按下沿置 1，被读走后清零


/**
  * @brief  初始化按键
  * @param  无
  * @retval 无
  * @note   使用引脚为：PA0、PC13
  */
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;					//PA0按键为模式切换按键
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;					//PC13按键为看门狗测试按键
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
  * @brief  读取按键引脚电平
  * @param  无
  * @retval 0 或 1
  * @note   硬件外部下拉、按下接 3V3，故按下为高电平、松开为低电平
  */
uint8_t Key_GetValue(void)
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == SET) return 1;		//按下高电平返回 1
	else 												return 0;		//其他状态返回 0
}


/**
  * @brief  扫描按键
  * @param  无
  * @retval 无
  * @note   函数运行在定时器中断函数，每10ms运行一次
  * @note   检测到上升沿时置 1
  */
void Key_Scan(void)
{
	static uint8_t prev_state = 0, this_state = 0;			//储存上一次扫描与本次扫描的状态
	prev_state = this_state;
	this_state = Key_GetValue();

	//上一次是低电平、本次是高电平，判断为上升沿，Key_Num 置 1
	if(prev_state == 0 && this_state == 1) Key_Num = 1;

}


/**
  * @brief  读取按键状态
  * @param  无
  * @retval 0 = 无，1 = 有按键事件
  * @note   运行在主循环，非阻塞判断按键事件
  */
uint8_t Key_GetKeyNum(void)
{
	uint8_t Value;
	__disable_irq();	//关中断服务，防止读清之间被中断插入导致丢事件
	Value = Key_Num;
	Key_Num = 0;
	__enable_irq();		//打开中断服务
	return Value;
}


