#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

int g_piSockFd[MAX_EPOLL_EVENTS];
int g_iServerPort = 0;
char g_pscServerAddr[20];

int ClientInit(void)
{
	SET_MEM_ZERO(&g_piSockFd, sizeof(g_piSockFd));
	
	g_iServerPort = 5000;
	
	SET_MEM_ZERO(g_pscServerAddr, sizeof(g_pscServerAddr));
	
	strcpy(g_pscServerAddr, "127.0.0.1");
	
	return SUCCESS;
}

int main(void)
{
	int iRet = ClientInit();
	if (iRet != SUCCESS)
	{
		printf("Server init failed!\n");
		return FAILED;
	}
	
	printf("Client init success!\n");
	
	int i = 0;
	for (i = 0; i < MAX_EPOLL_EVENTS; i++)
	{
		iRet = SocketCreate(&g_piSockFd[i]);
		if (iRet != SUCCESS)
		{
			printf("Create socket fd failed! i: %d\n", i + 1);
			return FAILED;
		}
		
		printf("Create client sock(%d) success!\n", i + 1);
		
		iRet = SocketConnectServer(g_piSockFd[i], g_iServerPort, g_pscServerAddr);
		if (iRet != SUCCESS)
		{
			printf("Bind socket(%d) failed!\n", i + 1);
			close(g_piSockFd[i]);
			return FAILED;
		}
		
		printf("Client(%d) connect to server success!\n", i + 1);
	}
	
	printf("All socket connected, begin to send data...\n");
	printf("###########################################\n");
	
	char pscDataBuff[100] = {0};
	for(i = 0; i < MAX_EPOLL_EVENTS; i++)
	{
		SET_MEM_ZERO(pscDataBuff, sizeof(pscDataBuff));
		sprintf(pscDataBuff, "This is the client(%d) send data.", i + 1);
		iRet = SocketSendData(g_piSockFd[i], pscDataBuff, strlen(pscDataBuff));
		if (iRet != SUCCESS)
		{
			printf("Socket send data(%d) failed!\n", i + 1);
		}
		close(g_piSockFd[i]);
	}
	
	printf("All socket send data end,...\n");
	printf("###########################################\n");
	
	return SUCCESS;
}