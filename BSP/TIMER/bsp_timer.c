/*******************************************
	*�ļ��� ��  bsp_timer.c
	*��   �ߣ�  �޳�
	*�޸�ʱ�䣺 2022.05.2
	*��   ����  v1.0
  *˵   ����  BSP/TIMER�ļ�
*******************************************/

#include "bsp_timer.h"

#if TIM1_INPUT_CAPTURE_MODE

/* 
	*��������Timer1_Init()
	*��  �ܣ�TIMER1��ʼ������
	*��  �ߣ��޳�
	*��  ����u16 psc:Ԥ��Ƶϵ��, u16 arr���Զ���װ��ֵ
	*����ֵ����
	*ʱ  �䣺2022.05.2
*/

static void NVIC_Time1_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* Enable the TIM1 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_CCx_IRQ; 										//��ʱ��1ͨ��1����/�Ƚ��ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* ���ö�ʱ��1�����ж� */
	NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_IRQ; 												//��ʱ��1�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    									//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;           									//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void Timer1_Init(u16 psc, u16 arr)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;


	/* TIM1 clock enable */
	RCC_APB2PeriphClockCmd(ADVANCE_TIM_CLK, ENABLE);
	/* GPIOA clock enable */
	RCC_APB2PeriphClockCmd(ADVANCE_TIM_CH1_GPIO_CLK, ENABLE);

	/* TIM1 channel 1 pin (PA.08) configuration */
	GPIO_InitStructure.GPIO_Pin = ADVANCE_TIM_CH1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                         //�½��ش�������Ϊ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ADVANCE_TIM_CH1_GPIO_PORT, &GPIO_InitStructure);

	/* ���ö�ʱ��1 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;               //����Ϊ���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;			              //����Ƶ
	TIM_TimeBaseInitStructure.TIM_Period = arr;											              //�Զ�װ�س�ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;									              //Ԥ��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = ADVANCE_TIM_RepetitionCount;//�߼���ʱ����������
	TIM_TimeBaseInit(ADVANCE_TIMEX, &TIM_TimeBaseInitStructure);				          //��ʼ����ʱ��

	/* ���ö�ʱ��1���벶�� */
	TIM_ICInitStructure.TIM_Channel = ADVANCE_TIM_CHANNEL_x;								      //ͨ��1
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;		                //�����ش���
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;               //ֱ��ӳ��(TI1-->IC1)
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;						        			//����Ƶ
	TIM_ICInitStructure.TIM_ICFilter = 0x8;													       			  //�����˲�
	TIM_ICInit(ADVANCE_TIMEX, &TIM_ICInitStructure); 

  NVIC_Time1_Config();
	TIM_ClearITPendingBit(ADVANCE_TIMEX, TIM_FLAG_Update | ADVANCE_TIM_IT_CCx); 	/*ʹ�ܸ����ж�ǰ����жϱ�־��
																																								��ֹһ��ʼʹ�ܾͽ����ж�*/
//	/* TIM enable counter */
//	TIM_Cmd(TIM1, ENABLE);
	/* Enable the CC1 and Update Interrupt Request */
//	TIM_ITConfig(TIM1, TIM_IT_CC1 | TIM_IT_Update, ENABLE);
}



#endif


static void NVIC_Time7_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* ���ö�ʱ��7�����ж� */
	NVIC_InitStructure.NVIC_IRQChannel = BASIC_TIM_IRQ; 													//��ʱ��7�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void NVIC_Time6_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* ���ö�ʱ��6�����ж� */
	NVIC_InitStructure.NVIC_IRQChannel = CPU_BASIC_TIM_IRQ; 													//��ʱ��6�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void NVIC_Time5_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* ���ö�ʱ��5�����ж� */
	NVIC_InitStructure.NVIC_IRQChannel = CPU_NORMAL_TIM_IRQ; 													//��ʱ��5�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void NVIC_Time4_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* ���ö�ʱ��5�����ж� */
	NVIC_InitStructure.NVIC_IRQChannel = OIL_NORMAL_TIM_IRQ; 													//��ʱ��5�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/* 
	*��������Timer7_Init()
	*��  �ܣ�TIMER7��ʼ������
	*��  �ߣ��޳�
	*��  ����u16 psc:Ԥ��Ƶϵ��, u16 arr���Զ���װ��ֵ
	*����ֵ����
	*ʱ  �䣺2022.05.2
*/

void Timer7_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM7 clock enable */
	RCC_APB1PeriphClockCmd(BASIC_TIM_CLK, ENABLE);

	/* ���ö�ʱ��7 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//����Ϊ���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//����Ƶ
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//�Զ�װ�س�ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//Ԥ��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//�߼���ʱ����������
	TIM_TimeBaseInit(BASIC_TIMX, &TIM_TimeBaseInitStructure);											//��ʼ����ʱ��
	
	NVIC_Time7_Config();
	TIM_ITConfig(BASIC_TIMX, TIM_IT_Update, ENABLE);
	TIM_Cmd(BASIC_TIMX, DISABLE);                                       			    //�رն�ʱ��7
}




void Timer6_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM6 clock enable */
	RCC_APB1PeriphClockCmd(CPU_BASIC_TIM_CLK, ENABLE);

	/* ���ö�ʱ��6 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//����Ϊ���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//����Ƶ
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//�Զ�װ�س�ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//Ԥ��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//�߼���ʱ����������
	TIM_TimeBaseInit(CPU_BASIC_TIM, &TIM_TimeBaseInitStructure);											//��ʼ����ʱ��
	
	NVIC_Time6_Config();
	TIM_TimeBaseInit(CPU_BASIC_TIM, &TIM_TimeBaseInitStructure);											// ��ʼ����ʱ��
	TIM_ClearFlag(CPU_BASIC_TIM, TIM_FLAG_Update);																		// ����������жϱ�־λ
	TIM_ITConfig(CPU_BASIC_TIM,TIM_IT_Update,ENABLE);																	// �����������ж�
   TIM_Cmd(CPU_BASIC_TIM, ENABLE);																									// ʹ�ܼ�����
}

void Timer5_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM5 clock enable */
	RCC_APB1PeriphClockCmd(NORMAL_TIM_CLK5, ENABLE);

	/* ���ö�ʱ��5 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//����Ϊ���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//����Ƶ
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//�Զ�װ�س�ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//Ԥ��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//�߼���ʱ����������
	TIM_TimeBaseInit(CPU_NORMAL_TIM, &TIM_TimeBaseInitStructure);											//��ʼ����ʱ��
	
	NVIC_Time5_Config();
	TIM_TimeBaseInit(CPU_NORMAL_TIM, &TIM_TimeBaseInitStructure);											// ��ʼ����ʱ��
	TIM_ClearFlag(CPU_NORMAL_TIM, TIM_FLAG_Update);																		// ����������жϱ�־λ
	TIM_ITConfig(CPU_NORMAL_TIM,TIM_IT_Update,ENABLE);																// �����������ж�
  TIM_Cmd(CPU_NORMAL_TIM, DISABLE);																									// ʹ�ܼ�����
}

void Timer4_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM1 clock enable */
	RCC_APB1PeriphClockCmd(NORMAL_TIM_CLK4, ENABLE);

	/* ���ö�ʱ��5 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//����Ϊ���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//����Ƶ
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//�Զ�װ�س�ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//Ԥ��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//�߼���ʱ����������
	TIM_TimeBaseInit(OIL_NORMAL_TIM, &TIM_TimeBaseInitStructure);											//��ʼ����ʱ��
	
	NVIC_Time4_Config();
	TIM_TimeBaseInit(OIL_NORMAL_TIM, &TIM_TimeBaseInitStructure);											// ��ʼ����ʱ��
	TIM_ClearFlag(OIL_NORMAL_TIM, TIM_FLAG_Update);																		// ����������жϱ�־λ
	TIM_ITConfig(OIL_NORMAL_TIM,TIM_IT_Update,ENABLE);																// �����������ж�
  TIM_Cmd(OIL_NORMAL_TIM, DISABLE);																									// ʹ�ܼ�����
}



