#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <iomanip>//setprecision
//#include <shared_mutex>
#include "UnityServer.h"
#include "Cueing.h"



int main()
{
	//prefix g_ for global variables	
	float g_motion_data[BUFLEN / 4];
	std::mutex mtx;
	std::condition_variable cond;

	std::atomic<bool> isDataReceived{ false };

	std::thread data_input_thread(UnityServer::GetInputMotionDataFromUnity, &g_motion_data[0], std::ref(isDataReceived), std::ref(mtx), std::ref(cond));
	std::thread cue_theard(Cueing::CueingTest, &g_motion_data[0], std::ref(isDataReceived), std::ref(mtx), std::ref(cond));

	data_input_thread.join();
	cue_theard.join();
	return 0;
}
