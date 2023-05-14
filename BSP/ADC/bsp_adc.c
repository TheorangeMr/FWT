/*******************************************
	*�ļ���  ��  bsp_adc.c
	*��   �� ��  WF
	*�޸�ʱ�䣺  2021.03.13
	*��   ����   v1.0
  *˵   ����   adcԴ�ļ�
*******************************************/


#include "bsp_adc.h"
#include "bsp_dma.h"
#include "FreeRTOS.h"
#include "task.h"


/* 
	���ʹ����ɨ��ģʽ��DMA������һ��ADC_Value[][]�������洢ADת�������
	��ADC1_SCAN_CHANNEL_NUM��ͨ����ÿ��ͨ������ADC_SMOOTHING�����ݡ�
*/
#if ADC1_SCAN_ENABLE && ADC1_DMA_ENABLE
vu16 ADC_Value[ADC_SMOOTHING][ADC1_SCAN_CHANNEL_NUM] = {0};
#endif


/* 
	*��������ADC_Configuration()
	*��  �ܣ�ADC��ʼ������
	*��  �ߣ�WF
	*��  ������
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/
void ADC_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef   ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1 , ENABLE);
	
	/* ����ADCͨ������Ϊģ������ */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN ;
	GPIO_InitStructure.GPIO_Pin = ADC_CH6_GPIO_PIN|ADC_CH2_GPIO_PIN;
	GPIO_Init(ADC_CH1_GPIO_PORT , &GPIO_InitStructure);
	
	/* ����ADC��Ƶ���� */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_DeInit(ADC1);
	
	/* ��ʼ��ADCת��ģʽ */
	#if ADC1_SCAN_ENABLE
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                      //�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                            //����ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                      //��������ת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;     //ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                  //�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = ADC1_SCAN_CHANNEL_NUM;             //ͨ������
	ADC_Init(ADC1, &ADC_InitStructure);
	#else
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                      //�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                           //������ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                     //����������ת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;     //ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                  //�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;                                 //ͨ������Ϊ1 
	ADC_Init(ADC1, &ADC_InitStructure);
	#endif
	
	
	/* ���ù������ͨ������ */
	#if ADC1_SCAN_ENABLE
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1 , ADC_SampleTime_239Cycles5);
	#endif
	
	#if ADC1_DMA_ENABLE
	DMA_ADC1_Configuration((u32)ADC_Value, ADC_SMOOTHING*ADC1_SCAN_CHANNEL_NUM);
	ADC_DMACmd(ADC1, ENABLE);
	#endif
	
	ADC_Cmd(ADC1 , ENABLE);                      //ʹ��ADC
	
	ADC_ResetCalibration(ADC1);	                 //ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	 //�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	                 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	     //�ȴ�У׼����
}


/* 
	*��������ADC_SingleMode_GetValue()
	*��  �ܣ���ȡ����ģʽADCֵ
	*��  �ߣ�WF
	*��  ����channel_x:ADCͨ����
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/
u16 ADC_SingleMode_GetValue(u8 channel_x)
{
	u32 Sum = 0;
	u16 Max = 0;
	u16 Min = 4095;
	u16 tempdata = 0;
	u16 Average = 0;
	
	/* ����ͨ������ */
	ADC_RegularChannelConfig(ADC1, channel_x, 1 , ADC_SampleTime_239Cycles5);
	
	/* ����˲����� */
	for(u8 i = 0; i < ADC_SMOOTHING; i++)
	{
		/* �������ת�� */
		ADC_SoftwareStartConvCmd(ADC1 , ENABLE);
		/* �ȴ�ת����� */
		while(ADC_GetFlagStatus(ADC1 , ADC_FLAG_EOC) == RESET);
		/* ��ȡ���� */
		tempdata = ADC_GetConversionValue(ADC1);
		
		Max = (Max>tempdata)?Max:tempdata;
		Min = (Min<tempdata)?Min:tempdata;
		Sum += tempdata;
	}
	
	/* ȥ�����ֵ����Сֵ����ƽ��ֵ */
	Average = (Sum-Max-Min)/(ADC_SMOOTHING-2);
	
	return Average;
}


/* 
	*��������ADC_ScanMode_GetVlaue()
	*��  �ܣ���ȡɨ��ģʽADCֵ
	*��  �ߣ�WF
	*��  ����*zdata������ָ��
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/

#if ADC1_SCAN_ENABLE

void ADC_ScanMode_GetVlaue(u16 *zdata)
{
	u32 Sum = 0;
	u16 Max = 0;
	u16 Min = 4095;
	

	/* �������ת�� */
	ADC_SoftwareStartConvCmd(ADC1 , ENABLE);
	
	/* �ȴ�ת����� */
	//while(ADC_GetFlagStatus(ADC1 , ADC_FLAG_EOC) == RESET);
	//���ڵȴ�ת����ɣ���ʹ��ʵʱ����ϵͳʱ������ʹ����ʱ��������CPUʹ��Ȩ��
	//�Ӷ������γ�����
	vTaskDelay(10);
	
	/* ����˲�����ֵ�˲��� */ 
	for(u8 i = 0; i < ADC1_SCAN_CHANNEL_NUM; i++)
	{
		Sum = 0;
		Max = 0;
		Min = 4095;
		for(u8 j = 0; j < ADC_SMOOTHING; j++)
		{
			Max = (Max>ADC_Value[j][i])?Max:ADC_Value[j][i];
		  Min = (Min<ADC_Value[j][i])?Min:ADC_Value[j][i];
			Sum += ADC_Value[j][i];
		}
		zdata[i] = (Sum-Min-Max)/(ADC_SMOOTHING-2);
	}
}

#endif


