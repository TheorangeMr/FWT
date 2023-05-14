//#include "sys.h"
#include "uart2.h"
#include "ringbuffer.h"

/************************************************
 ALIENTEK ս��STM32F103��������չʵ��
 ATK-GPRS-M26(GPRS DTU)Ӧ��ʵ��ʵ��        �豸�˳���Slave��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

extern RingBuffer *p_uart2_rxbuf;


void uart2_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //ʹ�ܴ���2�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //�����ȼ�1��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure); //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

    USART_InitStructure.USART_BaudRate = bound;//����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ���ݳ���
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;///��żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
    USART_Init(USART2, &USART_InitStructure); //��ʼ������


    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�

    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 

}


void USART2_IRQHandler(void)                        //����3�жϷ������
{
    u8 Res;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
    {
        Res = USART_ReceiveData(USART2);            //��ȡ���յ�������

        RingBuffer_In(p_uart2_rxbuf, &Res, 1);            //���뻺��
    }
}

void usart2_send_data(u8 *data, u32 size)
{
    for(u32 i = 0; i < size; i++)
    {
        while((USART2->SR & 0X40) == 0);

        USART2->DR = data[i];
    }
}




