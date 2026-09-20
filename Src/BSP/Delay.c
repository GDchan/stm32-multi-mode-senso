#include "stm32f10x.h"

/**
  * @brief  阻塞延时
  * @param  time: 1 ~ 65535（单位：ms）
  * @retval 无
  * @note   单片机系统时钟运行在72MHz
  */
void Delay_ms(uint16_t time)
{
	volatile uint16_t i;
    while(time--)
    {
    	i = 6600;	//是在Debug -O0下标定的系数
    	while(i--);
    }
}
