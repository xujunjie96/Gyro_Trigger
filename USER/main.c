#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "usart2.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "timer.h"



int	trigger_interval = 500;	//默认触发间隔
int roll_angle = 30;	//默认滚转角
int w_symbol = 1;	//默认角度符号	0:负 1:正
int interval = 50;	//角度间隔

float gyro_x;	//角速度	
float angle_x;	//角度

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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);		//延时初始化 
	uart_init(115200);	//串口初始化波特率为115200
	My_USART2_Init(115200);
	LED_Init();		  		//初始化与LED连接的硬件接口  
		TIM3_Int_Init(trigger_interval*10-100-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms   
	while(1)
	{
		if(USART_RX_STA&0x8000)
		{					   
			if(usbusart_handle())
			{
				printf("触发角度:%d;触发角速度符号:%d;触发间隔:%d;角度精度:%.2f\r\n",roll_angle,w_symbol,trigger_interval,(float)interval/100);			
				TIM3_Int_Init(trigger_interval*10-100-1,8400-1);				
			}
			else
			{
				printf("你个傻屌，看协议！\r\n");
			}
		}
		if(read_flag==1)
		{
		//输出角速度
	//	printf("Gyro:%.3f %.3f %.3f\r\n",(float)stcGyro.w[0]/32768*2000,(float)stcGyro.w[1]/32768*2000,(float)stcGyro.w[2]/32768*2000);
		//输出角度
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
						TIM_Cmd(TIM3,ENABLE); //使能定时器3
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
						TIM_Cmd(TIM3,ENABLE); //使能定时器3
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
	int	trigger_interval_temp=0;	//临时触发间隔
	int roll_angle_temp=0;	//临时滚转角
	int w_symbol_temp;	//临时角速度符号
	int interval_temp = 0;	//临时角速度符号
	
	int i=0;
	//int temp=0;
	if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			if (len == 13)	//判断是否为9位
			{
				if(USART_RX_BUF[3]==',' && USART_RX_BUF[5]==','&& USART_RX_BUF[9]==',')	//判断逗号
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


