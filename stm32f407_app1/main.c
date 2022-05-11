#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "flash.h"
#include "key.h"
#include "tim.h"
#include "ymodem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//������  
int main(void)
{
	//������������ǰ����������5
	SCB->VTOR = 0x8020000;
	
	//led��ʼ��
	led_init();
	
	//beep��ʼ��
	beep_init();	
	
	//�������
	key_init();
	
	tim3_init();
	
	//����1��ʼ��������Ϊ115200bps
	usart1_init(115200);
	
	//�����ӳ�һ�ᣬȷ��оƬ�ڲ����ȫ����ʼ��,printf���������
	delay_ms(1000);
		
	//��������
	printf("This is app1 with ymodem test by teacher.wen\r\n");
	
	
	
	while(1)
	{
		ymodem_download();
	}
	
	
	return 0;
}

