/*******************************************
	*�ļ���  ��  bsp_dma.c
	*��   �� ��  WF
	*�޸�ʱ�䣺  2021.03.13
	*��   ����   v1.0
    *˵   ����   dmaԴ�ļ�
*******************************************/



#include "bsp_dma.h"



/* 
	*��������DMA_USART1_TX_Configuration()
	*��  �ܣ�USART1_TX DMA���ú���
	*��  �ߣ�WF
	*��  ����MemAddr���洢����ַ��DataNum�����ݸ���
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/
void DMA_USART1_TX_Configuration(u32 MemAddr, u16 DataNum)
{
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);                     //ʹ��DMAʱ��
	
	DMA_DeInit(DMA1_Channel4);                                              //��λ
	
	/* ��ʼ��USART1_TX DMAͨ������ */
	DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)&USART1->DR;           //�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = MemAddr;                         //�洢������ַ           
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      //���䷽��������ΪĿ��
	DMA_InitStructure.DMA_BufferSize = DataNum;                             //���ô�����������
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //����ָ�벻����ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //�洢��ָ������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //�������ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         //�洢�����ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           //����ģʽ��ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   //�е����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            //��ʹ�ô洢�����洢��ģʽ
	DMA_Init(DMA1_Channel4 , &DMA_InitStructure );
}



/* 
	*��������DMA_USART1_RX_Configuration()
	*��  �ܣ�USART1_RX DMA���ú���
	*��  �ߣ�WF
	*��  ����MemAddr���洢����ַ��DataNum�����ݸ���
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/

void DMA_USART1_RX_Configuration(u32 MemAddr, u16 DataNum)
{
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);                     //ʹ��DMAʱ��
	
	DMA_DeInit(DMA1_Channel5);                                              //��λ
	
	/* ��ʼ��USART1_RX DMAͨ������ */
	DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)&USART1->DR;           //�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = MemAddr;                         //�洢������ַ           
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      //���䷽������-->�洢��
	DMA_InitStructure.DMA_BufferSize = DataNum;                             //���ô�����������
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //����ָ�벻����ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //�洢��ָ������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //�������ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         //�洢�����ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           //����ģʽ��ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   //�е����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            //��ʹ�ô洢�����洢��ģʽ
	DMA_Init(DMA1_Channel5 , &DMA_InitStructure );
}




/* 
	*��������DMA_ADC1_Configuration()
	*��  �ܣ�ADC1 DMA���ú���
	*��  �ߣ�WF
	*��  ����MemAddr���洢����ַ��DataNum�����ݸ���
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/
void DMA_ADC1_Configuration(u32 MemAddr, u16 DataNum)
{
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);            //ʹ��DMAʱ��
	
	DMA_DeInit(DMA1_Channel1);                                     //��λ
	
	/* ��ʼ��ADC1 DMAͨ������ */
	DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)&ADC1->DR;                 //�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = MemAddr;                             //�洢������ַ           
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                          //���䷽������-->�ڴ�
	DMA_InitStructure.DMA_BufferSize = DataNum;                                 //���ô�����������
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //����ָ�벻����ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //�洢��ָ������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //�������ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         //�洢�����ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             //����ģʽ��ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       //�е����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                //��ʹ�ô洢�����洢��ģʽ
	DMA_Init(DMA1_Channel1 , &DMA_InitStructure );
	
	DMA_Cmd(DMA1_Channel1 , ENABLE);                                            //ʹ��DMA1ͨ��1
}


/* 
	*��������DMA_USART1_TX_Transmission()
	*��  �ܣ�����USART1_TX��DMA����
	*��  �ߣ�WF
	*��  ����DataNum�����ݸ���
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/
void DMA_USART1_TX_Transmission(u16 DataNum)
{
	DMA_Cmd(DMA1_Channel4 , DISABLE);                         //�ر�DMAͨ��
	DMA_SetCurrDataCounter(DMA1_Channel4 ,DataNum);           //���ô�������
	DMA_Cmd(DMA1_Channel4 , ENABLE);                          //ʹ��DMA1ͨ��4,����һ�δ���
}


/* 
	*��������DMA_USART1_RX_Transmission()
	*��  �ܣ�����USART1_RX��DMA����
	*��  �ߣ�WF
	*��  ����DataNum�����ݸ���
	*����ֵ����
	*ʱ  �䣺2021.03.13
*/
void DMA_USART1_RX_Transmission(u16 DataNum)
{
	DMA_Cmd(DMA1_Channel5 , DISABLE);                         //�ر�DMAͨ��
	DMA_SetCurrDataCounter(DMA1_Channel5 ,DataNum);           //���ô�������
	DMA_Cmd(DMA1_Channel5 , ENABLE);                          //ʹ��DMA1ͨ��5,����һ�δ���
}

