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

int EpollCreate(int* const piEpollFd);

int EpollControl(int iSockFd, int iEpollFd, struct epoll_event* const pstEpollEvent);

int EpollGetClient(int iEpollFd, int iServerSockFd);

int EpollCycle(int iEpollFd, int iSockFd, struct epoll_event* const pstEpollEvent, int iEpollEventSize);

#endif

