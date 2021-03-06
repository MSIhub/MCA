// MCA.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "UnityServer.h"


void UnityServer::GetInputMotionDataFromUnity(DataThreadUnity& dth)
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN_UNITY];
	WSADATA wsa;
	slen = sizeof(si_other);

	UnityServer::InitializeWinsock(wsa);//Initialise winsock   
	UnityServer::CreateAndPrepareSocket(s, server);//Create a socket
	UnityServer::BindSocket(s, server);//Bind

	//keep listening for data
	while (true)
	{
		ZeroMemory(buf, BUFLEN_UNITY);
		std::unique_lock<std::mutex> lock(dth.mtx);
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN_UNITY, 0, (struct sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			dth.isDataReceived = false;
			exit(EXIT_FAILURE);
		}
		dth.isDataReceived = true;
		DeserializeRecvData(dth.motion_data, buf);
		lock.unlock();
		dth.cond.notify_one();
		std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
		/* Give one ms to start the loop again
			1. To ensure that the lock is not obtained by this theard before triggering the cue theard.
			2. Fixed update happens every 0.02 seconds [50Hz]
			[Tested Unity from 0.02s to 0.008s, the computation every 0.008 created significant lag is data piping]
		*/
	}
	closesocket(s); //close socket
	WSACleanup(); //shutdown socket
}



void UnityServer::InitializeWinsock(WSADATA& wsa)
{
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
}

void UnityServer::CreateAndPrepareSocket(SOCKET& s, sockaddr_in& server)
{
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT_UNITY);
}

void UnityServer::BindSocket(const SOCKET& s, sockaddr_in& server)
{
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");
}


void UnityServer::DeserializeRecvData(float* floatArray, char* byteArray)
{
	int itr = 0;
	for (int i = 0; i < BUFLEN_UNITY; i += 4)
	{
		if (itr < BUFLEN_UNITY / 4)
		{
			floatArray[itr] = UnityServer::BytesToFloat(byteArray[i], byteArray[i + 1], byteArray[i + 2], byteArray[i + 3]);
			itr++;
		}
	}
}


float UnityServer::BytesToFloat(char b0, char b1, char b2, char b3)
{
	// char byte_array[] = { b3, b2, b1, b0 };
	char byte_array[] = { b0, b1, b2, b3 };
	float result;
	std::copy(reinterpret_cast<const char*>(&byte_array[0]),
		reinterpret_cast<const char*>(&byte_array[4]),
		reinterpret_cast<char*>(&result));
	return result;
}


// //print details of the client/peer and the data received
// char clientIp[BUFLEN];
// ZeroMemory(clientIp, BUFLEN);
// inet_ntop(AF_INET, &si_other.sin_addr, clientIp, BUFLEN);
////printf("Received packet from %s:%d\n", clientIp, ntohs(si_other.sin_port));
