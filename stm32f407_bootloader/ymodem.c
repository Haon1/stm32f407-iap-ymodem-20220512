#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "flash.h"
#include "key.h"
#include "ymodem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/**
 * @bieaf CRC-16 У��
 *
 * @param addr ��ʼ��ַ
 * @param num   ����
 * @param num   CRC
 * @return crc  ����CRC��ֵ
 */
#define POLY        0x1021  
uint16_t crc16(uint8_t *addr, int32_t num, uint16_t crc)  
{  
    int32_t i;  
    for (; num > 0; num--)					/* Step through bytes in memory */  
    {  
        crc = crc ^ (*addr++ << 8);			/* Fetch byte from memory, XOR into CRC top byte*/  
        for (i = 0; i < 8; i++)				/* Prepare to rotate 8 bits */  
        {
            if (crc & 0x8000)				/* b15 is set... */  
                crc = (crc << 1) ^ POLY;  	/* rotate and XOR with polynomic */  
            else                          	/* b15 is clear... */  
                crc <<= 1;					/* just rotate */  
        }									/* Loop for 8 bits */  
        crc &= 0xFFFF;						/* Ensure CRC remains 16-bit value */  
    }										/* Loop until num=0 */  
    return(crc);							/* Return updated CRC */  
}



/* ���������Ĳ��� */
static enum UPDATE_STATE update_state = TO_START;

void ymodem_set_state(enum UPDATE_STATE state)
{
	update_state = state;
}


/* ��ѯ�����Ĳ��� */
uint8_t ymodem_get_state(void)
{
	return update_state;
}




/* ����ָ�� */
void ymodem_send_cmd(uint8_t command)
{

	USART_SendData(USART1,command);

	//�ȴ����ݷ��ͳɹ�
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	USART_ClearFlag(USART1,USART_FLAG_TXE);
	
	delay_ms(10);
}

/* ���������� */
void update_set_down(void)
{
	uint32_t update_flag = 0xAAAAAAAA;				// ��Ӧbootloader����������
	
	flash_program((APPLICATION_2_ADDR + APPLICATION_2_SIZE - 4), &update_flag,1 );
}


/**
 * @bieaf ymodem����
 *
 * @param none
 * @return none
 */
void ymodem_download(void)
{
	uint16_t crc = 0;

static 
	uint8_t data_state = 0;

	if(ymodem_get_state()==TO_START)
	{
		ymodem_send_cmd(CCC);
		
		delay_ms(1000);
	}
	
	/* ����1������һ�����ݰ� */
	if(g_usart1_rx_end)    	
	{

		
		/* ��ս�����ɱ�־λ�����ռ���ֵ */
		g_usart1_rx_end=0;
		g_usart1_rx_cnt=0;
		
		switch(g_usart1_rx_buf[0])
		{
			case SOH://���ݰ���ʼ
			{
					crc = 0;
				
					/* ����crc16 */
					crc = crc16((uint8_t *)&g_usart1_rx_buf[3], 128, crc);
						
					if(crc != (g_usart1_rx_buf[131]<<8|g_usart1_rx_buf[132]))
						return;
					
					if((ymodem_get_state()==TO_START)&&(g_usart1_rx_buf[1] == 0x00)&&(g_usart1_rx_buf[2] == (uint8_t)(~g_usart1_rx_buf[1])))// ��ʼ
					{

						ymodem_set_state(TO_RECEIVE_DATA);
						
						/* ��ymodem_send_cmdִ����sector_erase֮ǰ�����´������ݶ�������Ϊ������ر������ж� */
						/* ����Ӧ�ó���2������ */
						sector_erase(APPLICATION_2_SECTOR);						
						
						data_state = 0x01;						
						ymodem_send_cmd(ACK);
						ymodem_send_cmd(CCC);

	
					}
					else if((ymodem_get_state()==TO_RECEIVE_END)&&(g_usart1_rx_buf[1] == 0x00)&&(g_usart1_rx_buf[2] == (uint8_t)(~g_usart1_rx_buf[1])))// ����
					{
						update_set_down();						
						ymodem_set_state(TO_START);
						ymodem_send_cmd(ACK);
						
						/* ��һ��ʾ����ʾ������� */
						beep_on();delay_ms(80);beep_off();
						
						/* ��λ */
						NVIC_SystemReset();
					}					
					else if((ymodem_get_state()==TO_RECEIVE_DATA)&&(g_usart1_rx_buf[1] == data_state)&&(g_usart1_rx_buf[2] == (uint8_t)(~g_usart1_rx_buf[1])))// ��������
					{

						/* ��¼���� */
						flash_program((APPLICATION_2_ADDR + (data_state-1) * 128), (uint32_t *)(&g_usart1_rx_buf[3]), 32);
						data_state++;
						
						ymodem_send_cmd(ACK);		
					}
			}break;
			
			case EOT://���ݰ��������
			{
				if(ymodem_get_state()==TO_RECEIVE_DATA)
				{

					ymodem_set_state(TO_RECEIVE_EOT2);					
					ymodem_send_cmd(NACK);
				}
				else if(ymodem_get_state()==TO_RECEIVE_EOT2)
				{
	
					
					ymodem_set_state(TO_RECEIVE_END);					
					ymodem_send_cmd(ACK);
					ymodem_send_cmd(CCC);
				}
	
			}break;	
			
			default:break;
		}

	}
}



