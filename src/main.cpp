#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <iomanip>//setprecision
//#include <shared_mutex>
#include "UnityServer.h"

int main()
{
	//prefix g_ for global variables	
	float g_motion_data[BUFLEN / 4];
	std::mutex mtx;
	std::condition_variable cond;

	std::atomic<bool> isDataReceived{ false };

	std::thread data_input_thread(UnityServer::GetInputMotionDataFromUnity, &g_motion_data[0], std::ref(isDataReceived), std::ref(mtx), std::ref(cond));
	std::thread cue_theard([&](){
		while (true)
		{
			std::unique_lock<std::mutex> lock(mtx);
			while (!isDataReceived)
			{
				cond.wait(lock);
			}			
			if (isDataReceived)
			{
				printf("\n");
				for (auto md : g_motion_data)
				{
					printf("%f, ", md);
				}
			}			
			isDataReceived = false;
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});
	
	
	

	data_input_thread.join();
	cue_theard.join();
	return 0;
}