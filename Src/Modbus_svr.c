/***************************************************
 * Copyright (C),2019 www.idsse.ac.cn
 * Written by chenming
 * Version 1.0
 * Data  2019-3-3
 * Description: modbus接口实现文件
 * *************************************************/

#include <stdio.h>
#include "Modbus_svr.h"
#include "usart.h"
#include "SysTick.h"

Modbus_block mblock1;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

uint8_t pow1_frame[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00};
uint8_t pow2_frame[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00};
uint8_t pow1_buf[100] ;
uint8_t pow2_buf[100] ;
uint8_t ptr1,ptr2 ;
uint8_t pow1_ch, pow2_ch ;
uint8_t pow1_bRecv = 1;
uint8_t pow2_bRecv = 1;
u32 tick1_last;
u32 tick2_last;

//-------------------------------------------------------------------------------
//	@brief	协议栈初始化
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void Modbus_init(void)
{
	char msg[100];

	ModbusSvr_block_init(&mblock1, &huart2);

	sprintf(msg, "\r\nStation No: %d, Baudrate: %d", mblock1.station, mblock1.baudrate);
	Usart_SendString(&huart2, msg);
	sprintf(msg, "\r\nCoil Start adr: %4d, Len: %4d", mblock1.uCoilStartAdr, mblock1.uCoilLen);
	Usart_SendString(&huart2, msg);
	sprintf(msg, "\r\nReg  Start adr: %4d, Len: %4d", mblock1.uRegStartAdr, mblock1.uRegLen);
	Usart_SendString(&huart2, msg);
	sprintf(msg, "\r\nRom  Start adr: %4d, Len: %4d", mblock1.uRomStartAdr, mblock1.uRomLen);
	Usart_SendString(&huart2, msg);
}

//-------------------------------------------------------------------------------
//	@brief	协议任务调度
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void Modbus_task(void)
{
	ModbusSvr_task(&mblock1);
}

//-------------------------------------------------------------------------------
//	@brief	modbus recieve counter
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void ModbusTimer(void)
{
	mblock1.nMBInterval++;
}


//-------------------------------------------------------------------------------
//	@brief	HAL_UART_RxCpltCallback
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if ( huart == &huart2)
	{
		Modbus_UsartHandler(&mblock1) ;
	}
	
	if ( huart == &huart1)
	{
		mblock1.ptrRegs[20+ptr2] = pow1_ch ;
		pow1_buf[ptr1++] = pow1_ch ;
		HAL_UART_Receive_IT(&huart1, &pow1_ch, 1);
	}
	
	if ( huart == &huart3)
	{
		pow2_buf[ptr2++] = pow2_ch ;
		mblock1.ptrRegs[40+ptr2] = pow2_ch ;
		HAL_UART_Receive_IT(&huart3, &pow2_ch, 1);
	}
}


//-------------------------------------------------------------------------------
//	@brief	Modbus_SavePara
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void Modbus_SavePara(void)
{
	ModbusSvr_save_para(&mblock1) ;
}


//-------------------------------------------------------------------------------
//	@brief	Power_Init
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void Power_Init(void) 
{
	uint16_t uCRC ;
  	uCRC = CRC16(pow1_frame, 6);
  	pow1_frame[6] = uCRC & 0x00FF;
  	pow1_frame[7] = (uCRC & 0xFF00) >> 8;	
	ptr1 = 0 ;
	tick1_last = GetCurTick();

	uCRC = CRC16(pow2_frame, 6);
  	pow2_frame[6] = uCRC & 0x00FF;
  	pow2_frame[7] = (uCRC & 0xFF00) >> 8;		
	ptr2 = 0 ;
	tick2_last = GetCurTick();
}

//-------------------------------------------------------------------------------
//	@brief	Power_TxCmd
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void Power_TxCmd(int n) 
{
	if ( n == 1 )
	{
		if (pow1_bRecv == 0)
			mblock1.ptrRegs[7]++;

		pow1_bRecv = 0;
		ptr1 =  0 ;

		if (HAL_UART_Transmit(&huart1, pow1_frame, 8, 0xFFFF) != HAL_OK)
		{
			Error_Handler();
		}	
		HAL_UART_Receive_IT(&huart1, &pow1_ch, 1);
	}	
	
	if ( n == 2 )
	{
		if (pow2_bRecv == 0)
			mblock1.ptrRegs[17]++;

		pow2_bRecv = 0;
		ptr2 =  0 ;

		if (HAL_UART_Transmit(&huart3, pow2_frame, 8, 0xFFFF) != HAL_OK)
		{
			Error_Handler();
		}	
		HAL_UART_Receive_IT(&huart3, &pow2_ch, 1);
	}			
}

//-------------------------------------------------------------------------------
//	@brief	Power_Task
//	@param	None
//	@retval	None
//-------------------------------------------------------------------------------
void Power_Task(int n) 
{
	u32 tick;
	int i;

	mblock1.ptrRegs[20] = ptr1;
	mblock1.ptrRegs[40] = ptr2;
	if ( n == 1)
	{
		if ( ptr1 < 17 )
			return;
		
		if ( pow1_buf[0] != 1 || pow1_buf[1] != 0x03 )
			return;

		if ( pow1_buf[2] != 12 )
			return;

		tick = GetCurTick();
		mblock1.ptrRegs[8] = tick - tick1_last;
		tick1_last = tick;

		for (i = 1; i < 6; i++)
			mblock1.ptrRegs[i] = pow1_buf[3 + 2 * i] << 0x08 | pow1_buf[4 + 2 * i];

		mblock1.ptrRegs[6]++;
		pow1_bRecv = 1;
		ptr1 = 0;
	}

	if ( n == 2)
	{
		if ( ptr2 < 17 )
			return;

		if ( pow2_buf[0] != 2 || pow2_buf[1] != 0x03 )
			return;

		if ( pow2_buf[2] != 12 )
			return;

		tick = GetCurTick();
		mblock1.ptrRegs[18] = tick - tick2_last;
		tick2_last = tick;

		for (i = 1; i < 6; i++)
			mblock1.ptrRegs[10 + i] = pow2_buf[3 + 2 * i] << 0x08 | pow2_buf[4 + 2 * i];

		mblock1.ptrRegs[16]++;
		pow2_bRecv = 1;
		ptr2 = 0;
	}	
}

//-----------------end of file---------------------------------------------
