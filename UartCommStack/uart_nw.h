/**
 * @file uart_nw.h
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#ifndef __UART_NW_H
#define __UART_NW_H
#include "uart_comm.h"
#include "uart_if.h"

/***************************************NW TIME PARA**********************************************/
#define UARTNW_MAIN_PERIOD_MS (1u)
#define UARTNW_TIME_PARA_MS_Tn (5U)
/*************************************************************************************************/
/***************************************NW FSM PARA**********************************************/
#define UARTNW_SYNC_WORD (0xFE)
#define UARTNW_SYNC_WORD_NUM (2u)
#define UARTNW_LEN_NUM (sizeof(uint16))
#define UARTNW_CHECKSUM_NUM (1u)
/*************************************************************************************************/

#define UARTNW_BUFFER_LEN (200u)
#define UARTNW_RX_BUFFER_LEN UARTNW_BUFFER_LEN
#define UARTNW_TX_BUFFER_LEN (UARTNW_BUFFER_LEN + UARTNW_SYNC_WORD_NUM + UARTNW_LEN_NUM + UARTNW_CHECKSUM_NUM)

typedef enum
{
    UARTNW_RX_EXPCT_SYNC_WORD,
    UARTNW_RX_EXPCT_LEN,
    UARTNW_RX_EXPCT_DATA,
    UARTNW_RX_EXPCT_CHECKSUM,
    UARTNW_RX_FINISH
}UartNw_RxState;

typedef struct 
{
    uint32 tick_ms;
    uint8 fsm_cnt;
    uint16 expect_len;
    uint16 rx_len;
    UartNw_RxState rx_state;
    uint8 checksum;
}UartNw_RxFsmStatus;

typedef struct 
{
    UartIf_Handler* if_handler;

    uint8 tx_buffer[UARTNW_TX_BUFFER_LEN];
    uint8 rx_buffer[UARTNW_RX_BUFFER_LEN];
    UartNw_RxFsmStatus rx_fsm_status;
}UartNw_Handler;

void UartNw_Init(UartNw_Handler* handler, UartIf_Handler* if_handler);
void UartNw_Main(UartNw_Handler* handler);
int UartNw_SendPacket(UartNw_Handler* handler, uint8* data, uint16 len);
int UartNw_RecvPacket(UartNw_Handler* handler, uint8* data, uint16* len);
#endif
