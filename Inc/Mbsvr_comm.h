/***************************************************
 * Copyright (C),2019 www.idsse.ac.cn
 * Written by chenming
 * Version 1.0
 * Data  2019-3-3
 * Description: modbus服务程序公用库头文件
 * *************************************************/
#ifndef __MBSVR_H__
#define __MBSVR_H__

#include "main.h"

#define MB_STATION 90
#define MB_BAUDRATE 91
#define MB_BOOTNUM 92
#define MB_COILSTARTADR 93
#define MB_COILLEN 94
#define MB_REGSTARTADR 95
#define MB_REGLEN 96
#define MB_INPUTSTARTADR 97
#define MB_INPUTLEN 98
#define MB_TICK 99

#define BIT2BYTE(n) ((((n)&0x0007) == 0) ? ((n) >> 3) : (((n) >> 3) + 1))
#define GETBIT_BYTE(n, bit) (((n) >> (bit)) & 0x01)

typedef struct tag_ModbusModule
{
    int baudrate;  //通信波特率
    short station; //本modbus口站地址

    int uCoilStartAdr; //线圈起始地址
    int uCoilLen;      //线圈数目
    int uCoilEndAdr;   //线圈结束地址
    short *ptrCoils;   //保持线圈的地方

    int uRomStartAdr; //只读存储器起始地址
    int uRomLen;      //只读寄存器长度
    int uRomEndAdr;   //只读存储器接收地址
    short *ptrRoms;   //只读寄存器的地方

    int uRegStartAdr; //保持寄存器起始地址
    int uRegLen;      //保持寄存器长度
    int uRegEndAdr;   //保持寄存器结束地址
    short *ptrRegs;   //保持寄存器的地方

    u8 buffer[512]; //缓冲区
    u8 *tsk_buf;    //处理程序缓冲
    u8 *isr_buf;    //中断程序缓冲

    u8 pos_msg;           //接受指针
    u8 frame_len;         //命令帧长度
    u8 trans_len;         //响应帧长度
    u8 bFrameStart;       //开始接受响应标志
    u8 uFrameInterval;    //帧间隙
    u8 errno;             //当前错误代号
    __IO u16 nMBInterval; //接受字符间隙计数器
    u8 bSaved;            //寄存器改动保存标志
    u32 uLTick;           //上一次接收成功的tick值

    UART_HandleTypeDef *pUsart;
} Modbus_block;

//----------------------------------------------------------------------------------
void ModbusSvr_block_init(Modbus_block *pblk, UART_HandleTypeDef* pUsart); //初始化
void ModbusSvr_task(Modbus_block *pblk);
u8 ModbusSvr_procotol_chain(Modbus_block *pblk);
void ModbusSvr_save_para(Modbus_block *pblk);
void ModbusSvr_NVIC_Configuration(u8 nChn);
void Modbus_UsartHandler(Modbus_block *pblk);


u16 CRC16(const uint8_t *nData, uint8_t wLength);
void Usart_SendByte(UART_HandleTypeDef *pUSARTx, uint8_t ch);
void Usart_SendBytes(UART_HandleTypeDef *pUSARTx, uint8_t *ptr, int n);
void Usart_SendString(UART_HandleTypeDef *pUSARTx, char *str);
void Usart_SendHalfWord(UART_HandleTypeDef *pUSARTx, uint16_t ch);

#endif
/*-------------------------end of file------------------------------------------*/
