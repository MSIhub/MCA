#pragma once
/*
 * PRECOMPILED HEADER
 *
 */
#include <iostream>
#include <sstream>// ostringstream for logging
#include <chrono>
#include <iomanip>//setprecision
#include <fstream>//reading files
#include <string>
#include <iterator>
#include <map>
#include <cstdint>//int header
#include<math.h>
#include <ctime>


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

//MACROS: UNITY_SERVER
#define BUFLEN_UNITY 28
#define PORT_UNITY 23456

//MACROS: SP7_CLIENT
#define SERVER_SP7 "172.16.0.1"	//ip address of udp server
#define BUFLEN_SP7 190	//Max length of buffer
#define PORT_SP7 0x4654	//The port on which to listen for incoming data
#define MAGIC 0x41545353


//MACROS: CUEING
#define KERNEL_LENGTH 4	
constexpr double M_PI = 3.14159265358979323846;
constexpr double SP7_ZERO_POSE[6] = { 0.0, 0.0, 0.401, 0.0, 0.0, 0.0 };

//Custom datastructures
struct DataThreadUnity
{
	float motion_data[BUFLEN_UNITY / 4] = {};
	std::mutex mtx;
	std::condition_variable cond;
	std::atomic<bool> isDataReceived{ false };
};

struct DataThreadSP7
{
	float motion_data[BUFLEN_SP7] = {};
	std::mutex mtx;
	std::condition_variable cond;
	std::atomic<bool> isDataReceived{ false };
};
