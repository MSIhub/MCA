#include "pch.h"
#include "Cueing.h"

void Cueing::CueingTest(DataTheard& dth)
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(dth.mtx);
		while (!dth.isDataReceived)
		{
			dth.cond.wait(lock);
		}
		if (dth.isDataReceived)
		{
			printf("\n");
			for (int i = 0; i < BUFLEN / 4; i++)
			{
				printf("%f, ", dth.motion_data[i]);
			}
			dth.isDataReceived = false;
		}
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	}
}