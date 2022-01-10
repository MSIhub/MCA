#include "pch.h"
#include "Cueing.h"

void Cueing::CueMain(DataTheard& dth)
{
	float temp_motion_data[BUFLEN / 4] = {};

	while (true)
	{
		std::unique_lock<std::mutex> lock(dth.mtx);
		while (!dth.isDataReceived)
		{
			dth.cond.wait(lock);
		}
		if (dth.isDataReceived)
		{
			//Cueing function goes here
			//----------------------------//
			Cueing::ScaleInputData(&temp_motion_data[0], dth);

			printf("\n");
			for (int i = 0; i < BUFLEN / 4; i++)
			{
				printf("%f, ", temp_motion_data[i]);
			}

			//----------------------------//
			dth.isDataReceived = false;
		}
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	}
}

void Cueing::ScaleInputData(float*  temp_motion_data, DataTheard& dth)
{
	temp_motion_data[0] = dth.motion_data[0]; //timestamp
	for (int i = 1; i < BUFLEN / 4; i++)
	{
		temp_motion_data[i] = 0.01f * dth.motion_data[i];
	}
}

//printf("\n");
//for (int i = 0; i < BUFLEN / 4; i++)
//{
//	printf("%f, ", dth.motion_data[i]);
//}

