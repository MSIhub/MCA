#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 52	//Max length of buffer
#define PORT 23456

float BytesToFloat(char, char, char, char);
void DeserializeRecvData(float*, char*);
void InitializeWinsock(WSADATA& wsa);
void CreateAndPrepareSocket(SOCKET& s, sockaddr_in& server);
void BindSocket(const SOCKET& s, sockaddr_in& server);
