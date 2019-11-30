/**
 * @file uart_linux_interface.c
 * @author jozochen (jozocyz@hotmail.com)
 * @brief 
 * @date 2019-11-30
 * @copyright Apache License 2.0
 *            jozochen (jozocyz@hotmail.com) Copyright (c) 2019
 */
#include "uart_linux_interface.h"
#include "uart_comm.h"
#include "debug.h"

#define UART_LINUX_TTYSAC0 "/dev/ttySAC0"
#define UART_LINUX_TTYSAC1 "/dev/ttySAC1"
#define UART_LINUX_TTYSAC2 "/dev/ttySAC2"
#define UART_LINUX_TTYSAC3 "/dev/ttySAC3"

#define UIL_ERR(args,...) DBG_ERR(args, ##__VA_ARGS__)
// #define UIL_ERR(args,...)

typedef struct 
{
    int32 baud_rate;
    char* dev_addr;
}UIL_Config;

typedef struct
{
    const UIL_Config config;

    int32 uart_fd;
}ULI_Status;

ULI_Status uil_status[UIL_TTYSAC_MAX] = {
    /*UIL_TTYSAC0*/
    {
        .config = {
            .baud_rate = 115200,
            .dev_addr = UART_LINUX_TTYSAC0,
        },
        .uart_fd = 0,
    },
    /*UIL_TTYSAC1*/
    {
        .config = {
            .baud_rate = 115200,
            .dev_addr = UART_LINUX_TTYSAC1,
        },
        .uart_fd = 0,
    },
    /*UIL_TTYSAC2*/
    {
        .config = {
            .baud_rate = 115200,
            .dev_addr = UART_LINUX_TTYSAC2,
        },
        .uart_fd = 0,
    },
    /*UIL_TTYSAC3*/
    {
        .config = {
            .baud_rate = 115200,
            .dev_addr = UART_LINUX_TTYSAC3,
        },
        .uart_fd = 0,
    },
};

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) { 
		// perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
	{
        case 7:
            newtio.c_cflag |= CS7;
		break;
        case 8:
            newtio.c_cflag |= CS8;
		break;
	}

	switch( nEvent )
	{
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
		break;
        case 'E': 
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
		break;
        case 'N':  
            newtio.c_cflag &= ~PARENB;
		break;
	}

	// printf("Baund Rate: %d\n", nSpeed);

	switch( nSpeed )
	{
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
		break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
		break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
		break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
		break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
		break;
        case 921600:
            // printf("Rate:921600\n");
            cfsetispeed(&newtio, B921600);
            cfsetospeed(&newtio, B921600);
        break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
		break;
	}
	if( nStop == 1 )
	{
		newtio.c_cflag &=  ~CSTOPB;
	}
	else if ( nStop == 2 )
	{
		newtio.c_cflag |=  CSTOPB;
	}
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		// perror("com set error");
		return -1;
	}
//	printf("set done!\n\r");
	return 0;
}

int UIL_Init(UIL_DevNum num)
{
    int32 ret;

    if(num >= UIL_TTYSAC_MAX)
    {
        UIL_ERR("num fault\n");
        return E_UART_FAIL;
    }

    ULI_Status* pstatus = &uil_status[num];
    const UIL_Config* pconfig = &pstatus->config;

    pstatus->uart_fd = open(pconfig->dev_addr, O_RDWR);
	if (pstatus->uart_fd == -1)
	{
		UIL_ERR("Open faild\n");
		return E_UART_FAIL;
	}
	
	fcntl(pstatus->uart_fd, F_SETFL, FNDELAY);

	ret = set_opt(pstatus->uart_fd, pconfig->baud_rate, 8, 'N', 1);
	if (ret == -1)
	{
		UIL_ERR("Set uart faild\n");
		return E_UART_FAIL;
	}

    return E_UART_OK;
}


int UIL_Send(UIL_DevNum num, uint8* data, uint16 len)
{
    int ret;

    if(num >= UIL_TTYSAC_MAX)
    {
        UIL_ERR("num fault\n");
        return E_UART_FAIL;
    }
    ULI_Status* pstatus = &uil_status[num];

    ret = write(pstatus->uart_fd, data, len);

	if(ret <= 0)
	{
		UIL_ERR("send fail\n");
	}
	else
	{
		// UIL_ERR("send %d\n", ret);
	}
	

	return ret;
}

int UIL_Read(UIL_DevNum num, uint8* data, uint16 len)
{
    int ret;

    if(num >= UIL_TTYSAC_MAX)
    {
        UIL_ERR("num fault\n");
        return E_UART_FAIL;
    }
    ULI_Status* pstatus = &uil_status[num];

    ret = read(pstatus->uart_fd, data, len);

	return ret;
}

int UIL_DeInit(UIL_DevNum num)
{
    if(num >= UIL_TTYSAC_MAX)
    {
        UIL_ERR("num fault\n");
        return E_UART_FAIL;
    }

    ULI_Status* pstatus = &uil_status[num];

    if(pstatus->uart_fd == 0)
    {
        UIL_ERR("close fail\n");
        return E_UART_FAIL;
    }
    else
    {
        close(pstatus->uart_fd);
    }
    
    return E_UART_OK;
}
