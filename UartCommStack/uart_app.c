/**
 * @file uart_app.c
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#include "uart_app.h"

typedef union 
{
    struct{
        uint8 serv_id :6;
        uint8 status_cmd :2;
    }ctrl_cmd;
    uint8 byte;
}UartApp_CtrlByte;

static int UartApp_RxMsgParse(UartApp_Handler* handler, uint16 len);
static int UartApp_RequestMsgParse(UartApp_Handler* handler, uint16 len);
static int UartApp_InformMsgParse(UartApp_Handler* handler, uint16 len);
static int UartApp_RequestAckMsgParse(UartApp_Handler* handler, uint16 len);
static void UartApp_TxStausInit(UartApp_TxStatus* status);
static void UartApp_RxStausInit(UartApp_RxStatus* status);
static int UartApp_SendMsg(UartApp_Handler* handler, UartApp_CtrlByte ctrl,
                           uint8* para, uint16 len);
static void UartApp_RqstProcess(UartApp_Handler* handler);
static void UartApp_RqstAckProcess(UartApp_Handler* handler);

void UartApp_Main(UartApp_Handler* handler)
{
    uint16 rx_len = 0u;

    if(handler == NULL)
    {

    }
    else
    {
        UartApp_RqstProcess(handler);
        if(UartNw_RecvPacket(handler->nw_handler, handler->rx_buffer, &rx_len) == E_UART_OK)
        {
            UartApp_RxMsgParse(handler, rx_len);     
        }
        UartApp_RqstAckProcess(handler);
    }
}

int UartApp_Init(UartApp_Handler* handler, UartNw_Handler* nw_handler)
{
    int ret = E_UART_FAIL;
    uint16 i = 0u;

    if( (handler == NULL) || (nw_handler == NULL) )
    {

    }
    else
    {
        handler->nw_handler = nw_handler;
        for(i = 0u; i < UARTAPP_BUFFER_LEN; i++)
        {
            handler->tx_buffer[i] = 0u;
            handler->rx_buffer[i] = 0u;
        }
        UartApp_TxStausInit(&handler->tx_status);
        UartApp_RxStausInit(&handler->rx_status);
        ret = E_UART_OK;
    }
    
    return ret;
}

static void UartApp_RxStausInit(UartApp_RxStatus* status)
{
    if(status == NULL)
    {

    }
    else
    {
        status->rx_state = UARTAPP_RX_EMPTY;
        status->tick_ms = 0u;
        status->serv_id = 0u;
    }
}

static void UartApp_TxStausInit(UartApp_TxStatus* status)
{
    if(status == NULL)
    {

    }
    else
    {
        status->tick_ms = 0u;
        status->tx_state = UARTAPP_TX_EMPTY;
        status->serv_id = 0u;
        status->ack_func = NULL;
    }
}

static int UartApp_RequestMsgParse(UartApp_Handler* handler, uint16 len)
{
    int ret = E_UART_FAIL;
    UartApp_RxStatus* rx_status;
    UartApp_CtrlByte ctrl;
    uint32 i = 0u;
    UartApp_RxMsg* rx_msg_list = UARTAPP_INTFC_RX_LIST;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        rx_status = &handler->rx_status;
        ctrl.byte = handler->rx_buffer[0];

        if(rx_status->rx_state != UARTAPP_RX_EMPTY)
        {
            /*send busy*/
            UartApp_SendNegAckMsg(handler, ctrl.ctrl_cmd.serv_id, UARTAPP_E_BUSY);
        }
        else
        {  
            rx_status->rx_state = UARTAPP_RX_INIT;

            for(i = 0u; i < UARTAPP_INTFC_RX_LIST_NUM; i++)
            {
                if(rx_msg_list->serv_id == ctrl.ctrl_cmd.serv_id)
                {
                    if(rx_msg_list->rqst_func != NULL)
                    {
                        rx_status->serv_id = ctrl.ctrl_cmd.serv_id;
                        rx_status->rx_state = UARTAPP_RX_PROCESSING;
                        rx_msg_list->rqst_func(handler, &handler->rx_buffer[1], (len - 1u));
                        if(rx_status->rx_state == UARTAPP_RX_DONE)
                        {
                            UartApp_RxStausInit(rx_status);
                        }
                        else
                        {
                            /* code */
                        }   
                        ret = E_UART_OK;
                        break;
                    }
                    else
                    {
                        /* code */
                    }                

                }
                else
                {
                    rx_msg_list++;
                }
            }

            if(ret != E_UART_OK)
            {
                UartApp_SendNegAckMsg(handler, rx_status->serv_id, UARTAPP_E_SERVICE_NOT_SUPPORT);
                UartApp_RxStausInit(rx_status);
            }
        }
        
    }
    return ret;
}

static int UartApp_InformMsgParse(UartApp_Handler* handler, uint16 len)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl;
    uint32 i = 0u;
    UartApp_RxMsg* rx_msg_list = UARTAPP_INTFC_RX_LIST;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        ctrl.byte = handler->rx_buffer[0];

        for(i = 0u; i < UARTAPP_INTFC_RX_LIST_NUM; i++)
        {
            if(rx_msg_list->serv_id == ctrl.ctrl_cmd.serv_id)
            {
                if(rx_msg_list->rqst_func != NULL)
                {
                    rx_msg_list->rqst_func(handler, &handler->rx_buffer[1], (len - 1u));
                    ret = E_UART_OK;
                    break;
                }
                else
                {
                    /* code */
                }                

            }
            else
            {
                rx_msg_list++;
            }
        }
    }
    return ret;
}

static int UartApp_RxMsgParse(UartApp_Handler* handler, uint16 len)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl_byte;

    if( (handler == NULL) || (len == 0u))
    {
        ret = E_UART_FAIL;
    }
    else
    {
        ctrl_byte.byte = handler->rx_buffer[0];

        if(ctrl_byte.ctrl_cmd.status_cmd == UARTAPP_SC_SEND)
        {
            if(ctrl_byte.ctrl_cmd.serv_id <= SERV_ID_REQUEST_MAX)
            {
                UartApp_RequestMsgParse(handler, len);
            }
            else
            {
                UartApp_InformMsgParse(handler, len);
            }
            
        }
        else
        {
            UartApp_RequestAckMsgParse(handler, len);
        }  

        ret = E_UART_OK;      
    }
    return ret;
}

static void UartApp_RqstProcess(UartApp_Handler* handler)
{
    UartApp_RxStatus* rx_status;
    UartApp_RxMsg* rx_msg_list = UARTAPP_INTFC_RX_LIST;
    uint32 i = 0u;

    if(handler == NULL)
    {

    }
    else
    {
        rx_status = &handler->rx_status;

        if(rx_status->rx_state == UARTAPP_RX_PROCESSING)
        {
            rx_status->tick_ms += UARTAPP_MAIN_PERIOD_MS;
            if(rx_status->tick_ms >= UARTAPP_TIME_PARA_MS_Tb)
            {
                rx_status->tick_ms = 0u;
                UartApp_SendBusyAckMsg(handler, rx_status->serv_id);
            }
            else
            {
                /* code */
            }
            
            for(i = 0u; i < UARTAPP_INTFC_RX_LIST_NUM; i++)
            {
                if(rx_msg_list->serv_id == rx_status->serv_id)
                {
                    if(rx_msg_list->process_func != NULL)
                    {
                        rx_msg_list->process_func(handler);
                        if(rx_status->rx_state == UARTAPP_RX_DONE)
                        {
                            UartApp_RxStausInit(rx_status);
                        }
                        else
                        {
                            /* code */
                        }                       
                        break;
                    }
                    else
                    {
                        /* code */
                    }                

                }
                else
                {
                    rx_msg_list++;
                }
            }
        }
        else
        {
            /* code */
        }
        
    }
}

static int UartApp_SendMsg(UartApp_Handler* handler, UartApp_CtrlByte ctrl,
                           uint8* para, uint16 len)
{
    int ret = E_UART_FAIL;
    uint16 i = 0u;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        handler->tx_buffer[0] = ctrl.byte;
        for(i = 0u; i < len; i++)
        {
            handler->tx_buffer[i + 1] = para[i];
        }
        UartNw_SendPacket(handler->nw_handler, handler->tx_buffer, len + 1u);
        ret = E_UART_OK;
    }
    
    return ret;
}

int UartApp_SendNegAckMsg(UartApp_Handler* handler, uint8 serv_id, UartApp_ErrCode err)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl;
    uint8 err_byte = 0u;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        ctrl.byte = serv_id;
        ctrl.ctrl_cmd.status_cmd = UARTAPP_SC_NEG_ACK;
        err_byte = (uint8)err;
        UartApp_SendMsg(handler, ctrl, &err_byte, sizeof(err_byte));
        ret = E_UART_OK;
    }

    return ret;
}

int UartApp_SendPosAckMsg(UartApp_Handler* handler, uint8 serv_id, uint8* para, uint16 len)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        ctrl.byte = serv_id;
        ctrl.ctrl_cmd.status_cmd = UARTAPP_SC_POS_ACK;
        UartApp_SendMsg(handler, ctrl, para, len);
        ret = E_UART_OK;
    }

    return ret;
}

int UartApp_SendBusyAckMsg(UartApp_Handler* handler, uint8 serv_id)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        ctrl.byte = serv_id;
        ctrl.ctrl_cmd.status_cmd = UARTAPP_SC_BUSY_ACK;
        UartApp_SendMsg(handler, ctrl, NULL, 0u);
        ret = E_UART_OK;
    }

    return ret;
}

int UartApp_StartRqstMsg(UartApp_Handler* handler, uint8 serv_id, uint8* para, uint16 len, 
                        rqst_ack_func func)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl;
    UartApp_TxStatus* tx_status;

    if((handler == NULL) || (func == NULL))
    {
        ret = E_UART_FAIL;
    }
    else
    {
        tx_status = &handler->tx_status;

        if( (tx_status->tx_state == UARTAPP_TX_EMPTY) && (serv_id <= SERV_ID_REQUEST_MAX) )
        {
            tx_status->tx_state = UARTAPP_TX_INIT;
            tx_status->ack_func = func;
            tx_status->serv_id = serv_id;

            ctrl.byte = serv_id;
            ctrl.ctrl_cmd.status_cmd = UARTAPP_SC_SEND;
            UartApp_SendMsg(handler, ctrl, para, len);
            tx_status->tx_state = UARTAPP_TX_WAIT_ACK;
            ret = E_UART_OK;
        }
        else
        {
            /* code */
        }      

    }

    return ret;
}

int UartApp_StartInformMsg(UartApp_Handler* handler, uint8 serv_id, uint8* para, uint16 len)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        if( serv_id > SERV_ID_REQUEST_MAX )
        {
            ctrl.byte = serv_id;
            ctrl.ctrl_cmd.status_cmd = UARTAPP_SC_SEND;
            UartApp_SendMsg(handler, ctrl, para, len);
            ret = E_UART_OK;
        }
        else
        {
            /* code */
        }      

    }

    return ret;
}

static int UartApp_RequestAckMsgParse(UartApp_Handler* handler, uint16 len)
{
    int ret = E_UART_FAIL;
    UartApp_CtrlByte ctrl;
    UartApp_TxStatus* tx_status;

    if(handler == NULL)
    {
        ret = E_UART_FAIL;
    }
    else
    {
        tx_status = &handler->tx_status;
        ctrl.byte = handler->rx_buffer[0];

        if( (tx_status->tx_state == UARTAPP_TX_WAIT_ACK) && (ctrl.ctrl_cmd.serv_id == tx_status->serv_id) )
        {
            if(tx_status->ack_func != NULL)
            {
                tx_status->ack_func((UartApp_StatusCmd)ctrl.ctrl_cmd.status_cmd, &handler->rx_buffer[1], len - 1u);
            }
            else
            {
                /* code */
            }
            
            if(ctrl.ctrl_cmd.status_cmd == UARTAPP_SC_BUSY_ACK)
            {
                tx_status->tick_ms = 0u;
            }
            else
            {
                /*UARTAPP_SC_POS_ACK or UARTAPP_SC_NEG_ACK*/
                UartApp_TxStausInit(tx_status);
            }
            
        }
        else
        {
            /* code */
        }
        

    }
    return ret;    
}

static void UartApp_RqstAckProcess(UartApp_Handler* handler)
{
    UartApp_TxStatus* tx_status;

    if(handler == NULL)
    {
        
    }
    else
    {
        tx_status = &handler->tx_status;

        if(tx_status->tx_state == UARTAPP_TX_WAIT_ACK)
        {
            tx_status->tick_ms += UARTAPP_MAIN_PERIOD_MS;
            if(tx_status->tick_ms >= UARTAPP_TIME_PARA_MS_Ta)
            {
                if(tx_status->ack_func != NULL)
                {
                    tx_status->ack_func(UARTAPP_SC_TIMEOUT, NULL, 0u);
                }
                else
                {
                    /* code */
                }
                
                UartApp_TxStausInit(tx_status);
            }
        }
        else
        {
            /* code */
        }
        
    }
}
