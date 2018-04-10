#include "common.h"

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

int SocketServerBindAndListen(int iSockFd, int iPort)
{
	struct sockaddr_in stSockAddr;
	
	if (iSockFd < 0 || iPort < 0 || iPort > 65535)
	{
		printf("Bind socket fd(%d) failed! param is error.\n", iSockFd);
		return FAILED;
	}
	
	SET_MEM_ZERO(&stSockAddr, sizeof(struct sockaddr_in)); 
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(iPort);
	stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int iRet = bind(iSockFd, (struct sockaddr*)&stSockAddr, sizeof(stSockAddr));
	if (iRet < 0)
	{
		printf("Bind socket fd(%d) failed! errorno: %d, error: %s\n", iSockFd, errno, strerror(errno));
		return FAILED;
	}
	
	iRet = listen(iSockFd, 20);
	if (iRet < 0)
	{
		printf("Listen socket fd(%d) failed! errorno: %d, error: %s\n", iSockFd, errno, strerror(errno));
		return FAILED;
	}
	
	return SUCCESS;
}

int SocketSetOption(int iSockFd, int iOption)
{
	if (iSockFd < 0)
	{
		printf("Set socket option failed! param is error.\n");
		return FAILED;
	}
	
	int iOldOpt = fcntl(iSockFd, F_GETFL);
	if (iOldOpt < 0)
	{
		printf("Get old socket option failed! errorno(%d), strerror(%s).\n", errno, strerror(errno));
		return FAILED;
	}
	
	int iNewOpt = iOldOpt | iOption;
	int iRet = fcntl(iSockFd, F_SETFL, iNewOpt); //fcntl(st, F_SETFL, opts)
	if (iRet < 0)
	{
		printf("Set socket option(%x) failed! errorno(%d), strerror(%s).\n", iNewOpt, errno, strerror(errno));
		return FAILED;
	}
	return SUCCESS;
}

int SocketAcceptClient(int iSockFd, int* const piClienSockFd)
{
	if (iSockFd < 0 || piClienSockFd == NULL)
	{
		printf("Accept client socket failed! param error.\n");
		return FAILED;
	}
	
	struct sockaddr_in stClientAddr;
	
	SET_MEM_ZERO(&stClientAddr, sizeof(stClientAddr));
	
	int iClientAddrLen = sizeof(stClientAddr);
	int iClientSockFd = accept(iSockFd, (struct sockaddr*)&stClientAddr, (socklen_t*)&iClientAddrLen);
	if (iClientSockFd < 0)
	{
		printf("Accept client socket failed! errorno(%d), strerror(%s).\n", errno, strerror(errno));
		return FAILED;
	}
	
#ifdef DEBUG
	static unsigned long int ulClientCnt = 0;
	ulClientCnt++;
	unsigned char *pscAddr = (unsigned char*)&(stClientAddr.sin_addr.s_addr);
	printf("Accept socket num is: %lu, Get new client(%u.%u.%u.%u).\n", ulClientCnt, pscAddr[0], pscAddr[1], pscAddr[2], pscAddr[3]);
#endif
	*piClienSockFd = iClientSockFd;
	return SUCCESS;
}

int SocketRcvData(int iSockFd)
{
	if (iSockFd < 0)
	{
		printf("Set socket option failed! param is error.\n");
		return FAILED;
	}
	
	char scDataBuff[100] = {0};
	int iRet = recv(iSockFd, scDataBuff, 1024, 0);
	if (iRet < 0)
	{
		printf("Recv client data failed! errorno(%d), strerror(%s).\n", errno, strerror(errno));
		return FAILED;
	}
	
	if (iRet == 0)
	{
		//printf("Client close the tcp socket.\n");
		close(iSockFd);
		return SUCCESS;
	}
	
	printf("Get client data: [\" %s \"]\n", scDataBuff);
	
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

int EpollCreate(int* const piEpollFd)
{
	int iEpollFd = epoll_create(MAX_EPOLL_EVENTS);
	if (iEpollFd < 0)
	{
		printf("Create epoll fd failed! errorno: %d, error: %s\n", errno, strerror(errno));
		return FAILED;
	}

	*piEpollFd = iEpollFd;
	return SUCCESS;
}

int EpollControl(int iSockFd, int iEpollFd, struct epoll_event* const pstEpollEvent)
{
	if (iSockFd < 0 || iEpollFd < 0)
	{
		printf("Set epoll ctl failed! param is error.\n");
		return FAILED;
	}
	
	pstEpollEvent->data.fd = iSockFd;
	pstEpollEvent->events = EPOLLIN;
	
	int iRet = epoll_ctl(iEpollFd, EPOLL_CTL_ADD, iSockFd, pstEpollEvent);
	if (iRet < 0)
	{
		printf("Epoll ctl failed! iEpollFd: %d, iSockFd: %d errorno: %d, error: %s\n", iEpollFd, iSockFd, errno, strerror(errno));
		return FAILED;
	}
	return SUCCESS;
}

int EpollGetClient(int iEpollFd, int iServerSockFd)
{
	int iClientSockFd = 0;

	int iRet = SocketAcceptClient(iServerSockFd, &iClientSockFd);
	if (iRet != SUCCESS)
	{
		printf("Accept socket failed!\n");
		return FAILED;
	}
	if (iClientSockFd < 0)
	{
		return SUCCESS;
	}
	
	iRet = SocketSetOption(iClientSockFd, O_NONBLOCK);
	if (iRet != SUCCESS)
	{
		printf("Set socket noblock failed!\n");
		close(iClientSockFd);
		return FAILED;
	}
	
	struct epoll_event stClientEvent;
	SET_MEM_ZERO(&stClientEvent, sizeof(stClientEvent));
	
	iRet = EpollControl(iClientSockFd, iEpollFd, &stClientEvent);
	if (iRet != SUCCESS)
	{
		close(iClientSockFd);
		printf("[ERROR]: Epoll ctl failed!\n");
	}

	return SUCCESS;
}

int EpollCycle(int iEpollFd, int iSockFd, struct epoll_event* const pstEpollEvent, int iEpollEventSize)
{
	int iEpollGetFds = 0;
	int iRet = 0, i = 0 ;
	while (1)
	{
		iEpollGetFds = epoll_wait(iEpollFd, pstEpollEvent, iEpollEventSize, -1);
		if (iEpollGetFds < 0)
		{
			printf("[Error]: Epoll wait failed! errorno: %d, error: %s\n", errno, strerror(errno));
			break;
		}
		
		for(i = 0; i < iEpollGetFds; i++)
		{
			if (pstEpollEvent[i].data.fd < 0)
			{
				continue;
			}
			if (iSockFd == pstEpollEvent[i].data.fd)
			{
				iRet = EpollGetClient(iEpollFd, iSockFd);
				if (iRet != SUCCESS)
				{
					printf("[ERROR]: Epoll get client[%d] connection failed!\n", i);
					pstEpollEvent[i].data.fd = -1;
				}
				continue;
			}
			if (pstEpollEvent[i].events & EPOLLIN)
			{
				iRet = SocketRcvData(pstEpollEvent[i].data.fd);
				if (iRet != SUCCESS)
				{
					//printf("[ERROR]: Epoll get client[%d] data failed!\n", i);
					pstEpollEvent[i].data.fd = -1;
					close(pstEpollEvent[i].data.fd);
				}
				continue;
			}
			//other event...
		}
	}
	printf("Epoll cycle over. Server exit!\n");
	return SUCCESS;
}