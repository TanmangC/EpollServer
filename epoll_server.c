#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "common.h"

int g_iEpollFd = 0;
int g_iSockFd = 0;
struct epoll_event g_stEpollSetEvent, g_pstEpollWaitEvents[MAX_EPOLL_EVENTS];

int SetServerSockMaxfd()
{
	struct rlimit stLimit;
	stLimit.rlim_cur = 1000000; //RLIM_INFINITY;
	stLimit.rlim_max = 1000000; //RLIM_INFINITY;
	
	(void)system("ulimit -n 1020000");
	
	int iRet = setrlimit(RLIMIT_NOFILE, &stLimit);
	if (iRet < 0)
	{
		printf("Set max sockfd infinit failed! iRet: %d, errorno: %d, error: %s\n", iRet, errno, strerror(errno));
		printf("Before start server, 'ulimit -n 1000000'");
		return FAILED;
	}
	return SUCCESS;
}

int ServerInit(void)
{
	g_iEpollFd = 0;
	g_iSockFd = 0;
	SET_MEM_ZERO(&g_stEpollSetEvent, sizeof(g_stEpollSetEvent));
	SET_MEM_ZERO(g_pstEpollWaitEvents, sizeof(g_pstEpollWaitEvents));
	
	int iRet = SetServerSockMaxfd();
	if (iRet != SUCCESS)
	{
		printf("Set max sockfd failed!\n");
		return FAILED;
	}
	
	return SUCCESS;
}

int main(void)
{
	int iRet = ServerInit();
	if (iRet != SUCCESS)
	{
		printf("Server init failed!\n");
		return FAILED;
	}
	
	printf("Server init success!\n");

	iRet = SocketCreate(&g_iSockFd);
	if (iRet != SUCCESS)
	{
		printf("Create socket fd failed!\n");
		return FAILED;
	}

	printf("Create socket fd success!\n");

	iRet = SocketServerBindAndListen(g_iSockFd, 5000);
	if (iRet != SUCCESS)
	{
		printf("Bind socket failed!\n");
		close(g_iSockFd);
		return FAILED;
	}

	printf("Bind socket success!\n");
	
	iRet = SocketSetOption(g_iSockFd, O_NONBLOCK);
	if (iRet != SUCCESS)
	{
		printf("Set socket noblock failed!\n");
		close(g_iSockFd);
		return FAILED;
	}
	
	printf("Set socket noblock success!\n");

	iRet = InitConnectionPool(1000);
	if (iRet < 0)
	{
		printf("Init user connection failed.\n");
		return 0;
	}
	printf("Init connection success.\n");

	printf("Server init success, now working...\n");
	printf("###################################\n");
	int iEpollFd = InitEpoll(1024, g_iSockFd);
	if (iEpollFd < 0)
	{
		printf("Create epoll failed.\n");
		return -1;
	}
	printf("Create epoll success, iEpollFd: %d.\n", iEpollFd);
	HandleServerWork(g_iSockFd, iEpollFd, 1024);

	printf("Server work end.\n");
	DestoryConnection();
	close(g_iSockFd);
	close(iEpollFd);

	printf("###################################\n");
	printf("Server stop success.\n");
	return SUCCESS;
}
