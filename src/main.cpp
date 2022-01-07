#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>

#include "UnityServer.h"

int main()
{
	//prefix g_ for global variables	
	float g_motion_data[BUFLEN / 4];
	std::mutex mtx;
	std::condition_variable cond;

	bool isDataReceived = false;

	std::thread data_input_thread(UnityServer::GetInputMotionDataFromUnity, &g_motion_data[0], &isDataReceived, std::ref(mtx), std::ref(cond));
	while (1) {
		//std::this_thread::sleep_for(std::chrono::milliseconds(2));
		std::cout<< "\nIn print loop" << std::endl;
		std::unique_lock<std::mutex> lock(mtx);
		/*while (!isDataReceived)
		{
			cond.wait(lock);
			
		}*/
		cond.wait(lock, [&]()
			{ return isDataReceived; });
		printf("\n");
		for (auto md : g_motion_data)
		{
			printf("%f, ", md);
		}
		lock.unlock();
	}
	
	

	data_input_thread.join();
	
	return 0;
}