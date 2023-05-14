/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_delay.h"
#include "bsp_timer.h"
#include "usart.h"
#include "event_groups.h"
#include "bsp_sdio_sdcard.h"
#include "bsp_pwr.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
	printf("eeror!!\r\n");
	__set_FAULTMASK(1);
	NVIC_SystemReset();
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

extern void xPortSysTickHandler(void);
//systick�жϷ�����
void SysTick_Handler(void)
{	
#if (INCLUDE_xTaskGetSchedulerState  == 1 )
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
#endif  /* INCLUDE_xTaskGetSchedulerState */  
    
    xPortSysTickHandler();
    
#if (INCLUDE_xTaskGetSchedulerState  == 1 )
  }
#endif  /* INCLUDE_xTaskGetSchedulerState */
}

extern vu8 RTC_Time_Flag;   
extern CanRxMsg RxMessage;

/* CAN�жϷ����� */
void CAN_RX_IRQHandler(void)
{
//	printf("Interrupt OK!\r\n");
	u8 i = 0;
	
	/* ��ʼ��CAN������ */
	RxMessage.StdId=0x00;
  RxMessage.ExtId=0x00;
  RxMessage.IDE=0;
  RxMessage.DLC=0;
  RxMessage.FMI=0;
	
	for(i = 0; i < 8; i++)
	{
		RxMessage.Data[i]=0x00;
	}

	/* �������� */
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	/* �ж��Ƿ�ΪĿ�귽����,�������ݽ���У�� */
  if((RxMessage.ExtId==0x2001) && (RxMessage.IDE==CAN_ID_EXT) && (RxMessage.DLC==3)
		 && (RxMessage.Data[0]==0x02) && (RxMessage.Data[2] == 0x18))
  {
		 RTC_Time_Flag = RxMessage.Data[1];			
  }
}



#define EVENTBIT_0	(1<<0)                          //���ٽ����¼�
#define EVENTBIT_1	(1<<1)                          //���ڴ�ӡ�¼�
#define EVENTBIT_2	(1<<2)                          //��̼����¼�

vu8 Actual_Endflag = 0;                              //ʵ�ʽ�����־
u8 Startup_Flag = 0;                                //��ʼ�ɼ���־
vu8 Endup_Flag = 0;                                  //������־
u32 Pulse_Count = 1;                                //�������������
vu32 Total_Time_M1 = 0;                               //M1  Ƶ��
vu32 Total_Time_M2 = 0;                               //M2
vu32 Overflow_Count = 0;                             
vu8  Sampling_Flag = 0;
extern EventGroupHandle_t EventGroupHandler;



/* ��ʱ��1����/�Ƚ��жϷ����� */
void ADVANCE_TIM_CCx_IRQHandler(void)
{
	BaseType_t pxHigherPriorityTaskWoken,xResult;
	if (TIM_GetITStatus(ADVANCE_TIMEX, ADVANCE_TIM_IT_CCx) != 0) //ͨ��1����
	{
		if(Startup_Flag == 1)
		{
			Pulse_Count++;                                                            //M2�������
		}
		if(((GPIOA->IDR & GPIO_Pin_8) >> 8) == 1&&Sampling_Flag == 0)               //ȷ������Ϊ�ߵ�ƽ
		{
			if(Endup_Flag != 0)                                                       //�������ڽ�������ڶ���������
			{
				TIM_Cmd(ADVANCE_TIMEX,DISABLE);                                         //�رն�ʱ��1����
				TIM_ITConfig(ADVANCE_TIMEX, TIM_IT_CC1, DISABLE);		                    //��ֹ����/�Ƚ�1�ж�
				Total_Time_M1 = Pulse_Count;                                            //��ȡ���������ڵ�M1
				Total_Time_M2 = TIM_GetCounter(ADVANCE_TIMEX);									        //��ȡ��Ƶʱ������M2
				Pulse_Count = 1;                                                        //������1
				Startup_Flag = 0;                                                       //�״α�־����
				Actual_Endflag = 1;                                                     //ʵ�ʲ���������־EVENTBIT_2
				Endup_Flag = 0;
				/*�¼���λ*/
				pxHigherPriorityTaskWoken = pdFALSE;
				xResult = xEventGroupSetBitsFromISR(EventGroupHandler,EVENTBIT_0|EVENTBIT_2,&pxHigherPriorityTaskWoken);
				if(xResult != pdFAIL)
				{
					portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
				}
//				printf("timer 2\r\n");
			}
			else                            
			{
//				printf("timer 1\r\n");
				TIM_SetCounter(ADVANCE_TIMEX, 0);											                  //M1����������
				Startup_Flag = 1;							                                          //����״β���������
				Sampling_Flag = 1;
			}
		}
	 }
	TIM_ClearITPendingBit(ADVANCE_TIMEX, ADVANCE_TIM_IT_CCx);
}

/* ��ʱ��1�����жϷ����� */
void ADVANCE_TIM_IRQHandler(void)
{
	//printf("ok\r\n");	
	if (TIM_GetITStatus(ADVANCE_TIMEX, TIM_IT_Update) != 0)                                //��ʱ��1���������ж�
	{
		if (Endup_Flag == 0)                                                        //���������ɱ�־û�б���λ
		{
			Overflow_Count++; //�������
		}		
	}
	TIM_ClearITPendingBit(ADVANCE_TIMEX, TIM_IT_Update);                                   //����жϱ�־;
}
/* ��ʱ��7�����жϷ����� */
void BASIC_TIM_IRQHandler(void)
{
	static u8 time = 0;
	if(TIM_GetITStatus(BASIC_TIMX, TIM_IT_Update) != 0)
	{
		time++;
		if(time >= 8)
		{
			Sampling_Flag = 0;
			Endup_Flag = 1;                                                           //�涨���ڲ���������־
			time = 0;
			TIM_Cmd(BASIC_TIMX,DISABLE);                                                  //�رն�ʱ��7����
		}
	}
	TIM_ClearITPendingBit(BASIC_TIMX, TIM_IT_Update);
}


extern uint8_t Oil_base_dat;
/* ��ʱ��4�����жϷ����� */
void OIL_NORMAL_TIM_IRQHandler(void)
{
	static uint8_t tim4_count = 0;
	if(TIM_GetITStatus(OIL_NORMAL_TIM, TIM_IT_Update) != 0)
	{
		tim4_count++;
		if(tim4_count >= 120)
		{
			if(Oil_base_dat > 0)
			{
				Oil_base_dat--;
			}
      tim4_count = 0;		
		}
	}
	TIM_ClearITPendingBit(OIL_NORMAL_TIM, TIM_IT_Update);
}


/*�����жϷ�����*/

void DEBUG_USART_IRQHandler(void)
{
	BaseType_t pxHigherPriorityTaskWoken,xResult;
	if(USART_GetITStatus(DEBUG_USARTx, USART_IT_RXNE) != RESET)
	{
		uint8_t rdat = USART_ReceiveData(DEBUG_USARTx);
		USART_ClearITPendingBit(DEBUG_USARTx, USART_IT_RXNE);
		if(rdat == USART_ON)
		{
			printf(" \nSTART \r\n");
			pxHigherPriorityTaskWoken = pdFALSE;
			xResult = xEventGroupSetBitsFromISR(EventGroupHandler,EVENTBIT_1,&pxHigherPriorityTaskWoken);
			if(xResult != pdFAIL)
			{
				portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
			}
		}
		else if(rdat == USART_OFF)
		{
			printf(" \nSTOP \r\n");
			pxHigherPriorityTaskWoken = pdFALSE;
			xResult = xEventGroupClearBitsFromISR( EventGroupHandler, EVENTBIT_1 );
			if(xResult != pdFAIL)
			{
				portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
			}
		}
	}
}


void CPU_NORMAL_TIM_IRQHandler(void)
{
	static u8 time = 0;
	if(TIM_GetITStatus(CPU_NORMAL_TIM, TIM_IT_Update) != 0)
	{
		time++;
		if(time >= 20)
		{
			BKP_WriteBackupRegister(BKP_DR40, 0x18);
			TIM_Cmd(CPU_NORMAL_TIM,DISABLE);                                                  //�رն�ʱ��5����	
			Standby_mode();
		}
	}
	TIM_ClearITPendingBit(CPU_NORMAL_TIM, TIM_IT_Update);
}

void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();  //SDIO�жϴ�����
}

/* ����ͳ������ʱ�� */
volatile uint32_t CPU_RunTime = 0UL;

void  CPU_BASIC_TIM_IRQHandler (void)
{
	if ( TIM_GetITStatus( CPU_BASIC_TIM, TIM_IT_Update) != RESET ) 
	{	
    CPU_RunTime++;
		TIM_ClearITPendingBit(CPU_BASIC_TIM , TIM_FLAG_Update);  		 
	}		 	
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
