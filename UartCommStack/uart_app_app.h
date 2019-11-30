/**
 * @file uart_app_app.h
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#ifndef __UART_APP_APP
#define __UART_APP_APP
#include "uart_comm.h"
#include "uart_nw.h"
#include "uart_app.h"

extern UartNw_Handler uartnw_ch1;
extern UartApp_Handler uartapp_ch1;

void UartAppApp_Init(void);
void UartAppApp_IfMain(void);
void UartAppApp_NwMain(void);
void UartAppApp_AppMain(void);

#endif
