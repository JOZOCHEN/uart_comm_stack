/**
 * @file uart_if.c
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#include "uart_if.h"
#include "uart_linux_interface.h"

UartIf_Handler UartIf_Handler_ch1;

void UartIf_init(void)
{
    UartIf_Handler_ch1.hw_handle = UIL_TTYSAC3;

    UIL_Init(UartIf_Handler_ch1.hw_handle);
}

int UartIf_Send(UartIf_Handler* handler, uint8* data)
{
    int ret = E_UART_FAIL;

    if((handler == NULL) || (data == NULL))
    {
        ret = E_UART_FAIL;
    }
    else
    {
        // ret = Queue_Push(handler->queue_handler_tx, data);
        if(UIL_Send(handler->hw_handle, data, 1) == 1u)
        {
            ret = E_UART_OK;
        }
    }
    
    return ret;
}

int UartIf_Recv(UartIf_Handler* handler, uint8* data)
{
    int ret = E_UART_FAIL;

    if((handler == NULL) || (data == NULL) )
    {
        ret = E_UART_FAIL;
    }
    else
    {
        if(UIL_Read(handler->hw_handle, data, 1) == 1)
        {
            ret = E_UART_OK;
        }
    }
    
    return ret;
}

#if 0
int UartIf_SendSync(UartIf_Handler* handler, uint8* data, uint16 len, uint32 tout)
{
    UartIf_HwHandle* hw_handle;
    UART_HandleTypeDef* uart_handle;
    int ret = E_UART_FAIL;

    if( (handler == NULL ) || (data == NULL) )
    {
        ret = E_UART_FAIL;
    }
    else
    {
        hw_handle = &handler->hw_handle;
        uart_handle = hw_handle->uart_handle;
        if(uart_handle->hdmatx != NULL)
        {
            HAL_UART_DMAStop(uart_handle);
        }
        else
        {
            
        }
        HAL_UART_AbortTransmit_IT(uart_handle);

        if(HAL_UART_Transmit(uart_handle, data, len, tout) == HAL_OK)
        {
            ret = E_UART_OK;
        }
        else
        {
            ret = E_UART_FAIL;
        }
    }
    
    return ret;
}

int UartIf_RecvSync(UartIf_Handler* handler, uint8* data, uint16 len, uint32 tout)
{
    UartIf_HwHandle* hw_handle;
    UART_HandleTypeDef* uart_handle;
    int ret = E_UART_FAIL;

    if( (handler == NULL ) || (data == NULL) )
    {
        ret = E_UART_FAIL;
    }
    else
    {
        hw_handle = &handler->hw_handle;
        uart_handle = hw_handle->uart_handle;
        if(uart_handle->hdmatx != NULL)
        {
            HAL_UART_DMAStop(uart_handle);
        }
        else
        {
            
        }
        HAL_UART_AbortReceive_IT(uart_handle);
        
        if(HAL_UART_Receive(uart_handle, data, len, tout) == HAL_OK)
        {
            if(uart_handle->hdmarx != NULL)
            {
                HAL_UART_Receive_DMA(uart_handle, hw_handle->rx_buffer, UARTIF_HW_RX_BUFFER_LEN);               
            }
            else
            {
                HAL_UART_Receive_IT(uart_handle, hw_handle->rx_buffer, UARTIF_HW_RX_BUFFER_LEN);
            }
            ret = E_UART_OK;
        }
        else
        {
            ret = E_UART_FAIL;
        }
        
    }
    
    return ret;
}

void UartIf_Main(UartIf_Handler* handler)
{
    UartIf_HwHandle* hw_handle;
    // Queue_HandleType* rx_queue;
    Queue_HandleType* tx_queue;
    UART_HandleTypeDef* uart_handle;
    uint16 i = 0u;

    if(handler == NULL)
    {

    }
    else
    {
        hw_handle = &handler->hw_handle;
        // rx_queue = handler->queue_handler_rx;
        tx_queue = handler->queue_handler_tx;
        uart_handle = hw_handle->uart_handle;

        if(hw_handle->tx_state == UARTIF_HW_IDLE)
        {
            for(i = 0u; i < UARTIF_HW_TX_BUFFER_LEN; i++)
            {
                if(Queue_Pop(tx_queue, &hw_handle->tx_buffer[i]) == E_QUEUE_FAIL)
                {
                    break;
                }
            }
            if(i != 0u)
            {
                if(uart_handle->hdmatx != NULL)
                {
                    HAL_UART_Transmit_DMA(uart_handle, hw_handle->tx_buffer, i);
                }
                else
                {
                    HAL_UART_Transmit_IT(uart_handle, hw_handle->tx_buffer, i);
                }
                hw_handle->tx_state = UARTIF_HW_RUNNING;
            }
        }
        else
        {
            /*nothing*/
        }

        if(hw_handle->rx_state == UARTIF_HW_IDLE)
        {
            if(uart_handle->hdmarx != NULL)
            {
                HAL_UART_Receive_DMA(uart_handle, hw_handle->rx_buffer, UARTIF_HW_RX_BUFFER_LEN);               
            }
            else
            {
                HAL_UART_Receive_IT(uart_handle, hw_handle->rx_buffer, UARTIF_HW_RX_BUFFER_LEN);
            }
            hw_handle->rx_timeout_cnt = 0u;
            hw_handle->rx_state = UARTIF_HW_RUNNING;
        }
        else if(hw_handle->rx_state == UARTIF_HW_RUNNING)
        {
            /*waiting uart data*/
            if(hw_handle->rx_timeout_cnt > UARTIF_RX_TIMEOUT_MS)
            {
                hw_handle->rx_timeout_cnt = 0u;
                if(uart_handle->hdmarx != NULL)
                {
                    HAL_UART_Receive_DMA(uart_handle, hw_handle->rx_buffer, UARTIF_HW_RX_BUFFER_LEN);               
                }
                else
                {
                    HAL_UART_Receive_IT(uart_handle, hw_handle->rx_buffer, UARTIF_HW_RX_BUFFER_LEN);
                }
            }
            else
            {
                hw_handle->rx_timeout_cnt += UARTIF_MAIN_PERIOD_MS;
            }
        }
        else if(hw_handle->rx_state == UARTIF_HW_CPLT)
        {
            // for(i = 0u ; i < UARTIF_HW_RX_BUFFER_LEN; i++)
            // {
            //     Queue_Push(rx_queue, &hw_handle->rx_buffer[i]);
            // }
            // hw_handle->rx_state = UARTIF_HW_IDLE;
        }
        else
        {
            /*nothing*/
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == NULL)
    {

    }
    else
    {
        if(huart == &huart1)
        {
            UartIf_Handler_ch1.hw_handle.tx_state = UARTIF_HW_IDLE;
        }
        else if(huart == &huart2)
        {
            UartIf_Handler_ch2.hw_handle.tx_state = UARTIF_HW_IDLE;
        }
        else if(huart == &huart3)
        {
            UartIf_Handler_ch3.hw_handle.tx_state = UARTIF_HW_IDLE;
        }
        else if(huart == &huart4)
        {
            UartIf_Handler_ch4.hw_handle.tx_state = UARTIF_HW_IDLE;
        }
        else if(huart == &huart5)
        {
            UartIf_Handler_ch5.hw_handle.tx_state = UARTIF_HW_IDLE;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint16 i = 0u;
    
    if(huart == NULL)
    {

    }
    else
    {
        if(huart == &huart1)
        {
            // UartIf_Handler_ch1.hw_handle.rx_state = UARTIF_HW_CPLT;
            HAL_UART_Receive_DMA(UartIf_Handler_ch1.hw_handle.uart_handle, &UartIf_Handler_ch1.hw_handle.rx_buffer[i], UARTIF_HW_RX_BUFFER_LEN);
            for(i = 0u ; i < UARTIF_HW_RX_BUFFER_LEN; i++)
            {
                Queue_Push(UartIf_Handler_ch1.queue_handler_rx, &UartIf_Handler_ch1.hw_handle.rx_buffer[i]);
            }
            UartIf_Handler_ch1.hw_handle.rx_timeout_cnt = 0u;
        }
        else if(huart == &huart2)
        {
            // UartIf_Handler_ch2.hw_handle.rx_state = UARTIF_HW_CPLT;
            HAL_UART_Receive_DMA(UartIf_Handler_ch2.hw_handle.uart_handle, &UartIf_Handler_ch2.hw_handle.rx_buffer[i], UARTIF_HW_RX_BUFFER_LEN);
            for(i = 0u ; i < UARTIF_HW_RX_BUFFER_LEN; i++)
            {
                Queue_Push(UartIf_Handler_ch2.queue_handler_rx, &UartIf_Handler_ch2.hw_handle.rx_buffer[i]);
            }
            UartIf_Handler_ch2.hw_handle.rx_timeout_cnt = 0u;
        }
        else if(huart == &huart3)
        {
            // UartIf_Handler_ch3.hw_handle.rx_state = UARTIF_HW_CPLT;
            HAL_UART_Receive_DMA(UartIf_Handler_ch3.hw_handle.uart_handle, &UartIf_Handler_ch3.hw_handle.rx_buffer[i], UARTIF_HW_RX_BUFFER_LEN);
            for(i = 0u ; i < UARTIF_HW_RX_BUFFER_LEN; i++)
            {
                Queue_Push(UartIf_Handler_ch3.queue_handler_rx, &UartIf_Handler_ch3.hw_handle.rx_buffer[i]);
            }
            UartIf_Handler_ch3.hw_handle.rx_timeout_cnt = 0u;
        }
        else if(huart == &huart4)
        {
            // UartIf_Handler_ch4.hw_handle.rx_state = UARTIF_HW_CPLT;
            HAL_UART_Receive_DMA(UartIf_Handler_ch4.hw_handle.uart_handle, &UartIf_Handler_ch4.hw_handle.rx_buffer[i], UARTIF_HW_RX_BUFFER_LEN);
            for(i = 0u ; i < UARTIF_HW_RX_BUFFER_LEN; i++)
            {
                Queue_Push(UartIf_Handler_ch4.queue_handler_rx, &UartIf_Handler_ch4.hw_handle.rx_buffer[i]);
            }
            UartIf_Handler_ch4.hw_handle.rx_timeout_cnt = 0u;
        }
        else if(huart == &huart5)
        {
            // UartIf_Handler_ch5.hw_handle.rx_state = UARTIF_HW_CPLT;
            HAL_UART_Receive_IT(UartIf_Handler_ch5.hw_handle.uart_handle, &UartIf_Handler_ch5.hw_handle.rx_buffer[i], UARTIF_HW_RX_BUFFER_LEN);
            for(i = 0u ; i < UARTIF_HW_RX_BUFFER_LEN; i++)
            {
                Queue_Push(UartIf_Handler_ch5.queue_handler_rx, &UartIf_Handler_ch5.hw_handle.rx_buffer[i]);
            }
            UartIf_Handler_ch5.hw_handle.rx_timeout_cnt = 0u;
        }
        else
        {
            /*nothing*/
        }
        
    }
}

/*
void UartIf_test(void)
{
    uint8 i = 0u;
    uint8 test_tx[4] = {0x01, 0x02, 0x03, 0x04};
    uint8 test_rx[4];
    uint16 len;

    UartIf_Send(&UartIf_Handler_ch1, test_tx, 4);

    for( i = 0; i < 4; i ++)
    {
        Queue_Push(UartIf_Handler_ch1.queue_handler_rx, &i);
    }
    UartIf_Recv(&UartIf_Handler_ch1, test_rx, &len);
}
*/
#endif
