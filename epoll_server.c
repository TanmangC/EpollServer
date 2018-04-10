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

	iRet = EpollCreate(&g_iEpollFd);
	if (iRet != SUCCESS)
	{
		printf("Create epoll fd failed!\n");
		close(g_iEpollFd);
		close(g_iSockFd);
		return FAILED;
	}
	
	printf("Create epoll fd success, g_iEpollFd: %d, g_iSockFd: %d\n", g_iEpollFd, g_iSockFd);

	iRet = EpollControl(g_iSockFd, g_iEpollFd, &g_stEpollSetEvent);
	if (iRet != SUCCESS)
	{
		printf("Epoll ctl failed!\n");
		close(g_iEpollFd);
		close(g_iSockFd);
		return FAILED;
	}
	
	printf("Epoll ctl success!\n");
	printf("Server init success, now working...\n");
	printf("###################################\n");

	iRet = EpollCycle(g_iEpollFd, g_iSockFd, g_pstEpollWaitEvents, MAX_EPOLL_EVENTS);
	if (iRet != SUCCESS)
	{
		printf("Epoll ctl failed!\n");
		close(g_iEpollFd);
		close(g_iSockFd);
		return FAILED;
	}
	
	printf("###################################\n");
	printf("Server stop success.\n");
	return SUCCESS;
}