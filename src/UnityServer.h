/*
 * UnityServer.hpp
 *
 */
#pragma once
#ifndef UNITYSERVER_HPP_

#define UNITYSERVER_HPP_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <shared_mutex>

#pragma comment(lib, "Ws2_32.lib")

namespace UnityServer
{
	#define BUFLEN 52	//Max length of buffer
	#define PORT 23456


	//functions
	void GetInputMotionDataFromUnity(float *, std::atomic<bool>&, std::mutex&, std::condition_variable&);
	float BytesToFloat(char, char, char, char);
	void DeserializeRecvData(float*, char*);
	void InitializeWinsock(WSADATA& wsa);
	void CreateAndPrepareSocket(SOCKET& s, sockaddr_in& server);
	void BindSocket(const SOCKET& s, sockaddr_in& server);
}

#endif // UNITYSERVER_HPP_