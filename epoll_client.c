#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netdb.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEBUG

#define MAX_EPOLL_EVENTS 500
#define FAILED  -1
#define SUCCESS 0
#define SET_MEM_ZERO(pAddr, iSize) (void*)memset(pAddr, 0, iSize)

//int g_iEpollFd = 0;
int g_piSockFd[MAX_EPOLL_EVENTS];
int g_iServerPort = 0;
char g_pscServerAddr[20];
//struct epoll_event g_stEpollSetEvent, g_pstEpollWaitEvents[MAX_EPOLL_EVENTS];

int SocketCreate(int* const piSockFd)
{
	if (piSockFd == NULL)
	{
		printf("The address of socket fd is NULL!\n");
		return FAILED;
	}
	
	int iSock = socket(AF_INET, SOCK_STREAM, 0);
	if (iSock < 0)
	{
		printf("Create socket fd failed! errorno: %d, error: %s\n", errno, strerror(errno));
		return FAILED;
	}

	*piSockFd = iSock;
	
	return SUCCESS;
}

int SocketSendData(int iSockFd, const char* const pscData, int iDataSize)
{
	if (iSockFd < 0 || iDataSize < 0 || pscData == NULL)
	{
		printf("Socket send data failed! param is error.\n");
		return FAILED;
	}
	
	int iRet = send(iSockFd, pscData, iDataSize, 0);
	if (iRet < 0)
	{
		printf("Send data failed! iRet: %d, errorno(%d), strerror(%s).\n", iRet, errno, strerror(errno));
		return FAILED;
	}
	
	return SUCCESS;
}

int SocketConnectServer(int iSockFd, int iPort, const char* const pscAddr)
{
	if (iSockFd < 0 || iPort < 0 || iPort > 65535 || pscAddr == NULL)
	{
		printf("Socket connect to server failed! param is error.\n");
		return FAILED;
	}
	
	struct sockaddr_in stClientAddr;
	stClientAddr.sin_family = AF_INET;
	stClientAddr.sin_port = htons(iPort);
	stClientAddr.sin_addr.s_addr = inet_addr(pscAddr);

	int iRet = connect(iSockFd, (struct sockaddr*)&stClientAddr, sizeof(stClientAddr));
	if (iRet < 0)
	{
		printf("Client connect to server failed! errorno(%d), strerror(%s).\n", errno, strerror(errno));
		return FAILED;
	}
	
	return SUCCESS;
}

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