/**
 * @file uart_app.h
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#ifndef __UART_APP_H
#define __UART_APP_H
#include "uart_comm.h"
#include "uart_nw.h"

/***************************************APP TIME PARA**********************************************/
#define UARTAPP_MAIN_PERIOD_MS (10u)
#define UARTAPP_TIME_PARA_MS_Ta (1000u)
#define UARTAPP_TIME_PARA_MS_Tb (500u)
/*************************************************************************************************/
/***************************************APP RX PARA***********************************************/

/*************************************************************************************************/
#define UARTAPP_BUFFER_LEN (200u)

typedef enum
{
    UARTAPP_E_INVALID_PARA = 0x01,
    UARTAPP_E_INCORRECT_STATE = 0x02,
    UARTAPP_E_SERVICE_NOT_SUPPORT = 0X03,
    UARTAPP_E_BUSY = 0xff
}UartApp_ErrCode;

typedef enum
{
    UARTAPP_RX_EMPTY,
    UARTAPP_RX_INIT,
    UARTAPP_RX_PROCESSING,
    UARTAPP_RX_DONE
}UartApp_Rxtate;

typedef struct 
{
    uint32 tick_ms;

    uint8 serv_id;
    UartApp_Rxtate rx_state;
}UartApp_RxStatus;

typedef enum
{
    UARTAPP_TX_EMPTY,
    UARTAPP_TX_INIT,
    UARTAPP_TX_WAIT_ACK,
    UARTAPP_TX_DONE
}UartApp_Txtate;

typedef enum{
    UARTAPP_SC_SEND = 0x00,
    UARTAPP_SC_POS_ACK = 0x01,
    UARTAPP_SC_NEG_ACK = 0x02,
    UARTAPP_SC_BUSY_ACK = 0x03,
    UARTAPP_SC_TIMEOUT = 0x04
}UartApp_StatusCmd;

typedef void (*rqst_ack_func)(UartApp_StatusCmd status, uint8* para, uint16 len);

typedef struct 
{
    uint32 tick_ms;
    uint8 serv_id;
    UartApp_Txtate tx_state;
    rqst_ack_func ack_func;
}UartApp_TxStatus;

typedef struct 
{
    UartNw_Handler* nw_handler;

    uint8 tx_buffer[UARTAPP_BUFFER_LEN];
    uint8 rx_buffer[UARTAPP_BUFFER_LEN];
    UartApp_RxStatus rx_status;
    UartApp_TxStatus tx_status;
}UartApp_Handler;

/***************************************APP INTERFACE DEFINITION**********************************/
#define SERV_ID_REQUEST_MAX (0x20)

typedef void (*rqst_func)(UartApp_Handler* handler, uint8* para, uint16 len);
typedef void (*process_func)(UartApp_Handler* handler);

typedef struct 
{
    const uint8 serv_id;
    const rqst_func rqst_func;
    const process_func process_func;
}UartApp_RxMsg;
#define UARTAPP_INTFC_RX_LIST_NUM (3)
extern UartApp_RxMsg UartAppIntfc_rx_msg_list[UARTAPP_INTFC_RX_LIST_NUM];
#define UARTAPP_INTFC_RX_LIST UartAppIntfc_rx_msg_list
/*************************************************************************************************/
int UartApp_SendNegAckMsg(UartApp_Handler* handler, uint8 serv_id, UartApp_ErrCode err);
int UartApp_SendPosAckMsg(UartApp_Handler* handler, uint8 serv_id, uint8* para, uint16 len);
int UartApp_SendBusyAckMsg(UartApp_Handler* handler, uint8 serv_id);

void UartApp_Main(UartApp_Handler* handler);
int UartApp_Init(UartApp_Handler* handler, UartNw_Handler* nw_handler);
int UartApp_StartRqstMsg(UartApp_Handler* handler, uint8 serv_id, uint8* para, uint16 len, 
                        rqst_ack_func func);
int UartApp_StartInformMsg(UartApp_Handler* handler, uint8 serv_id, uint8* para, uint16 len);
#endif
