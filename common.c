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
	
	iRet = listen(iSockFd, 20000);
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

struct USER_CONNECTION {
	struct epoll_event m_tEvent;
	int m_iFd;
	int m_iInUse;
	int (*m_pfHandle)(int, struct USER_CONNECTION*);
	struct USER_CONNECTION* m_pNext;
};

struct CONNECTION_POOL {
	struct USER_CONNECTION* m_pAllUserConnection;
	struct USER_CONNECTION* m_pUnusedUserConnection;
	int m_iSize;
};

struct CONNECTION_POOL* g_pConnectionPool = NULL;

#define NOT_USED 0
#define USED     1

int InitConnectionPool(int iSize)
{
	if (iSize < 0)
	{
		return -1;
	}
	struct USER_CONNECTION* pAllUserConnection = (struct USER_CONNECTION*)malloc(iSize * sizeof(USER_CONNECTION));
	if (NULL == pAllUserConnection)
	{
		printf("Malloc user connection failed, error: [%d][%s].\n", errno, strerror(errno));
		return -1;
	}

	memset(pAllUserConnection, 0, iSize * sizeof(USER_CONNECTION));

	for(int i = 0; i < iSize - 1; i++)
	{
		pAllUserConnection[i].m_iInUse = NOT_USED;
		pAllUserConnection[i].m_pNext = &pAllUserConnection[i + 1];
	}

	pAllUserConnection[iSize - 1].m_iInUse = NOT_USED;
	pAllUserConnection[iSize - 1].m_pNext = NULL;

	g_pConnectionPool = (struct CONNECTION_POOL*)malloc(sizeof(struct CONNECTION_POOL) * 1);
	if (NULL == g_pConnectionPool)
	{
		printf("Malloc connection pool failed, error: [%d][%s].\n", errno, strerror(errno));
		return -1;
	}

	g_pConnectionPool->m_iSize = iSize;
	g_pConnectionPool->m_pAllUserConnection = pAllUserConnection;
	g_pConnectionPool->m_pUnusedUserConnection = pAllUserConnection;
	return 0;
}

struct USER_CONNECTION* GetUserConnection()
{
	int iRet = 0;
	if (NULL == g_pConnectionPool || NULL == g_pConnectionPool->m_pAllUserConnection)
	{
		iRet = InitConnectionPool(1000);
		if (iRet < 0)
		{
			printf("Init user connection failed.\n");
			return NULL;
		}
	}

	if (NULL == g_pConnectionPool->m_pUnusedUserConnection)
	{
		printf("[Error] There is no more connection can be use.\n");
		return NULL;
	}

	struct USER_CONNECTION* ptGetConnection = g_pConnectionPool->m_pUnusedUserConnection;
	g_pConnectionPool->m_pUnusedUserConnection = g_pConnectionPool->m_pUnusedUserConnection->m_pNext;

	ptGetConnection->m_iInUse = USED;

	return ptGetConnection;
}

void ReturnConnection(struct USER_CONNECTION* pConnection)
{
	struct USER_CONNECTION* ptUnUsedConnection = g_pConnectionPool->m_pUnusedUserConnection;
	pConnection->m_iInUse = NOT_USED;
	close(pConnection->m_iFd);

	g_pConnectionPool->m_pUnusedUserConnection = pConnection;
	g_pConnectionPool->m_pUnusedUserConnection->m_pNext = ptUnUsedConnection;
}

void DestoryConnection()
{
	if (NULL == g_pConnectionPool)
	{
		return;
	}

	if (NULL != g_pConnectionPool->m_pAllUserConnection)
	{
		//for(struct USER_CONNECTION* pConnection = g_pConnectionPool->m_pUsedUserConnection; pConnection != NULL; pConnection = pConnection->m_pNext)
		for(int i = 0; i < g_pConnectionPool->m_iSize; i++)
		{
			if (USED == g_pConnectionPool->m_pAllUserConnection->m_iInUse)
			{
				close(g_pConnectionPool->m_pAllUserConnection->m_iFd);
			}
		}

		free(g_pConnectionPool->m_pAllUserConnection);
		g_pConnectionPool->m_pAllUserConnection = NULL;
	}

	delete g_pConnectionPool;
	g_pConnectionPool = NULL;
}


long g_ServerHandleClient = 0;

int AddEpollEvent(int iEpollFd, struct USER_CONNECTION* ptConnection)
{
	int iRet = epoll_ctl(iEpollFd, EPOLL_CTL_ADD, ptConnection->m_iFd, &ptConnection->m_tEvent);
	if (iRet < 0)
	{
		printf("Epoll add failed, iEpollFd: %d, clinetFd: %d, error: [%d][%s].\n", iEpollFd, ptConnection->m_tEvent.data.fd, errno, strerror(errno));
		return iRet;
	}
	return 0;
}

int DelEpollEvent(int iEpollFd, struct USER_CONNECTION* ptConnection)
{
	int iRet = epoll_ctl(iEpollFd, EPOLL_CTL_DEL, ptConnection->m_iFd, &ptConnection->m_tEvent);
	if (iRet < 0)
	{
		printf("Epoll add event failed, error: [%d][%s].\n", errno, strerror(errno));
		return iRet;
	}
	return 0;
}

int ModifyEpollEvent(int iEpollFd, struct USER_CONNECTION* ptConnection)
{
	int iRet = epoll_ctl(iEpollFd, EPOLL_CTL_MOD, ptConnection->m_iFd, &ptConnection->m_tEvent);
	if (iRet < 0)
	{
		printf("Epoll add event failed, error: [%d][%s].\n", errno, strerror(errno));
		return iRet;
	}
	return 0;
}

int WriteClient(int iEpollFd, struct USER_CONNECTION* pConnection);

int ReadClient(int iEpollFd, struct USER_CONNECTION* pConnection)
{
	char szBuff[1024] = {0};
	int iClientSockFd = pConnection->m_iFd;
	int iRet = read(iClientSockFd, szBuff, sizeof(szBuff));
	if (iRet < 0)
	{
		printf("Read failed, error: [%d][%s].\n", errno, strerror(errno));
		//close(iClientSockFd);
		return -1;
	}

	if (iRet == 0 && strlen(szBuff) == 0)
	{
		iRet = DelEpollEvent(iEpollFd, pConnection);
		if (iRet < 0)
		{
			printf("Delete event failed.\n");
		}
		ReturnConnection(pConnection);
		printf("Read client data length is 0, Return one connection.\n");
	} else {
		printf("Read client data: %s.\n", szBuff);

		pConnection->m_pfHandle = WriteClient;
		pConnection->m_tEvent.events = EPOLLOUT; // | EPOLLET;
		iRet = ModifyEpollEvent(iEpollFd, pConnection);
		if (iRet < 0)
		{
			printf("Modify epoll event failed.\n");
			//return iRet;
		}
		printf("Modify event to write.\n");
	}
	return iRet;
}

int WriteClient(int iEpollFd, struct USER_CONNECTION* pConnection)
{
	char szBuff[128] = {0};
	sprintf(szBuff, "Server Handled client: %ld.", g_ServerHandleClient++);
	int iRet = write(pConnection->m_iFd, szBuff, strlen(szBuff));
	if (iRet < 0)
	{
		printf("Write failed, error: [%d][%s].\n", errno, strerror(errno));
		return iRet;
	}

	//pConnection->m_tEvent.data.ptr = ReadClient;
#define READ
#ifdef READ
	pConnection->m_pfHandle = ReadClient;
	pConnection->m_tEvent.events = EPOLLIN | EPOLLET;
	iRet = ModifyEpollEvent(iEpollFd, pConnection);
	if (iRet < 0)
	{
		printf("Modify epoll event failed.\n");
		//return iRet;
	}
	printf("Modify event to read.\n");
#else
	iRet = DelEpollEvent(iEpollFd, pConnection);
	if (iRet < 0)
	{
		printf("Modify epoll event failed.\n");
		//return iRet;
	}
	ReturnConnection(pConnection);
	printf("Return one connection.\n");
#endif
	return iRet;
}

int HandleClientConnect(int iEpollFd, struct USER_CONNECTION* pConnection)
{
	int iClientSockFd = 0;
	struct sockaddr_in tClientAddr;
	int iClientLen = 0;
	struct USER_CONNECTION* ptClientConnection = GetUserConnection();

	int iServerSocketFd = pConnection->m_iFd;
	iClientSockFd = accept(iServerSocketFd, (struct sockaddr *)&tClientAddr, (socklen_t *)&iClientLen);
	if (iClientSockFd < 0)
	{
		printf("Accept failed, error: [%d][%s].\n", errno, strerror(errno));
		return -1;
	}

	int iRet = 0;
	if (iClientSockFd != iEpollFd) {
	int iRet = SocketSetOption(iClientSockFd, O_NONBLOCK);
		if (iRet < 0)
		{
			printf("Set client socket fd failed, error: [%d][%s].\n", errno, strerror(errno));
			return iRet;
		}
	}

	ptClientConnection->m_iFd = iClientSockFd;
	ptClientConnection->m_pfHandle = ReadClient;
	ptClientConnection->m_tEvent.data.ptr = ptClientConnection;
	ptClientConnection->m_tEvent.events = EPOLLIN | EPOLLET;
	iRet = AddEpollEvent(iEpollFd, ptClientConnection);
	if (iRet < 0)
	{
		printf("Add epoll event failed, error: [%d][%s].\n", errno, strerror(errno));
		//return iRet;
	}

	return iRet;
}

int InitEpoll(int iEpollFdNum, int iServerSocketFd)
{
	int iEpollFd = epoll_create(iEpollFdNum);
	if (iEpollFd < 0)
	{
		printf("Epoll create failed, error: [%d][%s].\n", errno, strerror(errno));
		return iEpollFd;
	}

	struct USER_CONNECTION* ptConnection = GetUserConnection();
	ptConnection->m_iFd = iServerSocketFd;
	ptConnection->m_pfHandle = HandleClientConnect;
	ptConnection->m_tEvent.data.ptr = ptConnection;
	ptConnection->m_tEvent.events = EPOLLIN | EPOLLET;

	int iRet = AddEpollEvent(iEpollFd, ptConnection);
	if (iRet < 0)
	{
		printf("Add epoll event failed, iServerSocketFd: %d -- %d, error: [%d][%s].\n", iServerSocketFd, ptConnection->m_tEvent.data.fd, errno, strerror(errno));
		close(iEpollFd);
		return iRet;
	}

	return iEpollFd;
}

int HandleServerWork(int iServerSocketFd, int iEpollFd, int iMaxEvents)
{
	if (iServerSocketFd < 0 || iEpollFd < 0)
	{
		return -1;
	}

	//int iRet = 0;
	int iEventNum = 0;

	struct epoll_event tGetEpollEvents[200];
	memset(&tGetEpollEvents, 0, sizeof(struct epoll_event) * 200);
	struct USER_CONNECTION* pConnection;

	while(1) {
		iEventNum = epoll_wait(iEpollFd, tGetEpollEvents, iMaxEvents, 0);
		if (iEventNum < 0)
		{
			printf("epoll wake up, iEventNum: %d...\n", iEventNum);
			continue;
		}

		for(int i = 0; i < iEventNum; i++)
		{
			pConnection = (struct USER_CONNECTION*)tGetEpollEvents[i].data.ptr;
			pConnection->m_pfHandle(iEpollFd, pConnection);
			if (pConnection->m_iFd == iServerSocketFd)
			{
				printf("This is Server fd.\n");
				//ReturnConnection(pConnection);
			}
			printf("Handle one connection, fd: %d, iEventNum: %d.\n", pConnection->m_iFd, iEventNum);
		}

	}
	return 0;
}
