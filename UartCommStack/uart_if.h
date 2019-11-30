/**
 * @file uart_if.h
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#ifndef __UART_IF_H
#define __UART_IF_H
#include "uart_comm.h"
#include "uart_linux_interface.h"

/***************************************IF TIME PARA**********************************************/
#define UARTIF_MAIN_PERIOD_MS (1u)
#define UARTIF_RX_TIMEOUT_MS (5u)
/*************************************************************************************************/

typedef struct 
{
    UIL_DevNum hw_handle;
}UartIf_Handler;

extern UartIf_Handler UartIf_Handler_ch1;

void UartIf_init(void);
int UartIf_Send(UartIf_Handler* handler, uint8* data);
int UartIf_Recv(UartIf_Handler* handler, uint8* data);
int UartIf_SendSync(UartIf_Handler* handler, uint8* data, uint16 len, uint32 tout);
int UartIf_RecvSync(UartIf_Handler* handler, uint8* data, uint16 len, uint32 tout);
void UartIf_Main(UartIf_Handler* handler);
#endif
