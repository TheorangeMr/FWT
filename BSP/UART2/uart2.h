#ifndef __UART3_H
#define __UART3_H

//#include "sys.h"
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


void uart2_init(u32 bound);
void usart2_send_data(u8 *data, u32 size);


#endif




