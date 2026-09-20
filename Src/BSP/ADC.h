/*
 * ADC.h
 *
 *  Created on: 2026年9月3日
 *      Author: 28066
 */

#ifndef BSP_ADC_H_
#define BSP_ADC_H_

#define TEMP_WARNING_VALUE		30		//温度报警阈值（℃）
#define MINIMUM_BRIGHT_VALUE	30		//夜灯触发阈值（光照强度 %）

/*工作模式，初值 1 对应 Pot_Mode*/
typedef enum {
	Pot_Mode = 1,		//模式1：电位器调光
	Temp_Mode,			//模式2：温度报警
	Light_Mode,			//模式3：光敏夜灯
}Mode_t;

/*三路传感器的输出值，供日志与模式逻辑共用*/
typedef struct{
	uint16_t Pot;		//PWM 占空比 0 ~ 999
	uint16_t Temp;		//温度 0 ~ 60（℃）
	uint16_t Light;		//光照强度 0 ~ 100（%）
}ADC_Value;

void ADC_INIT(void);
uint16_t ADC_GetValue(uint8_t ADC_Channel);

uint16_t Pot_GetValue(void);
uint16_t Temp_GetValue(void);
uint16_t Light_GetValue(void);
#endif /* BSP_ADC_H_ */
