#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "usart2.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "timer.h"



int	trigger_interval = 500;	//Ĭ�ϴ������
int roll_angle = 30;	//Ĭ�Ϲ�ת��
int w_symbol = 1;	//Ĭ�ϽǶȷ���	0:�� 1:��
int interval = 50;	//�Ƕȼ��

float gyro_x;	//���ٶ�	
float angle_x;	//�Ƕ�

int usbusart_handle(void);

extern int  read_flag ;
extern int trig_flag;
extern struct SGyro 		stcGyro;
extern struct SAngle 	stcAngle;

int main(void)
{ 
 
	u8 t;
	u8 len;	
	u16 times=0;  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);		//��ʱ��ʼ�� 
	uart_init(115200);	//���ڳ�ʼ��������Ϊ115200
	My_USART2_Init(115200);
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�  
		TIM3_Int_Init(trigger_interval*10-100-1,8400-1);	//��ʱ��ʱ��84M����Ƶϵ��8400������84M/8400=10Khz�ļ���Ƶ�ʣ�����5000��Ϊ500ms   
	while(1)
	{
		if(USART_RX_STA&0x8000)
		{					   
			if(usbusart_handle())
			{
				printf("�����Ƕ�:%d;�������ٶȷ���:%d;�������:%d;�ǶȾ���:%.2f\r\n",roll_angle,w_symbol,trigger_interval,(float)interval/100);			
				TIM3_Int_Init(trigger_interval*10-100-1,8400-1);				
			}
			else
			{
				printf("���ɵ�ţ���Э�飡\r\n");
			}
		}
		if(read_flag==1)
		{
		//������ٶ�
	//	printf("Gyro:%.3f %.3f %.3f\r\n",(float)stcGyro.w[0]/32768*2000,(float)stcGyro.w[1]/32768*2000,(float)stcGyro.w[2]/32768*2000);
		//����Ƕ�
	//	printf("Angle:%.3f %.3f %.3f\r\n",(float)stcAngle.Angle[0]/32768*180,(float)stcAngle.Angle[1]/32768*180,(float)stcAngle.Angle[2]/32768*180);
			gyro_x=(float)stcGyro.w[0]/32768*2000;
			angle_x=(float)stcAngle.Angle[0]/32768*180;
			printf("Gyro:%.3f Angle:%.3f\r\n",gyro_x,angle_x);
			if(w_symbol==1)
			{
				//if((gyro_x>0)&&(angle_x<=(roll_angle+0.01))&&(angle_x>=(roll_angle-0.01)))
				if((gyro_x>0)&&(angle_x<=(roll_angle+(float) (interval/100) ))&&(angle_x>=(roll_angle-(float) (interval/100))))
				//if((gyro_x>=0)&&(angle_x<=(0.43+0.01))&&(angle_x>=(0.44-0.01)))
				{
					
					if(trig_flag==1)
					{
						LED1=1;
						delay_ms(10);
						printf("trig\r\n");
						LED1=0;
						trig_flag=0;
						TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
					}
				}
			}
			else 
			{
				//if((gyro_x<0)&&(angle_x<=(roll_angle+0.01))&&(angle_x>=(roll_angle-0.01)))
				if((gyro_x<0)&&(angle_x<=(roll_angle+(float) (interval/100)))&&(angle_x>=(roll_angle-(float) (interval/100))))
				{
					if(trig_flag==1)
					{
						LED1=1;
						delay_ms(10);
						printf("trig\r\n");
						LED1=0;
						trig_flag=0;
						TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
					}
				}
			}
			read_flag=0;
		}
	}
}

int usbusart_handle(void)
{
	u8 len;	
	int	trigger_interval_temp=0;	//��ʱ�������
	int roll_angle_temp=0;	//��ʱ��ת��
	int w_symbol_temp;	//��ʱ���ٶȷ���
	int interval_temp = 0;	//��ʱ���ٶȷ���
	
	int i=0;
	//int temp=0;
	if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			if (len == 13)	//�ж��Ƿ�Ϊ9λ
			{
				if(USART_RX_BUF[3]==',' && USART_RX_BUF[5]==','&& USART_RX_BUF[9]==',')	//�ж϶���
				{
					for(i=0;i<3;i++)
					{
						if ((USART_RX_BUF[i]>='0')&&(USART_RX_BUF[i]<='9'))
						{
							roll_angle_temp=(roll_angle_temp*10+ (USART_RX_BUF[i]-48));
						}
						else
						{
							USART_RX_STA=0;
							return 0;
						}
					}
					
					if (USART_RX_BUF[4]=='0')
					{
						w_symbol_temp = 0;
					}
					else if (USART_RX_BUF[4]=='1')
					{
						w_symbol_temp = 1;
					}
					else
					{
						USART_RX_STA=0;
						return 0;
					}
					
					for(i=6;i<9;i++)
					{
						if ((USART_RX_BUF[i]>='0')&&(USART_RX_BUF[i]<='9'))
						{
							trigger_interval_temp=(trigger_interval_temp*10+ (USART_RX_BUF[i]-48));
						}
						else
						{
							USART_RX_STA=0;
							return 0;
						}
					}
					
					for(i=10;i<13;i++)
					{
						if ((USART_RX_BUF[i]>='0')&&(USART_RX_BUF[i]<='9'))
						{
							interval_temp=(interval_temp*10+ (USART_RX_BUF[i]-48));
						}
						else
						{
							USART_RX_STA=0;
							return 0;
						}
					}
					
					trigger_interval = trigger_interval_temp;
					if(trigger_interval<20)
					{
						trigger_interval=20;
					}
					if(roll_angle_temp<=180)
					{
						roll_angle = roll_angle_temp;
					}
					else
					{
						roll_angle = roll_angle_temp-360;
					}
					w_symbol = w_symbol_temp;
					interval = interval_temp;
					USART_RX_STA=0;
					return 1;
				}
				else
				{
					USART_RX_STA=0;
					return 0;
				}
		
			}
			else
			{
				USART_RX_STA=0;
				return 0;
			}
		}
}


