/*
 * UnityServer.hpp
 *
 */
#pragma once
#ifndef UNITYSERVER_HPP_
#define UNITYSERVER_HPP_

namespace UnityServer
{
	//function
	//void GetInputMotionDataFromUnity(float *, std::atomic<bool>&, std::mutex&, std::condition_variable&);
	void GetInputMotionDataFromUnity(DataTheard&);
	float BytesToFloat(char, char, char, char);
	void DeserializeRecvData(float*, char*);
	void InitializeWinsock(WSADATA& wsa);
	void CreateAndPrepareSocket(SOCKET& s, sockaddr_in& server);
	void BindSocket(const SOCKET& s, sockaddr_in& server);
}

#endif // UNITYSERVER_HPP_