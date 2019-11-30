/**
 * @file uart_linux_interface.h
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#ifndef __UART_LINUX_INTERFACE_H
#define __UART_LINUX_INTERFACE_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#include "uart_comm.h"

typedef enum
{
    UIL_TTYSAC0 = 0,
    UIL_TTYSAC1,
    UIL_TTYSAC2,
    UIL_TTYSAC3,
    UIL_TTYSAC_MAX
}UIL_DevNum;

int UIL_Init(UIL_DevNum num);
int UIL_Send(UIL_DevNum num, uint8* data, uint16 len);
int UIL_Read(UIL_DevNum num, uint8* data, uint16 len);
int UIL_DeInit(UIL_DevNum num);
#endif
