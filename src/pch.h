#pragma once
/*
 * PRECOMPILED HEADER
 *
 */
#include <iostream>
#include <chrono>
#include <iomanip>//setprecision

//Multithreading
#include <thread>
#include <condition_variable>

// Windows socket programming
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

//GLOBAL DEFINES
#define BUFLEN 52
#define PORT_UNITY 23456

//Custom datastructures
struct DataTheard
{
	float motion_data[BUFLEN / 4] = {};//prefix g_ for global variables	
	std::mutex mtx;
	std::condition_variable cond;
	std::atomic<bool> isDataReceived{ false };
};

