#include "stm32f10x.h"
#include "ADC.h"
#include "NTC.h"

#define VDD_VOLTAGE   	3.3f			//单片机供电电压
#define R_DIVIDER     	10.0f			//热敏传感器上的分压电阻单位为KΩ
#define NTC_LEN			61				//温度表长

#define BRIGHT_VALUE	2800.0f			//光敏传感器最亮限制
#define DARK_VALUE		3800.0f			//光敏传感器最暗限制


/**
  * @brief  初始化ADC
  * @param  无
  * @retval 无
  * @note   使用引脚为：PA4、PA5、PA6
  */
void ADC_INIT(void)
{
	ADC_InitTypeDef ADC_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//ADC 时钟 = 72MHz / 6 = 12MHz（F103 上限 14MHz）
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;

	/*PA4接电位器、PA5接热敏传感器、PA6接光敏传感器*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_Cmd(ADC1, ENABLE);

	/*ADC校准过程*/
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1) == SET);	//等待复位完成
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) == SET);		//等待校准完成


	/* 滑动窗口长度 8，预填充避免开机前 7 次被 0 拉偏 */
	for(uint8_t i = 0; i < 8; i++)
	{
		Pot_GetValue();
		Temp_GetValue();
		Light_GetValue();
	}
}



/**
  * @brief  获取定义对应传感器的数值
  * @param  要读取的ADC通道：ADC_Channel_x(其中x改成4、5、6)
  * @retval 读出的AD转换数值范围：0 ~ 4095
  * @note   使用软件触发转换ADC_SoftwareStartConvCmd
  */
uint16_t ADC_GetValue(uint8_t ADC_Channel)
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);		//等待转换完成

	return ADC_GetConversionValue(ADC1);
}


/**
  * @brief  滑动平均滤波函数
  * @param  Filter_value：对应的采样数组指针，Value：更新的数据
  * @retval 输入值的 8 点平均值
  * @note   使每个传感器输出的值更稳定
  */
uint16_t Moving_Average(uint16_t *Filter_value, uint16_t Value)
{
	int8_t 		i;
	uint16_t	sum = 0;

	for(i = 0; i < 7; i++)
	{
		//正序搬：往低地址搬必须从前往后
		Filter_value[i] = Filter_value[i + 1];
	}

	Filter_value[7] = Value;

	for(i = 0; i < 8; i++)
	{
		sum += Filter_value[i];
	}

	return (sum / 8);
}


/**
  * @brief  温度检测（二分查表法 + 插值公式）
  * @param  计算得出的热敏电阻阻值 Ro
  * @retval 0.0 ~ 60.0
  * @note   将热敏电阻的AD转换值换算成实际温度(单位为：℃)
  */
float Scan_NTC(float Ro)
{
	uint8_t Left = 0, Right = NTC_LEN - 1, Middle;
	float Temp;

	/*钳位判断数值是否超出量程*/
	if(Ro <= ntc_R_kohm[Right]) return 60.0;	//低于表的阻值下限直接输出温度最大值 60℃
	if(Ro >= ntc_R_kohm[Left])	return 0.0;		//高于表的阻值上限直接输出温度最小值 0℃

	/*二分查表法，交叉退出循环*/
	while(Left <= Right)
	{
		Middle = (Left + Right) / 2;

		if(ntc_R_kohm[Middle] == Ro) return Middle;
		else if(Ro > ntc_R_kohm[Middle]) Right = Middle - 1;
		else if(Ro < ntc_R_kohm[Middle]) Left= Middle + 1;
	}

	//从 Right（低温侧）起插值
	Temp = Right + (Left - Right) * (Ro - ntc_R_kohm[Right]) / (ntc_R_kohm[Left] - ntc_R_kohm[Right]);

	return Temp;
}



/**
  * @brief  读取电位器的AD值转换成PWM输出
  * @param  无
  * @retval 0 ~ 999
  */
uint16_t Pot_GetValue(void)
{
	uint16_t 			AD_Value, led_duty;
	static uint16_t 	pot_buf[8];							//滑动采样数值

	AD_Value = ADC_GetValue(ADC_Channel_4);

	//转换成PWM：范围是0 ~ 999
	led_duty = (float)(4095 - AD_Value) / 4095.0 * 999.0;	//先转 float，否则整数除法只出 0 或 999

	return Moving_Average(pot_buf, led_duty);
}


/**
  * @brief  读取热敏传感器的AD值转换成温度输出
  * @param  无
  * @retval 0 ~ 60（单位：℃）
  */
uint16_t Temp_GetValue(void)
{
	uint16_t 			AD_Value, Temp;
	static uint16_t		temp_buf[8];				//滑动采样数值
	float 				Vo, Ro;

	AD_Value = ADC_GetValue(ADC_Channel_5);
	Vo = (float) AD_Value / 4095 * VDD_VOLTAGE;		//将AD值转换成输入电压
	Ro = (Vo * R_DIVIDER) / (VDD_VOLTAGE - Vo);		//计算出热敏电阻的阻值

	Temp = Scan_NTC(Ro);							//查表 + 插值计算出具体温度

	return Moving_Average(temp_buf, Temp);
}


/**
  * @brief  读取光敏传感器的AD值转换成亮度百分比
  * @param  无
  * @retval 0 ~ 100（单位：%）
  */
uint16_t Light_GetValue(void)
{
	uint16_t 			AD_Value, Lux;
	static uint16_t		light_buf[8];				//滑动采样数值

	AD_Value = ADC_GetValue(ADC_Channel_6);

	/*钳位判断数值是否超出量程*/
	if(AD_Value > DARK_VALUE) 	AD_Value = DARK_VALUE;		//过暗就按照最暗量程来计算
	if(AD_Value < BRIGHT_VALUE)	AD_Value = BRIGHT_VALUE;	//过亮就按照最亮量程来计算

	/*AD值转换亮度百分比*/
	Lux = 100 - ((float)AD_Value - BRIGHT_VALUE) / (DARK_VALUE - BRIGHT_VALUE) * 100;

	return Moving_Average(light_buf, Lux);
}



