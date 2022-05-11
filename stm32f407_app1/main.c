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


//主函数  
int main(void)
{
	//重设向量表，当前代码在扇区5
	SCB->VTOR = 0x8020000;
	
	//led初始化
	led_init();
	
	//beep初始化
	beep_init();	
	
	//按键检测
	key_init();
	
	tim3_init();
	
	//串口1初始化波特率为115200bps
	usart1_init(115200);
	
	//串口延迟一会，确保芯片内部完成全部初始化,printf无乱码输出
	delay_ms(1000);
		
	//发送数据
	printf("This is app1 with ymodem test by teacher.wen\r\n");
	
	
	
	while(1)
	{
		ymodem_download();
	}
	
	
	return 0;
}

