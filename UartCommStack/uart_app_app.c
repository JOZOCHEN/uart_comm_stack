/**
 * @file uart_app_app.c
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#include "stdlib.h"
#include "uart_app.h"
#include "uart_if.h"
#include "heartbeat_manage_app.h"

UartNw_Handler uartnw_ch1;

UartApp_Handler uartapp_ch1;

static uint8 app_buffer[100];

#define SET_RQST_PROCESS_DONE(handler) (handler->rx_status.rx_state = UARTAPP_RX_DONE)
void UartAppApp_HeartbeatInform(UartApp_Handler* handler, uint8* para, uint16 len);
void UartAppApp_VersionCtrlRqst(UartApp_Handler* handler, uint8* para, uint16 len);
void UartAppApp_DstBrdCtrlRqst(UartApp_Handler* handler, uint8* para, uint16 len);

UartApp_RxMsg UartAppIntfc_rx_msg_list[UARTAPP_INTFC_RX_LIST_NUM] =
{
    {
        .serv_id = 0x00,
        .rqst_func = UartAppApp_VersionCtrlRqst,
        .process_func = NULL
    },
    {
        .serv_id = 0x1C,
        .rqst_func = UartAppApp_DstBrdCtrlRqst,
        .process_func = NULL
    },
    {
        .serv_id = 0x3B,
        .rqst_func = UartAppApp_HeartbeatInform,
        .process_func = NULL
    }
};


void UartAppApp_HeartbeatInform(UartApp_Handler* handler, uint8* para, uint16 len)
{
    if(handler == &uartapp_ch1)
    {
        HB_RxIrqHandler(&hb_chs[HB_CHANNEL_1], para, len);
    }
}

void UartAppApp_VersionCtrlRqst(UartApp_Handler* handler, uint8* para, uint16 len)
{
    uint8 i = 0u;

    app_buffer[i++] = 0;
    app_buffer[i++] = 1;
    app_buffer[i++] = 2;  
    app_buffer[i++] = 0;
    app_buffer[i++] = 1;
    app_buffer[i++] = 2;  
    UartApp_SendPosAckMsg(handler, 0x00, app_buffer, i);
    SET_RQST_PROCESS_DONE(handler);
}

void UartAppApp_DstBrdCtrlRqst(UartApp_Handler* handler, uint8* para, uint16 len)
{
    #define DST_BRD_CTRL_CMD_SLEEP 0u
    #define DST_BRD_CTRL_CMD_CAN_STOP 1u
    #define DST_BRD_CTRL_CMD_CAN_START 2u
    uint8 cmd = 0u;

    if( (handler == &uartapp_ch1) && (len == 1u))
    {
        cmd = para[0];
        switch(cmd)
        {
            case DST_BRD_CTRL_CMD_SLEEP:
                UartApp_SendPosAckMsg(handler, 0x1C, &cmd, 1u);
                system("bash shutdown");
                SET_RQST_PROCESS_DONE(handler);
            break;

            case DST_BRD_CTRL_CMD_CAN_STOP:
                UartApp_SendPosAckMsg(handler, 0x1C, &cmd, 1u);
                SET_RQST_PROCESS_DONE(handler);
            break;

            case DST_BRD_CTRL_CMD_CAN_START:
                UartApp_SendPosAckMsg(handler, 0x1C, &cmd, 1u);
                SET_RQST_PROCESS_DONE(handler);
            break;

            default:
                UartApp_SendNegAckMsg(handler, 0x1C, UARTAPP_E_INVALID_PARA);
                SET_RQST_PROCESS_DONE(handler);
            break;
        }
    }
}

void UartAppApp_Init(void)
{
    UartIf_init();

    UartNw_Init(&uartnw_ch1, &UartIf_Handler_ch1);

    UartApp_Init(&uartapp_ch1, &uartnw_ch1);
}

void UartAppApp_IfMain(void)
{
    // UartIf_Main(&UartIf_Handler_ch1);
    // UartIf_Main(&UartIf_Handler_ch2);
    // UartIf_Main(&UartIf_Handler_ch3);
    // UartIf_Main(&UartIf_Handler_ch4);
    // UartIf_Main(&UartIf_Handler_ch5);
}

void UartAppApp_NwMain(void)
{
    UartNw_Main(&uartnw_ch1);
}

void UartAppApp_AppMain(void)
{
    UartApp_Main(&uartapp_ch1);
}


