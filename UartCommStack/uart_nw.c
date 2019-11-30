/**
 * @file uart_nw.c
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#include "uart_nw.h"

static void UartNw_RxFsmInit(UartNw_RxFsmStatus* rx_fsm_status);
static void UartNw_RxFsm(UartNw_Handler* handler, uint8 data);
static uint8 UartNw_MakeChecksum(uint8* data, uint16 len);

void UartNw_Init(UartNw_Handler* handler, UartIf_Handler* if_handler)
{
    uint8 i =0u;

    if((handler == NULL) || (if_handler == NULL))
    {
        /*nothing*/
    }
    else
    {
        handler->if_handler = if_handler;
        for( i = 0u; i < UARTNW_TX_BUFFER_LEN; i++)
        {
            handler->tx_buffer[i] = 0u;
        }
        for( i = 0u; i < UARTNW_RX_BUFFER_LEN; i++)
        {
            handler->rx_buffer[i] = 0u;
        }
        UartNw_RxFsmInit(&handler->rx_fsm_status);
    }
    
}

void UartNw_Main(UartNw_Handler* handler)
{
    uint16 retry_cnt = UARTNW_TX_BUFFER_LEN;
    UartNw_RxFsmStatus* rx_fsm_status;
    uint8 temp_data = 0u;

    if(handler == NULL)
    {

    }
    else
    {
        rx_fsm_status = &handler->rx_fsm_status;

        if(rx_fsm_status->rx_state != UARTNW_RX_FINISH)
        {
            rx_fsm_status->tick_ms += UARTNW_MAIN_PERIOD_MS;
            if(rx_fsm_status->tick_ms >= UARTNW_TIME_PARA_MS_Tn)
            {
                UartNw_RxFsmInit(rx_fsm_status);
            }

            while( (UartIf_Recv(handler->if_handler, &temp_data) == E_UART_OK) && (retry_cnt--) )
            {
                UartNw_RxFsm(handler, temp_data);
            }
        }
        else
        {
            
        }
    }
}

int UartNw_SendPacket(UartNw_Handler* handler, uint8* data, uint16 len)
{
    int ret = E_UART_FAIL;
    uint8* tx_buffer;
    uint16 i = 0u;
    uint16 tx_len = 0u;

    if((handler == NULL) || (data == NULL) || (len == 0) || (len > UARTNW_BUFFER_LEN))
    {
        ret = E_UART_FAIL;
    }
    else
    {
        tx_buffer = handler->tx_buffer;
        for(i = 0u; i < UARTNW_SYNC_WORD_NUM; i++)
        {
            tx_buffer[tx_len++] = UARTNW_SYNC_WORD;
        }
        tx_buffer[tx_len++] = (uint8)(len & 0x00ff);
        tx_buffer[tx_len++] = (uint8)((len & 0xff00) >> 8);
        for(i = 0; i < len; i++)
        {
            tx_buffer[tx_len++] = data[i];
        }
        tx_buffer[tx_len++] = UartNw_MakeChecksum(data, len);
        for( i = 0u; i < tx_len; i++)
        {
            UartIf_Send(handler->if_handler, &tx_buffer[i]);
        }
    }
    
    return ret;
}

int UartNw_RecvPacket(UartNw_Handler* handler, uint8* data, uint16* len)
{
    int ret = E_UART_FAIL;
    UartNw_RxFsmStatus*  rx_fsm_status;
    uint16 i = 0u;

    if((handler == NULL) || (data == NULL) || (len == NULL) )
    {
        ret = E_UART_FAIL;
    }
    else
    {
        rx_fsm_status = &handler->rx_fsm_status;

        if(rx_fsm_status->rx_state == UARTNW_RX_FINISH)
        {
            for( i = 0u; i < rx_fsm_status->rx_len; i++)
            {
                data[i] = handler->rx_buffer[i];
            }
            *len = rx_fsm_status->rx_len;
            UartNw_RxFsmInit(rx_fsm_status);

            ret = E_UART_OK;
        }
        else
        {
            /* code */
        }       
    }

    return ret;
}

static void UartNw_RxFsmInit(UartNw_RxFsmStatus* rx_fsm_status)
{
    if(rx_fsm_status == NULL)
    {

    }
    else
    {
        rx_fsm_status->tick_ms = 0u;
        rx_fsm_status->fsm_cnt = 0u;
        rx_fsm_status->expect_len = 0u;
        rx_fsm_status->rx_len = 0u;
        rx_fsm_status->rx_state = UARTNW_RX_EXPCT_SYNC_WORD;
    }
    
}

static void UartNw_RxFsm(UartNw_Handler* handler, uint8 data)
{
    UartNw_RxFsmStatus* rx_fsm_status;
    uint8* rx_buffer;
    uint8 result = TRUE;
    uint8 calc_checksum = 0u;
    
    if(handler == NULL)
    {
        /*nothing*/
    }
    else
    {
        rx_fsm_status = &handler->rx_fsm_status;
        rx_buffer = handler->rx_buffer;

        switch(rx_fsm_status->rx_state)
        {
            case UARTNW_RX_EXPCT_SYNC_WORD:
                if(data == UARTNW_SYNC_WORD)
                {
                    rx_fsm_status->fsm_cnt++;
                    if(rx_fsm_status->fsm_cnt == UARTNW_SYNC_WORD_NUM)
                    {
                        rx_fsm_status->fsm_cnt = 0u;
                        rx_fsm_status->rx_state = UARTNW_RX_EXPCT_LEN;
                    }
                }
                else
                {
                    result = FALSE;
                }
                
            break;

            case UARTNW_RX_EXPCT_LEN:
                rx_buffer[rx_fsm_status->fsm_cnt] = data;
                rx_fsm_status->fsm_cnt++;
                if(rx_fsm_status->fsm_cnt == UARTNW_LEN_NUM)
                {
                    rx_fsm_status->expect_len = ((uint16)rx_buffer[1] << 8) +  (uint16)rx_buffer[0];
                    if( (rx_fsm_status->expect_len <= UARTNW_BUFFER_LEN) && (rx_fsm_status->expect_len != 0u))
                    {
                        rx_fsm_status->rx_state = UARTNW_RX_EXPCT_DATA;
                    }
                    else
                    {
                        result = FALSE;
                    }
                    
                }
            break;

            case UARTNW_RX_EXPCT_DATA:
                rx_buffer[rx_fsm_status->rx_len] = data;
                rx_fsm_status->rx_len++;
                if(rx_fsm_status->rx_len == rx_fsm_status->expect_len)
                {
                    rx_fsm_status->rx_state = UARTNW_RX_EXPCT_CHECKSUM;
                }
            break;

            case UARTNW_RX_EXPCT_CHECKSUM:
                rx_fsm_status->checksum = data;
                calc_checksum = UartNw_MakeChecksum(rx_buffer, rx_fsm_status->rx_len);
                if(rx_fsm_status->checksum == calc_checksum)
                {
                    rx_fsm_status->rx_state = UARTNW_RX_FINISH;
                }
                else
                {
                    result = FALSE;
                }
                
            break;

            case UARTNW_RX_FINISH:
                /*wait UartNw_RecvPacket*/
            break;

            default:
                result = FALSE;
            break;
        }

        if(result == FALSE)
        {
            UartNw_RxFsmInit(rx_fsm_status);
        }
        else
        {
            rx_fsm_status->tick_ms = 0u;
        }
        
    }
}

static uint8 UartNw_MakeChecksum(uint8* data, uint16 len)
{
    uint16 i = 0u;
    uint8 ret = 0u;

    for(i = 0u; i < len; i++)
    {
        ret += data[i];
    }

    return ret;
}

