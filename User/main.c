/*******************************************
	*�ļ��� ��  VCU4.4
	*��   �ߣ�  �޳�
	*�޸�ʱ�䣺 2022.10.12
	*��   ����  v1.4
*******************************************/

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "portmacro.h"

#include "ff.h"
#include "diskio.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "usart.h"
#include "bsp_can.h"
#include "bsp_dma.h"
#include "bsp_adc.h"
#include "bsp_timer.h"
#include "bsp_sdio_sdcard.h"
#include "uart2.h"
#include "RingBuffer.h"
#include "atk_m750.h"
#include "string.h"
#include "bsp_pwr.h"
#include "bsp_delay.h"
#include "bsp_iwdg.h"


#define TOOTH_NUM 55			 //����
#define WHEEL_RADIUS 0.257 //�־�(��λ:m)
#define PI 3.14						 //Բ����
#define TANK_HEIGHT 210		 //����߶�
#define GEAR_RATIO  4.05    //������

/********************************* ���Դ�ӡ ****************************************/

#define TEST_vApp_Hook 1
#define TEST_PRINTF_SIMULATOR 0
#define TEST_PRINTF_CAN   0
#define TEST_PRINTF_SPEED 0
#define TEST_PRINTF_OIL 0
#define TEST_PRINTF_CPU 0
#define TEST_PRINTF_BATTERY 0
#define TEST_PRINTF_SD 0
#define TEST_PRINTF_4GDTU 0
#define TEST_PRINTF_ONENET 0
#define TEST_PRINTF_RINGBUFFER 0
#define TEST_IWDG 0



#if TEST_PRINTF_SIMULATOR
portCHAR flag1;
portCHAR flag2;
portCHAR flag3;
portCHAR flag4;
portCHAR flag5;
portCHAR flag6;
portCHAR flag7;
portCHAR flag8;
portCHAR flag9;
portCHAR flag10;
portCHAR flag11;
#endif

/********************************* �����ź��� ****************************************/
SemaphoreHandle_t MuxSem_Handle =NULL;    //�����ź������


/********************************* �¼� ****************************************/

EventGroupHandle_t EventGroupHandler = NULL;	//�¼���־����


#define EVENTBIT_0	(1<<0)                          //���ٽ����¼�
#define EVENTBIT_1	(1<<1)                          //���ڴ�ӡ�¼�
#define EVENTBIT_2	(1<<2)                          //��̼����¼�
#define EVENTBIT_3	(1<<3)                          //ι���Ź��¼�
#define EVENTBIT_4	(1<<4)                          //ONENET���ݷ����¼�



/********************** ��������ʼ�� ****************************************/

static TaskHandle_t AppTaskCreate_Handle = NULL; 
static TaskHandle_t SPEED_Task_Handle = NULL;
static TaskHandle_t OIL_Task_Handle = NULL;
static TaskHandle_t CAN_Task_Handle = NULL;
static TaskHandle_t USART_Task_Handle = NULL;
static TaskHandle_t Mileage_count_Task_Handle = NULL;
static TaskHandle_t SD_RWTask_Handle = NULL;
static TaskHandle_t Battery_capacity_Task_Handle = NULL;
static TaskHandle_t CPU_Task_Handle = NULL;
static TaskHandle_t OneNET_4G_DTU_Handle = NULL;
static TaskHandle_t DTU_4G_Task_Handle = NULL;
static TaskHandle_t RingBuffer_Read_Task_Handle = NULL;
static TaskHandle_t IWDG_Task_Handle = NULL;

/**************************���徲̬�����ջ�����ƿ�****************************************/


/* �������������ջ */
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
/* ��ʱ�����������ջ */
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];

/* ��������ջ */
static StackType_t AppTaskCreate_Stack[256];
/* SPEED�����ջ */
static StackType_t SPEED_Task_Stack[128];
/* OIL�����ջ */
static StackType_t OIL_Task_Stack[64];
/* CAN�����ջ */
static StackType_t CAN_Task_Stack[128];
/* OneNET_4G_DTU�����ջ */
static StackType_t OneNET_4G_DTU_Task_Stack[256];
/* 4G_DTU�����ջ */
static StackType_t DTU_4G_Task_Stack[128];


/* ����������ƿ� */
static StaticTask_t Idle_Task_TCB;
/* ��ʱ��������ƿ� */
static StaticTask_t Timer_Task_TCB;
/* ����������ƿ� */
static StaticTask_t AppTaskCreate_TCB;

static StaticTask_t SPEED_Task_TCB;
static StaticTask_t OIL_Task_TCB;
static StaticTask_t CAN_Task_TCB;
static StaticTask_t OneNET_4G_DTU_TCB;
static StaticTask_t DTU_4G_TCB;



/* ��ȡ����������ڴ� */

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
	                                 StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &Idle_Task_TCB;
	*ppxIdleTaskStackBuffer = Idle_Task_Stack;
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* ��ȡ��ʱ��������ڴ� */

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
	                                 StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &Timer_Task_TCB;
	*ppxTimerTaskStackBuffer = Timer_Task_Stack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}


#if TEST_vApp_Hook
/*ջ������Ӻ���*/
void vApplicationStackOverflowHook(TaskHandle_t xTask,signed char *pcTaskName)
{
	printf("ջ�������������\r\n%s\r\n",pcTaskName);
}

#endif
/*
*************************************************************************
*                             ��������
*************************************************************************
*/


static void AppTaskCreate(void);                                                /* ���ڴ������� */
static void BSP_Init(void);
static void SPEED_Task(void* parameter);                                        /*�ٶȲ�������*/
static void OIL_Task(void* parameter);                                          /*�������Ժ���*/
static void CAN_Task(void* parameter);                                          /*CANͨѶ����*/
static void USART_Task(void* parameter);                                        /*USARTͨѶ����*/
static void Mileage_count_Task(void* parameter);                                /*��¼���ֵ*/
static void SD_RWTask(void* parameter);                                          /*sd���ݶ�д*/
static float SD_ReadtoMileage_count(void);                                       /*��̼���*/
static void Battery_capacity_Task(void* parameter);                              /*����ص���*/
static void CPU_Task(void* pvParameters);                                        /* CPU_Task����ʵ�� */
static void OneNET_4G_DTU_Task(void* pvParameters);                              /* 4G-OneNET */
static void DTU_4G_Task(void* pvParameters);                                		 /*DTUʵʱʱ�䣬4G�ź�*/
static void RingBuffer_Read_Task(void* pvParameters);                            /*����4G DTU����*/
static void OneNet_FillBuf(uint8_t buff[][64]);
static void IWDG_Task(void* pvParameters);
void PreSleepProcessing(uint32_t ulExpectedIdleTime);
void PostSleepProcessing(uint32_t ulExpectedIdeTime);



/*
*************************************************************************
*                             ȫ�ֱ���
*************************************************************************
*/

extern vu8 Endup_Flag;                      //�涨T������־
extern vu32 Total_Time_M1;                  //����������
extern vu32 Total_Time_M2;                  //��Ƶ��ʱ��
extern vu32 Overflow_Count;                 //�������           
extern vu8 Actual_Endflag;                  //ʵ�ʲ���������־
extern const char sqpa[4][4];
extern ST_Time Timedat;

//vu8 Detection_update_flag = 0;              //���ٺ������±�־
u32 Car_SpeedData = 0;			             			//����
float Rotate_Speed = 0;                       //ת��
vu8 RTC_Time_Flag = 0;				     					//CAN��������
u8 SD_Mileage_sum_flag = 0;                   //SD-������ݶ�ȡ��־
CanRxMsg RxMessage;							 							//CAN������Ϣ�ṹ��
CanTxMsg TxMessage;							 							//CAN������Ϣ�ṹ��
u8 VCU_Data_Send[8];           						    //VCU_Data���ͱ�������
u8 VCU_Data2_Send[8];          						    //VCU_Data2���ͱ�������
uint8_t CanTx_Buf1[8] = {0};									//CAN���ͻ�������1
uint8_t CanTx_Buf2[8] = {0};				 					//CAN���ͻ�������2
uint8_t Oil = 0;							 							  //����
uint8_t Oil_count[110]= {0};
uint16_t Oil_dat = 0;                         //ADC�����ɼ�ԭʼ����
uint8_t Oil_base_dat = 97;                     //�������ջ�׼ֵ
uint16_t High[8] = {0};                       //�����߶�����
float Mileage_count = 0;                      //��̼���
float Mileage_sum = 0;
uint16_t Battery = 0;                         //����
uint8_t Battery_flag = 0;                     //��ʾ��������
uint8_t Network_size = 0;                     //4G�ź�
static uint8_t Mileagecount_Task = 0,SD_RW_Task = 0;


//4G DTU
#define DTU_ONENETDATE_RX_BUF (512)
#define DTU_ONNETDATE_TX_BUF (64)
static uint8_t DTU_Time_flag = 0;
static uint32_t dtu_rxlen = 0;
static uint8_t dtu_rxbuf[DTU_ONENETDATE_RX_BUF] = {0};
RingBuffer *p_uart2_rxbuf;
uint8_t Sdat[3] = {3,0,0x46};
uint8_t Send_date[10][DTU_ONNETDATE_TX_BUF] = {0};
uint8_t DTUTask1 = 0,DTUTask2 = 0,DTUTask3 = 0;
static uint8_t DTUMode_Switch_flag = 0;
uint8_t DTU_AT_CLKFLAG = 0;
uint8_t ONENET_OFF_FLAG = 0;
const char ONENET_COM_OFF[]="lc0218";
const char ONENET_COM_ON[] = "lc2001";

//�ļ�ϵͳ
FATFS fs;
FIL fnew;
FRESULT res_flash;
UINT fnum;
BYTE work[FF_MAX_SS];

char ReadBuffer[512] = {0};
char Fdat[50];

/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{
	BSP_Init();
	
//    printf("�����ӵ���������������VCU��������!\r\n");
	AppTaskCreate_Handle = xTaskCreateStatic((TaskFunction_t	)AppTaskCreate,		  //������
															(const char* 	)"AppTaskCreate",		                //��������
															(uint32_t 		)256,	                              //�����ջ��С
															(void* 		  	)NULL,				                      //���ݸ��������Ĳ���
															(UBaseType_t 	)1, 	                              //�������ȼ�
															(StackType_t*   )AppTaskCreate_Stack,	            //�����ջ
															(StaticTask_t*  )&AppTaskCreate_TCB);	            //������ƿ� 
  /* ����������� */          
	if(NULL != AppTaskCreate_Handle)                                              /* �����ɹ� */
    vTaskStartScheduler();    
	else
		printf("����ʧ��! \r\n");
    while(1);   /* ��������ִ�е����� */
}

/**********************************************************************
  * @ ������  �� BSP_Task
  * @ ����˵���� �����ʼ��
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

void BSP_Init(void)
{
	uint16_t timeout = 0;
	int ret;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  ADC_Configuration();
	uart_init(115200);
	uart2_init(115200);
	Delay_Init(72);
	my_mem_init(SRAMIN);
	p_uart2_rxbuf = RingBuffer_Malloc(1024);        /*���ڴ���з���1K���ڴ������3����DTU����*/
	CAN_Config();
	Timer6_Init(72-1, 50-1);              /*50us*/
	Timer1_Init(72 - 1, 50000 - 1);      /*50ms*/
	Timer7_Init(7200-1, 500-1);            /*50ms*/
	Timer5_Init(7200-1, 5000-1);          /*500ms*/
	Timer4_Init(7200-1, 5000-1);          /*500ms*/	
	TIM_ITConfig(ADVANCE_TIMEX, TIM_IT_CC1| TIM_IT_Update, ENABLE);           //������/�Ƚ�1�жϺ͸����ж�
	TIM_Cmd(ADVANCE_TIMEX,ENABLE);                                            //�򿪶�ʱ��1��ʱ
	TIM_Cmd(BASIC_TIMX,ENABLE);                                                  
	
	//�����ļ�ϵͳ
	res_flash = f_mount(&fs,"1:",1);
  if(res_flash!=FR_OK)
  {
    printf("�����ⲿSD�����ļ�ϵͳʧ�ܡ�(%d)\r\n",res_flash);
    printf("��������ԭ��SDIO_SD��ʼ�����ɹ���\r\n");
		if(res_flash == FR_NO_FILESYSTEM)
		{
			res_flash = f_mkfs("1:",0,work,sizeof work);
			if(res_flash == FR_OK)
			{
				printf("��SD�ѳɹ���ʽ���ļ�ϵͳ��\r\n");
				/* ��ʽ������ȡ������ */
				res_flash = f_mount(NULL,"1:",1);	
				/* ���¹���	*/			
				res_flash = f_mount(&fs,"1:",1);
				goto	repeat;
			}
			else
			{
				printf("������ʽ��ʧ�ܡ�����\r\n");
			}
		}
		SD_RW_Task = 1;Mileagecount_Task = 1;
		goto	repeat;		
    repeat:
		;
  }
  else
  {
    printf("���ļ�ϵͳ���سɹ�\r\n");
  }
	//����4G DTU

	printf("Wait for Cat1 DTU to start, wait 10s.... \r\n");
	while( timeout <= 10 )   /* �ȴ�Cat1 DTU��������Ҫ�ȴ�5-6s�������� */
	{
			ret = dtu_config_init(DTU_WORKMODE_ONENET);    /*��ʼ��DTU��������*/
			if( ret == 0 )
			{
				printf("Cat1 DTU Init Success \r\n");
				DTUMode_Switch_flag = 1;
				if(BKP_ReadBackupRegister(BKP_DR40) != 0x18)
				{
					WWDG_Config(6,1000);                          //6.4s
				}
				else
				{
					BKP_DeInit();
					BKP_ClearFlag();
					Standby_mode();
				}
				break;				
			}
			timeout++;
			delay_xms(1000);
	}
	while( timeout > 10 )   /* ��ʱ */
	{
		printf("**************************************************************************\r\n");
		printf("ATK-DTU Init Fail ...\r\n");
		printf("�밴�����²�����м��:\r\n");
		printf("1.ʹ�õ�����λ������������DTU�ܷ񵥶���������\r\n");
		printf("2.���DTU���ڲ�����STM32ͨѶ�Ĵ��ڲ����Ƿ�һ��\r\n");
		printf("3.���DTU��STM32���ڵĽ����Ƿ���ȷ\r\n");
		printf("4.���DTU�����Ƿ�������DTU�Ƽ�ʹ��12V/1A��Դ���磬��Ҫʹ��USB��5V��ģ�鹩�磡��\r\n");
		printf("**************************************************************************\r\n\r\n");
		DTUTask1 = 1;DTUTask2 = 1;DTUTask3 = 1;
		delay_xms(1000);
		printf("Cat1 DTU Init Fail\r\n");
		break;
	}
}



/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
	BaseType_t xReturn = pdPASS;     /* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */	
  taskENTER_CRITICAL();           //�����ٽ���

	  /* ����EventGroup */  
	EventGroupHandler = xEventGroupCreate();
	if(NULL != EventGroupHandler)
	printf("EventGroupHandler �¼������ɹ���\r\n");

	  /* ����MuxSem */
  MuxSem_Handle = xSemaphoreCreateMutex();	 
  if(NULL != MuxSem_Handle)
    printf("MuxSem_Handle�����������ɹ�!\r\n");

  xReturn = xSemaphoreGive( MuxSem_Handle );//����������
  if( xReturn == pdTRUE )
  printf("�ͷ��ź���!\r\n");	

#if TEST_PRINTF_CPU
  /* ����CPU_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )CPU_Task, /* ������ں��� */
                        (const char*    )"CPU_Task",/* �������� */
                        (uint16_t       )256,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )15,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&CPU_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    printf("����CPU_Task����ɹ�!\r\n");	
	else
		printf("CPU_Task���񴴽�ʧ��!\r\n");	
#endif
	
  /* ����SPEED_Task���� */
	SPEED_Task_Handle = xTaskCreateStatic((TaskFunction_t	)SPEED_Task,		        //������
															(const char* 	)"SPEED_Task",		                  //��������
															(uint32_t 		)128,					                      //�����ջ��С
															(void* 		  	)NULL,				                      //���ݸ��������Ĳ���
															(UBaseType_t 	)14, 				                        //�������ȼ�
															(StackType_t*   )SPEED_Task_Stack,	              //�����ջ
															(StaticTask_t*  )&SPEED_Task_TCB);	              //������ƿ�													
																														
	if(NULL != SPEED_Task_Handle)                                                 /* �����ɹ� */
		printf("SPEED_Task���񴴽��ɹ�!\r\n");
	else
		printf("SPEED_Task���񴴽�ʧ��!\r\n");
	
	  /* ����OIL_Task���� */
	OIL_Task_Handle = xTaskCreateStatic((TaskFunction_t	)OIL_Task,		            //������
															(const char* 	)"OIL_Task",		                    //��������
															(uint32_t 		)64,					                      //�����ջ��С
															(void* 		  	)NULL,				                      //���ݸ��������Ĳ���
															(UBaseType_t 	)13, 				                        //�������ȼ�
															(StackType_t*   )OIL_Task_Stack,	                //�����ջ
															(StaticTask_t*  )&OIL_Task_TCB);	                //������ƿ�  
	
	if(NULL != OIL_Task_Handle)                                                   /* �����ɹ� */
		printf("OIL_Task���񴴽��ɹ�!\r\n");
	else
		printf("OIL_Task���񴴽�ʧ��!\r\n");
	
	  /* ����CAN_Task���� */
	CAN_Task_Handle = xTaskCreateStatic((TaskFunction_t	)CAN_Task,		            //������
															(const char* 	)"CAN_Task",		                    //��������
															(uint32_t 		)128,					                      //�����ջ��С
															(void* 		  	)NULL,				                      //���ݸ��������Ĳ���
															(UBaseType_t 	)12, 				                        //�������ȼ�
															(StackType_t*   )CAN_Task_Stack,	                //�����ջ
															(StaticTask_t*  )&CAN_Task_TCB);	                //������ƿ�  
	
	if(NULL != CAN_Task_Handle)                                                   /* �����ɹ� */
		printf("CAN_Task���񴴽��ɹ�!\r\n");
	else
		printf("CAN_Task���񴴽�ʧ��!\r\n");
	
	  /* ����USART_Task���� */
	xReturn = xTaskCreate((TaskFunction_t	)USART_Task,		                        //������
															(const char* 	)"USART_Task",		                  //��������
															(uint16_t 		)128,					                      //�����ջ��С
															(void* 		  	)NULL,				                      //���ݸ��������Ĳ���
															(UBaseType_t 	)5, 				                        //�������ȼ�
															(TaskHandle_t*  )&USART_Task_Handle);	            //������ƿ�ָ��   
	
	if(pdPASS == xReturn)/* �����ɹ� */
		printf("USART_Task���񴴽��ɹ�!\r\n");
	else
		printf("USART_Task���񴴽�ʧ��!\r\n");
	if(Mileagecount_Task == 0)
	{  	
			/* ����Mileage_count_Task���� */
		xReturn = xTaskCreate((TaskFunction_t	)Mileage_count_Task,		                 //������
																(const char* 	)"Mileage_count_Task",		           //��������
																(uint16_t 		)128,					                       //�����ջ��С
																(void* 		  	)NULL,				                       //���ݸ��������Ĳ���
																(UBaseType_t 	)4, 				                         //�������ȼ�
																(TaskHandle_t*  )&Mileage_count_Task_Handle);	             //������ƿ�ָ��   
		
		if(pdPASS == xReturn)/* �����ɹ� */
			printf("Mileage_count_Task���񴴽��ɹ�!\r\n");
		else
			printf("Mileage_count_Task���񴴽�ʧ��!\r\n");	
	}
	if(SD_RW_Task == 0)
	{  
			/* ����SD_ReadTask���� */
		xReturn = xTaskCreate((TaskFunction_t	)SD_RWTask,		                     		 //������
																(const char* 	)"SD_RWTask",		                   //��������
																(uint16_t 		)512,					                       //�����ջ��С
																(void* 		  	)NULL,				                       //���ݸ��������Ĳ���
																(UBaseType_t 	)11, 				                         //�������ȼ�
																(TaskHandle_t*  )&SD_RWTask_Handle);	           //������ƿ�ָ��   
		
		if(pdPASS == xReturn)/* �����ɹ� */
			printf("SD_RWTask���񴴽��ɹ�!\r\n");
		else
			printf("SD_RWTask���񴴽�ʧ��!\r\n");		
  }
	  /* ����Battery_capacity_Task���� */
	xReturn = xTaskCreate((TaskFunction_t	)Battery_capacity_Task,		               //������
															(const char* 	)"Battery_capacity_Task",		         //��������
															(uint16_t 		)128,					                       //�����ջ��С
															(void* 		  	)NULL,				                       //���ݸ��������Ĳ���
															(UBaseType_t 	)3, 				                         //�������ȼ�
															(TaskHandle_t*  )&Battery_capacity_Task_Handle);	 //������ƿ�ָ��   
	
	if(pdPASS == xReturn)/* �����ɹ� */
		printf("Battery_capacity_Task���񴴽��ɹ�!\r\n");
	else
		printf("Battery_capacity_Task���񴴽�ʧ��!\r\n");

	if(DTUTask3 == 0)
	{
		/* ����RingBuffer_Read_Task���� */
		xReturn = xTaskCreate((TaskFunction_t	)RingBuffer_Read_Task,		         //������
													(const char* 	)"RingBuffer_Read_Task",		         //��������
													(uint16_t 		)1024,					                     //�����ջ��С
													(void* 		  	)NULL,				                       //���ݸ��������Ĳ���
													(UBaseType_t 	)7, 				                         //�������ȼ�
													(TaskHandle_t*  )&RingBuffer_Read_Task_Handle);	   //������ƿ�ָ��   
	}
	if(pdPASS == xReturn)   /* �����ɹ� */
		printf("RingBuffer_Read_Task���񴴽��ɹ�!\r\n");
	else
		printf("RingBuffer_Read_Task���񴴽�ʧ��!\r\n");

	if(DTUTask1 == 0)
	{	
			/* ����OneNET_4G_DTU_Task���� */
		OneNET_4G_DTU_Handle = xTaskCreateStatic((TaskFunction_t	)OneNET_4G_DTU_Task,  //������
																(const char* 	)"OneNET_4G_DTU_Task",		            //��������
																(uint32_t 		)256,					                        //�����ջ��С
																(void* 		  	)NULL,				                        //���ݸ��������Ĳ���
																(UBaseType_t 	)10, 				                          //�������ȼ�
																(StackType_t*   )OneNET_4G_DTU_Task_Stack,	        //�����ջ
																(StaticTask_t*  )&OneNET_4G_DTU_TCB);	              //������ƿ�
		if(NULL != OneNET_4G_DTU_Handle)                                                /* �����ɹ� */
			printf("OneNET_4G_DTU���񴴽��ɹ�!\r\n");
		else
			printf("OneNET_4G_DTU���񴴽�ʧ��!\r\n");
	}
	if(DTUTask2 == 0)
	{		
		/* ����4G_DTU_Task���� */
		DTU_4G_Task_Handle = xTaskCreateStatic((TaskFunction_t	)DTU_4G_Task,  					//������
																(const char* 	)"DTU_4G_Task",		                    //��������
																(uint32_t 		)128,					                        //�����ջ��С
																(void* 		  	)NULL,				                        //���ݸ��������Ĳ���
																(UBaseType_t 	)8, 				                          //�������ȼ�
																(StackType_t*   )DTU_4G_Task_Stack,	                //�����ջ
																(StaticTask_t*  )&DTU_4G_TCB);	                    //������ƿ�  
		if(NULL != DTU_4G_Task_Handle)                                             			/* �����ɹ� */
			printf("DTU_4G_Task���񴴽��ɹ�!\r\n");
		else
			printf("DTU_4G_Task���񴴽�ʧ��!\r\n");	
	}
	/* ����IWDG_Task���� */
	xReturn = xTaskCreate((TaskFunction_t	)IWDG_Task,		         							//������
												(const char* 	)"IWDG_Task",		         								//��������
												(uint16_t 		)128,					                     		//�����ջ��С
												(void* 		  	)NULL,				                       //���ݸ��������Ĳ���
												(UBaseType_t 	)16, 				                         //�������ȼ�
												(TaskHandle_t*  )&IWDG_Task_Handle);	   						//������ƿ�ָ��   
	if(pdPASS == xReturn)   /* �����ɹ� */
		printf("IWDG_Task���񴴽��ɹ�!\r\n");
	else
		printf("IWDG_Task���񴴽�ʧ��!\r\n");	
	
  vTaskDelete(AppTaskCreate_Handle);                                            //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();                                                          //�˳��ٽ���
}




/**********************************************************************
  * @ ������  �� CAN_Task
  * @ ����˵���� CAN���ߴ���
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void CAN_Task(void* parameter)
{
	static uint8_t rtc_dat_time = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag1 =~ flag1;
#endif		

		if(DTU_AT_CLKFLAG == 1)
		{
			CanTx_Buf2[0] = 0x02;
			CanTx_Buf2[1] = (Timedat.year%100);
			CanTx_Buf2[2] = Timedat.month;
			CanTx_Buf2[3] = Timedat.day;
			CanTx_Buf2[4] = Timedat.hour;
			CanTx_Buf2[5] = Timedat.minute;
			CanTx_Buf2[6] = Timedat.second;
			CanTx_Buf2[7] = 0x18;
			CAN_SendMsg(0x2001 ,CanTx_Buf2, 8);
			if(RTC_Time_Flag == 0xab)
			{
				DTU_AT_CLKFLAG = 0;
			}
      else if(rtc_dat_time >= 20)
			{
				rtc_dat_time = 0;
				DTU_AT_CLKFLAG = 0;
			}
			else
			{
				rtc_dat_time++;	
			}
#if TEST_PRINTF_MILEAGE
			uint8_t i = 0;
			for(i = 0;i < 8; i++)
			{
				printf("CanTx_Buf2:%d \r\n",CanTx_Buf2[i]);
			}			
#endif
		}
		else
		{
			CanTx_Buf1[0] = 0x02;                     
			CanTx_Buf1[1] = Car_SpeedData;
			CanTx_Buf1[2] = (Oil_base_dat % 100);
			CanTx_Buf1[3] = (Mileage_sum/1000);              //����������Ϊ255km
			CanTx_Buf1[4] = ((int)Mileage_sum%1000/10);
			CanTx_Buf1[5] = Battery_flag;
			CanTx_Buf1[6] = Network_size;		
			CanTx_Buf1[7] = 0x12;
			CAN_SendMsg(0x1998 ,CanTx_Buf1, 8);
		}	

#if TEST_PRINTF_MILEAGE
		printf(" can \r\n ");
		printf(" ����� : %.2f \r\n ",Mileage_sum);
		printf(" �ź� : %d \r\n ",CanTx_Buf1[6]);
 #endif	

		vTaskDelay(200);
  }
}


/**********************************************************************
  * @ ������  �� OIL_Task
  * @ ����˵���� ��������
								�㷨�������������ۼ�ȡƽ����ÿ��Сʱ���»�׼ֵ
                      ��׼ֵ�涨ʱ��4,��1���ӵݼ�1��
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/


static void OIL_Task(void* parameter)
{
	static uint16_t Oil_i = 0;
	uint32_t count_som = 0;
	static uint8_t first_flag = 1;
	static uint8_t oil30_flag = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag2 =~ flag2;
#endif	
		static u8 High_Count = 0;
		if(first_flag == 1)
		{
			oil_first:
			/* ��ȡ��ͨ��ADC�ɼ�ֵ */
			Oil_dat = ADC_SingleMode_GetValue(ADC_Channel_6);
			/* �����߶������ѹ�Ĺ�ϵ High(mm) = Volt(mv) / 5 */
			if(Oil_dat > 100 && Oil_dat < 1400)//&& Oil_dat
			{
#if TEST_PRINTF_OIL
				printf("adc = %d\r\n",Oil_dat); 
#endif
				High[High_Count] = (Oil_dat * 3300*1.8) / 4095 / 5;
				High_Count++;
				if(High_Count >= 100)
				{
					for(u8 i = 0; i < High_Count-1; i++)
					{
						for(u8 j = i+1; j < High_Count; j++)
						{
							if(High[j] < High[i])
							{
								u16 temp = High[i];
								High[i] = High[j];
								High[j] = temp;
							}
						}
					}
#if TEST_PRINTF_OIL
					printf("High[High_Count / 2] = %d\r\n",High[High_Count / 2]);
#endif
					Oil = (High[High_Count / 2] * 100) / TANK_HEIGHT;           //ȥ��λ��
					High_Count = 0;
					printf("Oil = %d",Oil);
					if(Oil <= 99 && Oil >=30)
					{
						Oil_base_dat = Oil;
					}
					if(Oil > 99)
					{
						goto oil_first;
					}
					first_flag = 0;
					oil30_flag = 0;
					TIM_Cmd(OIL_NORMAL_TIM, ENABLE);																									// ʹ�ܼ�����
				}
			}
		}
		else if(oil30_flag == 0)
		{
				/* ��ȡ��ͨ��ADC�ɼ�ֵ */
			Oil_dat = ADC_SingleMode_GetValue(ADC_Channel_6);
			/* �����߶������ѹ�Ĺ�ϵ High(mm) = Volt(mv) / 5 */
			if(Oil_dat > 350)    /* û�⵽Һ��ʱС��50mv������ȥ350 */
			{
				High[High_Count] = (Oil_dat * 3300*1.8) / 4095 / 5;
				High_Count++;
				if(High_Count >= 5)
				{
					for(u8 i = 0; i < High_Count-1; i++)
					{
						for(u8 j = i+1; j < High_Count; j++)
						{
							if(High[j] < High[i])
							{
								u16 temp = High[i];
								High[i] = High[j];
								High[j] = temp;
							}
						}
					}
					Oil = (High[High_Count / 2] * 100) / TANK_HEIGHT;
					High_Count = 0;
				}
		  }
			if(Oil < Oil_base_dat+5&&Oil > Oil_base_dat-5)
			{
				Oil_count[Oil_i++] = Oil;
			}
			if(Oil_i >= 50)
			{
				for(u8 i = 0; i < 50; i++)
				{
					count_som += Oil_count[i];
				}
				Oil_base_dat = count_som/50;
				Oil_i = 0;
			}
#if TEST_PRINTF_OIL
			printf("volt : %d",(Oil_dat * 3300) / 4095);
			printf(" oil��%d\r\n ",Oil);
#endif
		}
		if(Oil_base_dat <= 30)
		{
			oil30_flag = 1;
		}
		if(first_flag == 1)
		{
			vTaskDelay(1);
		}
		else
		{
			vTaskDelay(1000);
		}
	}
}




/**********************************************************************
  * @ ������  �� SPEED_Task
  * @ ����˵���� ����������ʻ�ٶ�
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void SPEED_Task(void* parameter)
{
	EventBits_t r_event;
	BaseType_t xResult;
	xResult = xEventGroupSetBits(EventGroupHandler,EVENTBIT_4);
  while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag3 =~ flag3;
#endif
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_0,pdTRUE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_0) == EVENTBIT_0)
		{
#if TEST_PRINTF_SPEED
			 printf(" speed \r\n ");			
#endif			
			if (Actual_Endflag != 0)                         //�����źŲ������
			{
				if(Total_Time_M1 == 0)
				{
					Car_SpeedData = 0;
					Total_Time_M2 = 0;
					Overflow_Count = 0;
					Actual_Endflag = 0;
//					Detection_update_flag = 0;
				}
				else
				{
					Actual_Endflag = 0;
					if(Total_Time_M1 >= 24)
					{
					  Mileage_count = (Total_Time_M1*1.0/TOOTH_NUM) * 2 * PI * WHEEL_RADIUS/GEAR_RATIO;
					}
					else
					{
						Mileage_count = 0;
					}
					Total_Time_M2 += (50000*Overflow_Count);
					Rotate_Speed=(1000000.0*60*Total_Time_M1)/(TOOTH_NUM*Total_Time_M2);    //����ת��(r/min)
					Car_SpeedData = (Rotate_Speed * 2 * PI * WHEEL_RADIUS /GEAR_RATIO)*3/50;      //ת��Ϊ����(km/h)
#if TEST_PRINTF_SPEED
					printf(" SPEED = %d,M1 = %d,M2 =  %d\r\n ",Car_SpeedData,Total_Time_M1,Total_Time_M2);
  				printf(" SPEED = %d \r\n ",Car_SpeedData);
#endif
					Total_Time_M1 = 0;
					Total_Time_M2 = 0;
					Overflow_Count = 0;
//					Detection_update_flag = 0;
					vTaskDelay(1);
				}
			}
//			if(Detection_update_flag == 0)
//			{
//				printf(" 2 \r\n ");
				TIM_ITConfig(ADVANCE_TIMEX, ADVANCE_TIM_IT_CCx| TIM_IT_Update, ENABLE);   //������/�Ƚ�1�жϺ͸����ж�
				TIM_Cmd(ADVANCE_TIMEX,ENABLE);                                            //�򿪶�ʱ��1��ʱ
				TIM_SetCounter(BASIC_TIMX, 0);                                            //�������ڼ�������				
				TIM_Cmd(BASIC_TIMX,ENABLE);                                               //�򿪶�ʱ��7����ʼ�������ڲ���
//				Detection_update_flag = 1;
//			}
	//		else
	//		printf(" 3 \r\n ");
		}
		else
			printf("�¼�����  \r\n");
  }   	
}

/**********************************************************************
  * @ ������  �� USART_Task
  * @ ����˵���� ���ڴ�ӡ
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

static void USART_Task(void* parameter)
{
	EventBits_t r_event;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag4 =~ flag4;
#endif		
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_1,pdFALSE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_1) == EVENTBIT_1)
		{
			printf("-------------------------------------");
			printf("Speed : %d km/h \r\n",Car_SpeedData);
			printf("Oil : %d km/h \r\n",Oil);
			printf("Mileage : %.2f mk \r\n",Mileage_sum/1000);
			printf("Battery = %d mv��Battery_flag = %d �� \r\n",Battery,Battery_flag);
			printf("Network_Signal = %d �� \r\n",Network_size);		
			printf("-------------------------------------");
			vTaskDelay(10100);
		}
	}
}

/**********************************************************************
  * @ ������  �� Mileage_count_Task
  * @ ����˵���� ��¼������ʻ���
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

static void Mileage_count_Task(void* parameter)
{
	EventBits_t r_event;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag5 =~ flag5;		
#endif
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_2,pdTRUE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_2) == EVENTBIT_2)
		{
			Mileage_sum += Mileage_count;
#if TEST_PRINTF_MILEAGE
			printf("��̼���:%f\r\n",Mileage_count);
			printf("%f\r\n",Mileage_sum);			
#endif
			Mileage_count = 0;
		}
	}
}

/**********************************************************************
  * @ ������  �� SD_RWTask
  * @ ����˵���� ������д�복����ʻ���
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

static void SD_RWTask(void* parameter)
{
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
			flag6 =~ flag6;	
 #endif
		if(SD_Mileage_sum_flag == 1)
		{
//			taskENTER_CRITICAL();																								 //�����ٽ���
			res_flash = f_open(&fnew, "1:�������/U12�������.txt",FA_OPEN_EXISTING | FA_WRITE);
			if ( res_flash == FR_OK )
			{
				uint16_t b_size = 0;
#if TEST_PRINTF_SD
				printf("����U12�������.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");				
#endif
				if (res_flash == FR_OK)
				{
					b_size = 0x22;                                                   //��ֵͨ���鿴1:�������/��¼�����λ��.txt���
					/* д��������� */
					sprintf(Fdat,"%.1f",Mileage_sum);
					res_flash = f_lseek(&fnew,b_size);
					f_printf(&fnew,"%20s",Fdat);
#if TEST_PRINTF_SD
					printf("����д�����\n");					
#endif
				}
//				taskEXIT_CRITICAL();                                               //�˳��ٽ���
			}
			f_close(&fnew);
		}
		else
		{
			taskENTER_CRITICAL();
			Mileage_sum = SD_ReadtoMileage_count();
#if TEST_PRINTF_SD
			printf("Mileage_sum: %.1f\r\n",Mileage_sum);
#endif
			SD_Mileage_sum_flag = 1;
			taskEXIT_CRITICAL(); 
	  }
		vTaskDelay(1010);
  }
}
/**********************************************************************
  * @ ������  �� SD_ReadtoMileage_count
  * @ ����˵���� ����SD���еĳ�����ʻ���
  * @ ����    ��   
  * @ ����ֵ  �� Readdat
  ********************************************************************/
static float SD_ReadtoMileage_count(void)
{
	DIR dir;
	float Readdat = 0;
	char floatdat[50];
	uint32_t b_size = 0;
				/* ���Դ�Ŀ¼ */
	res_flash=f_opendir(&dir,"1:�������");
	if(res_flash!=FR_OK)
	{
		float starter = 0.0;
		char BSIZE[10] = {0};
		/* ��Ŀ¼ʧ�ܣ��ʹ���Ŀ¼ */
		res_flash=f_mkdir("1:�������");
		if(res_flash != FR_OK)
		{
			printf("�����ļ�ʧ��(%d)",res_flash);				
		}
		res_flash=f_closedir(&dir);
		res_flash = f_open(&fnew, "1:�������/U12�������.txt",FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		if ( res_flash == FR_OK )
		{
#if TEST_PRINTF_SD
			printf("����/����U12�������.txt�ļ��ɹ������ļ�д�����ݡ�\n");
#endif
			if (res_flash == FR_OK)
			{
				res_flash = f_lseek(&fnew,0);
				/* ��ʽ��д�룬������ʽ����printf���� */
				f_printf(&fnew,"��������U12��̼�¼\n");
				f_printf(&fnew,"����̴�С��");
				b_size = f_size(&fnew);
#if TEST_PRINTF_SD				
				printf("�������ݴ�С��0x%x\n",b_size);
#endif				
				sprintf(floatdat,"%.1f",starter);
				res_flash = f_lseek(&fnew,b_size);
				f_printf(&fnew,"%20s",floatdat);
				res_flash = f_lseek(&fnew,0);
				/* ��ȡ�ļ��������ݵ������� */
				res_flash = f_read(&fnew,ReadBuffer,f_size(&fnew),&fnum);
				if(res_flash == FR_OK)
				{
#if TEST_PRINTF_SD					
					printf("���ļ����ݣ�\n%s\n",ReadBuffer);
#endif
				}
			}
		}
		f_close(&fnew);
		res_flash = f_open(&fnew, "1:�������/��¼�����λ��.txt",FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		sprintf(BSIZE,"0x%x",b_size);
		f_printf(&fnew,"%s",BSIZE);
		res_flash = f_lseek(&fnew,0);
		/* ��ȡ�ļ��������ݵ������� */
		res_flash = f_read(&fnew,ReadBuffer,f_size(&fnew),&fnum);
		if(res_flash == FR_OK)
		{
#if TEST_PRINTF_SD
			printf("���ļ����ݣ�\n%s\n",ReadBuffer);
#endif
		}
		f_close(&fnew);
	}
	else
	{
		res_flash=f_closedir(&dir);
		res_flash = f_open(&fnew, "1:�������/U12�������.txt",FA_OPEN_EXISTING | FA_WRITE | FA_READ);
		if ( res_flash == FR_OK )
		{
#if TEST_PRINTF_SD
			printf("����U12�������.txt�ļ��ɹ������ļ���ȡ���ݡ�\n");
#endif
			if (res_flash == FR_OK)
			{
				b_size = 0x22;                                               //��ֵͨ���鿴1:�������/��¼�����λ��.txt���
				res_flash = f_lseek(&fnew,b_size);
				/* ��ȡ������� */
				res_flash = f_read(&fnew,ReadBuffer,f_size(&fnew),&fnum);
				if(res_flash == FR_OK)
				{
#if TEST_PRINTF_SD
					printf("\n%s\n",ReadBuffer);
#endif
					Readdat = atof(ReadBuffer);
#if TEST_PRINTF_SD
					printf("�������ݣ�\r\n%.1f\r\n",Readdat);
#endif					
				}
				f_close(&fnew);
			}
		}
		else
		{
			printf("�������ļ�ʧ�ܡ�(%d)\r\n",res_flash);
		}
	}
	return Readdat;
}


/**********************************************************************
  * @ ������  �� Battery_capacity_Task
  * @ ����˵���� ��ص�����⺯��
			������       Battery >3270 mv
			5�����      3190 < Battery < 3270 mv
			4�����      3100 < Battery < 3190 mv
			3�����      3020 < Battery < 3100 mv
			2�����      2930 < Battery < 3020 mv
			1�����      2870 < Battery < 2930 mv
			0�����      Battery < 2850 mv
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

static void Battery_capacity_Task(void* parameter)
{
	while(1)
	{
		Battery = ADC_SingleMode_GetValue(ADC_Channel_4)*3300/4096;
#if TEST_PRINTF_SIMULATOR
		flag7 =~ flag7;
#endif
#if TEST_PRINTF_BATTERY
		printf("Battery = %d mv \r\n",Battery);	
#endif
		if(Battery >3270)
		{Battery_flag = 6;
		}
		else if( Battery > 3190&&Battery < 3270)
		{Battery_flag = 5;
		}
		else if( Battery > 3100&&Battery < 3190)
		{Battery_flag = 4;
		}
		else if( Battery > 3020&&Battery < 3100)
		{Battery_flag = 3;
		}
		else if( Battery > 2930&&Battery < 3020)
		{Battery_flag = 2;
		}
		else if( Battery > 2870&&Battery < 2930)
		{Battery_flag = 1;
		}
		else if(Battery < 2850)
		{Battery_flag = 0;
		}
#if TEST_PRINTF_BATTERY
		printf("Battery_flag = %d �� \r\n",Battery_flag);		
#endif
    if(Battery_flag <= 1&&DTUTask1 == 0)
		{
			BKP_WriteBackupRegister(BKP_DR40, 0x18);
			__set_FAULTMASK(1);
			NVIC_SystemReset();
		}
		else if(Battery_flag <= 1&&DTUTask1 == 1)
		{
			TIM_Cmd(CPU_NORMAL_TIM, ENABLE);
		}
		vTaskDelay(30050);
	}
}


/**********************************************************************
  * @ ������  �� CPU_Task
  * @ ����˵���� CPU������ͳ�ƺ���
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

#if TEST_PRINTF_CPU

static void CPU_Task(void* parameter)
{	
  uint8_t CPU_RunInfo[400];		//������������ʱ����Ϣ
  
  while (1)
  {
    memset(CPU_RunInfo,0,400);				//��Ϣ����������
    
    vTaskList((char *)&CPU_RunInfo);  //��ȡ��������ʱ����Ϣ
    
    printf("---------------------------------------------\r\n");
    printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n");
    
    memset(CPU_RunInfo,0,400);				//��Ϣ����������
    
    vTaskGetRunTimeStats((char *)&CPU_RunInfo);
    
    printf("������       ���м���         ʹ����\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n\n");
    vTaskDelay(1000);   /* ��ʱ500��tick */		
  }
}

#endif
/**********************************************************************
  * @ ������  �� OneNET_4G_DTU_Task
  * @ ����˵���� ͨ��4G DTU��OneNET�й��ƶ�������ƽ̨���ͳ�����̬����
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

static void OneNET_4G_DTU_Task(void* pvParameters)                         
{
	EventBits_t r_event;
	int res = 0;
	uint8_t i = 0;
	static uint8_t k = 0;
	while(1)
	{
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_4,pdFALSE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_4) == EVENTBIT_4)
		{
	#if TEST_PRINTF_ONENET
			printf("OneNET_4G_DTU_Task\r\n");
	#endif
	#if TEST_PRINTF_SIMULATOR
			flag8 =~ flag8;
	#endif
			if(DTUMode_Switch_flag == 0)             															 //�ж��Ƿ��ڿ��ƽ׶� 0��������״̬ 1����͸��״̬
			{
				/*1.DTU��������״̬*/
				while(i<10)
				{
					res = dtu_enter_configmode();
					if ( res != 0 )
					{
						printf("��������ʧ��\r\n");
						i++;
						vTaskDelay(10);
					}
					else
					{
						i = 0;
						break;
					}
				}
				/*DTU����͸��״̬*/
				res = dtu_enter_transfermode();
				if( res != 0 )
				{
						printf("DTU����͸��״̬ʧ��\r\n");
				}
				else
				{
					DTUMode_Switch_flag = 1;
					printf("DTU����͸��״̬\r\n");
					OneNet_FillBuf(Send_date);   
					send_data_to_dtu(&Sdat[0], sizeof(Sdat[0]));
					send_data_to_dtu(&Sdat[1], sizeof(Sdat[1]));
					send_data_to_dtu(&Sdat[2], sizeof(Sdat[2]));
					if(k<=10)
					{
						send_data_to_dtu(Send_date[0], sizeof(Send_date[0]));
						delay_xms(50);
						send_data_to_dtu(Send_date[1], sizeof(Send_date[1]));
						delay_xms(50);
						k++;					
					}
					else
					{
						k = 0;
						send_data_to_dtu(Send_date[2], sizeof(Send_date[2]));
						delay_xms(50);
						send_data_to_dtu(Send_date[3], sizeof(Send_date[3]));
						delay_xms(50);
						send_data_to_dtu(Send_date[4], sizeof(Send_date[4]));					
					}			
#if TEST_PRINTF_ONENET
#endif
				}
			}
			else
			{
				OneNet_FillBuf(Send_date);
				if(k <= 10)
				{
					send_data_to_dtu(Send_date[0], sizeof(Send_date[0]));
					delay_xms(50);
					send_data_to_dtu(Send_date[1], sizeof(Send_date[1]));
					delay_xms(50);
					k++;					
				}
				else
				{
					k = 0;
					send_data_to_dtu(Send_date[2], sizeof(Send_date[2]));
					delay_xms(50);
					send_data_to_dtu(Send_date[3], sizeof(Send_date[3]));
					delay_xms(50);
					send_data_to_dtu(Send_date[4], sizeof(Send_date[4]));					
				}
			}
			xEventGroupSetBits(EventGroupHandler,EVENTBIT_3);
			vTaskDelay(3510);
		}
	}
}

/**********************************************************************
  * @ ������  �� DTU_4G_Task
  * @ ����˵���� ͨ��4G DTU��ȡ�����źţ�ͨ��4G DTU��ȡʵʱʱ��
  * 	�����ź�     CSQvalue >14
			4���ź�      9 < CSQvalue <= 14
			3���ź�      5 < CSQvalue <= 9
			2���ź�      2 < CSQvalue <= 5
			1���ź�      1 < CSQvalue <= 2
			0���ź�      CSQvalue = 0
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/



static void DTU_4G_Task(void* pvParameters)                              
{
	BaseType_t xReturn = pdPASS;
	uint8_t i = 0,res = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag9 =~ flag9;
#endif		
		xReturn = xSemaphoreTake(MuxSem_Handle,/* ��������� */
															portMAX_DELAY); /* �ȴ�ʱ�� */
#if TEST_PRINTF_4GDTU		
		if(xReturn!=0)
		{printf("DTU_4G_Task_Handle ��û�����ʧ��!\r\n");		
		}
		else
		{printf("DTU_4G_Task_Handle ��û�����!\r\n");
			printf("DTU_4G_Task_Handle\r\n");
		}
#endif
    if(DTU_Time_flag == 1)
		{
#if 	TEST_PRINTF_4GDTU
			printf("Signal_4G_DTU_Task\r\n");
#endif
				while(i<10)
				{
					/*1.DTU��������״̬*/
					res = dtu_enter_configmode();
					if ( res != 0 )
					{
						printf("��������ʧ��\r\n");
						vTaskDelay(10);
						i++;
					}
					else
					{
						i = 0;
						DTUMode_Switch_flag = 0;
						send_data_to_dtu("AT+CSQ\r\n", strlen("AT+CSQ\r\n"));
#if 		  TEST_PRINTF_4GSIGNAL
						printf("AT+CSQ\r\n");
#endif
						break;
					}
				}
		}		
    else
		{
				while(i<10)
				{
					/*1.DTU��������״̬*/
					res = dtu_enter_configmode();
					if ( res != 0 )
					{
						printf("��������ʧ��\r\n");
						vTaskDelay(10);
						i++;
					}
					else
					{			
						i = 0;
						DTUMode_Switch_flag = 0;
						send_data_to_dtu("AT+CLK\r\n", strlen("AT+CLK\r\n"));
						DTU_Time_flag = 1;
#if TEST_PRINTF_4GDTU				
						printf("AT+CLK\r\n");
#endif	
						break;
					}
				}
		}
	  xReturn = xSemaphoreGive( MuxSem_Handle );//����������
#if TEST_PRINTF_4GDTU
		if(xReturn != 0)
		{		printf("DTU_4G_Task_Handle �ͷŻ�����!\r\n");
		}
		else
		{}
#endif	
			vTaskDelay(20050);
  }
}

/**********************************************************************
  * @ ������  �� RingBuffer_Read_Task
  * @ ����˵���� ����4G DTU���ص������뾯��
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

static void RingBuffer_Read_Task(void* pvParameters)
{
	BaseType_t xReturn = pdPASS;
	uint8_t buf = 0;
	char *token;
	char DTU_Dat[6][64] = {0};
	const char DTU_ATCLK[] = "AT+CLK";
	const char DTU_ATCSQ[] = "AT+CSQ";
	uint8_t i = 0,CSQvalue = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
	  flag11 =~ flag11;	
#endif	

		xReturn = xSemaphoreTake(MuxSem_Handle,/* ��������� */
													portMAX_DELAY); /* �ȴ�ʱ�� */
#if TEST_PRINTF_RINGBUFFER
		if(xReturn != 0 )
		{}
		else
    {printf("RingBuffer_Read_Task ��ȡ������!\r\n");	
		}								
#endif
		if (RingBuffer_Len(p_uart2_rxbuf) > 0)          /*���յ�DTU���͹����ķ���������*/
		{
				RingBuffer_Out(p_uart2_rxbuf, &buf, 1);
				dtu_rxbuf[dtu_rxlen++] = buf;
				dtu_get_urc_info(buf);                      /*����DTU�ϱ���URC��Ϣ*/
				if (dtu_rxlen >= DTU_ONENETDATE_RX_BUF)     /*���ջ������*/
				{
						usart1_send_data(dtu_rxbuf, dtu_rxlen); /*���յ���DTU���������������ݣ�ת�������Դ���1���*/
						dtu_rxlen = 0;
				}
		}
		else
		{
				if (dtu_rxlen > 0)
				{
#if TEST_PRINTF_RINGBUFFER
						usart1_send_data(dtu_rxbuf, dtu_rxlen); /*���յ���DTU���������������ݣ�ת�������Դ���1���*/
#endif
					  if(strcmp((char *)dtu_rxbuf,ONENET_COM_OFF)== 0)
						{
#if TEST_PRINTF_RINGBUFFER							
							printf("data = %s\r\n",dtu_rxbuf);
#endif							
							ONENET_OFF_FLAG = 1;
							xEventGroupClearBits(EventGroupHandler,EVENTBIT_4);
						}
						else if(strcmp((char *)dtu_rxbuf,ONENET_COM_ON)== 0)
						{
							ONENET_OFF_FLAG = 0;
							xEventGroupSetBits(EventGroupHandler,EVENTBIT_4);							
						}
						/* ��ȡ��һ�����ַ��� */
						token = strtok((char *)dtu_rxbuf, sqpa[0]);
						/* ������ȡ���������ַ��� */
						while( token != NULL )
						{
							i++;
							strcpy(DTU_Dat[i],token);
#if TEST_PRINTF_RINGBUFFER
							printf( "%s\r\n", DTU_Dat[i]);					
#endif
							token = strtok(NULL, sqpa[0]);
						}
						if(strcmp(DTU_Dat[1],DTU_ATCSQ) == 0)
						{
							CSQvalue = DTU_AT_CSQ_DataAnalyze(DTU_Dat);
							if(CSQvalue > 14){
							Network_size = 5;}
							else if(CSQvalue > 9&&CSQvalue <= 14){
							Network_size = 4;}
							else if(CSQvalue > 5&&CSQvalue <= 9){
							Network_size = 3;}
							else if(CSQvalue > 2&&CSQvalue <= 5){
							Network_size = 2;}
							else if(CSQvalue > 1&&CSQvalue <= 2){
							Network_size = 1;}
							else if(CSQvalue == 0){
							Network_size = 0;}
#if TEST_PRINTF_RINGBUFFER
						  printf("CSQvalue = %d\r\n",CSQvalue);	
#endif
						}
						else if(strcmp(DTU_Dat[1],DTU_ATCLK) == 0)
						{
							DTU_AT_CLK_DataAnalyze(DTU_Dat);
							DTU_AT_CLKFLAG = 1;
#if TEST_PRINTF_RINGBUFFER
							printf("%d \r\n",Timedat.year);
							printf("%d \r\n",Timedat.month);
							printf("%d \r\n",Timedat.day);
							printf("%d \r\n",Timedat.hour);
							printf("%d \r\n",Timedat.minute);
							printf("%d \r\n",Timedat.second);
						  printf("CLKvalue = %s\r\n",DTU_Dat[1]);
#endif	
						}
						i = 0;
						dtu_rxlen = 0;
				}	
		}
	  xReturn = xSemaphoreGive( MuxSem_Handle );//����������
#if TEST_PRINTF_RINGBUFFER
		if(xReturn != 0)
		{}
		else
		{printf("RingBuffer_Read_Task �ͷŻ�����!\r\n");
		}
#endif
		if(ONENET_OFF_FLAG == 1)
		{
			xEventGroupSetBits(EventGroupHandler,EVENTBIT_3);
		}
			vTaskDelay(3);	  
	}
}


/**********************************************************************
  * @ ������  �� IWDG_Task
  * @ ����˵���� ��ֹ4G DTU-Onenet�����ܷ�
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void IWDG_Task(void* pvParameters)
{
	EventBits_t r_event;
	while(1)
	{
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_3,pdTRUE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_3) == EVENTBIT_3)
		{
#if TEST_IWDG
			printf("IWDG_ReloadCounter \r\n");
 #endif
			IWDG_ReloadCounter();
		}
	}
}


static void OneNet_FillBuf(uint8_t buff[][64])
{
	char text[64] = {0};
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_Car_Speed':'%d'}", Car_SpeedData); //FWT_Car_Speed����������һ�����ƣ�Car_SpeedData���ٶ�ֵ;
#if TEST_PRINTF_ONENET
	printf("Car_SpeedData = %d\r\n",Car_SpeedData);	
#endif
	strcpy((char*)buff[0], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_Car_Oil':'%d'}", Oil);
#if TEST_PRINTF_ONENET
		printf("Oil = %d\r\n",Oil);
#endif
	strcpy((char*)buff[1], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_Car_Mileage':'%.2f'}", Mileage_sum/1000);
#if TEST_PRINTF_ONENET
	printf("Mileage_sum = %.2f\r\n",Mileage_sum/1000);	
#endif
	strcpy((char*)buff[2], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_VCUBattery':'%d'}", Battery_flag);
#if TEST_PRINTF_ONENET
	printf("Battery_flag = %d\r\n",Battery_flag);	
#endif	
	strcpy((char*)buff[3], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_4G_Signal':'%d'}", Network_size);
#if TEST_PRINTF_ONENET
	printf("Network_size = %d\r\n",Network_size);	
#endif	
	strcpy((char*)buff[4], text);	
}
