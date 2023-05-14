/*******************************************
	*�ļ���  ��  bsp_dma.h
	*��   �� ��  WF
	*�޸�ʱ�䣺  2021.03.13
	*��   ����   v1.0
    *˵   ����   dmaͷ�ļ�
*******************************************/


#ifndef BSP_DMA_H
#define BSP_DMA_H

#include "stm32f10x.h"

void DMA_USART1_TX_Configuration(u32 MemAddr, u16 DataNum);
void DMA_USART1_RX_Configuration(u32 MemAddr, u16 DataNum);
void DMA_ADC1_Configuration(u32 MemAddr, u16 DataNum);
void DMA_USART1_TX_Transmission(u16 DataNum);
void DMA_USART1_RX_Transmission(u16 DataNum);


#endif /* !BSP_DMA_H */

