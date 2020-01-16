#ifndef __COMMON_H__
#define __COMMON_H__

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

#define MAX_EPOLL_EVENTS 500
#define DEBUG
#define FAILED  -1
#define SUCCESS 0
#define SET_MEM_ZERO(pAddr, iSize) (void*)memset(pAddr, 0, iSize)

int SocketCreate(int* const piSockFd);

int SocketServerBindAndListen(int iSockFd, int iPort);

int SocketSetOption(int iSockFd, int iOption); 

int SocketAcceptClient(int iSockFd, int* const piClienSockFd);

int SocketRcvData(int iSockFd);

int SocketSendData(int iSockFd, const char* const pscData, int iDataSize);

int SocketConnectServer(int iSockFd, int iPort, const char* const pscAddr);

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

extern struct CONNECTION_POOL* g_pConnectionPool;

#define NOT_USED 0
#define USED     1

int InitConnectionPool(int iSize);

struct USER_CONNECTION* GetUserConnection();

void ReturnConnection(struct USER_CONNECTION* pConnection);

void DestoryConnection();

int AddEpollEvent(int iEpollFd, struct USER_CONNECTION* ptConnection);

int DelEpollEvent(int iEpollFd, struct USER_CONNECTION* ptConnection);

int ModifyEpollEvent(int iEpollFd, struct USER_CONNECTION* ptConnection);

int WriteClient(int iEpollFd, struct USER_CONNECTION* pConnection);

int ReadClient(int iEpollFd, struct USER_CONNECTION* pConnection);

int HandleClientConnect(int iEpollFd, struct USER_CONNECTION* pConnection);

int InitEpoll(int iEpollFdNum, int iServerSocketFd);

int HandleServerWork(int iServerSocketFd, int iEpollFd, int iMaxEvents);

#endif

